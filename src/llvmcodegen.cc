#include "llvmcodegen.hh"
#include "ast.hh"
#include <iostream>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <vector>

#define MAIN_FUNC compiler->module.getFunction("main")

/*
The documentation for LLVM codegen, and how exactly this file works can be found
ins `docs/llvm.md`
*/

void LLVMCompiler::compile(Node *root) {
    /* Adding reference to print_i in the runtime library */
    // void printi();
    FunctionType *printi_func_type = FunctionType::get(
        builder.getVoidTy(),
        {builder.getInt64Ty()},
        false
    );
    Function::Create(
        printi_func_type,
        GlobalValue::ExternalLinkage,
        "printi",
        &module
    );
    /* we can get this later 
        module.getFunction("printi");
    */

    /* Main Function */
    // int main();
    FunctionType *main_func_type = FunctionType::get(
        builder.getInt64Ty(), {}, false /* is vararg */
    );
    Function *main_func = Function::Create(
        main_func_type,
        GlobalValue::ExternalLinkage,
        "main",
        &module
    );

    // create main function block
    BasicBlock *main_func_entry_bb = BasicBlock::Create(
        *context,
        "entry",
        main_func
    );

    // move the builder to the start of the main function block
    builder.SetInsertPoint(main_func_entry_bb);

    root->llvm_codegen(this);

    // return 0;
    builder.CreateRet(builder.getInt64(0));
}

void LLVMCompiler::dump() {
    outs() << module;
}

void LLVMCompiler::write(std::string file_name) {
    std::error_code EC;
    raw_fd_ostream fout(file_name, EC, sys::fs::OF_None);
    WriteBitcodeToFile(module, fout);
    fout.flush();
    fout.close();
}

//  ┌―――――――――――――――――――――┐  //
//  │ AST -> LLVM Codegen │  //
// └―――――――――――――――――――――┘   //

// codegen for statements
Value *NodeStmts::llvm_codegen(LLVMCompiler *compiler) {
    Value *last = nullptr;
    for(auto node : list) {
        last = node->llvm_codegen(compiler);
    }

    return last;
}

Value *NodeDebug::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    Function *printi_func = compiler->module.getFunction("printi");
    compiler->builder.CreateCall(printi_func, {expr});

    return expr;
}

Value *NodeInt::llvm_codegen(LLVMCompiler *compiler) {
    return compiler->builder.getInt64(value);
}

Value *NodeBinOp::llvm_codegen(LLVMCompiler *compiler) {
    Value *left_expr = left->llvm_codegen(compiler);
    Value *right_expr = right->llvm_codegen(compiler);

    switch(op) {
        case PLUS:
        return compiler->builder.CreateAdd(left_expr, right_expr, "addtmp");
        case MINUS:
        return compiler->builder.CreateSub(left_expr, right_expr, "minustmp");
        case MULT:
        return compiler->builder.CreateMul(left_expr, right_expr, "multtmp");
        case DIV:
        return compiler->builder.CreateSDiv(left_expr, right_expr, "divtmp");
    }
}


Value *NodeDecl::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    IRBuilder<> temp_builder(
        &MAIN_FUNC->getEntryBlock(),
        MAIN_FUNC->getEntryBlock().begin()
    );

    Type *t;

    switch(datatype) {
        case INT:
        t = compiler->builder.getInt32Ty();
        break;
        case SHORT:
        t = compiler->builder.getInt16Ty();
        break;
        case LONG:
        t = compiler->builder.getInt64Ty();
        break;
    }

    t = compiler->builder.getInt64Ty();

    AllocaInst *alloc = temp_builder.CreateAlloca(t, 0, identifier);
    // compiler->locals[identifier].push_back(alloc);
    // compiler->locals[identifier].push_back(alloc);
    compiler->locals[compiler->level][identifier] = alloc;

    return compiler->builder.CreateStore(expr, alloc);
}

Value *NodeIdent::llvm_codegen(LLVMCompiler *compiler) {
    AllocaInst *alloc = compiler->locals[compiler->level][identifier];

    // if your LLVM_MAJOR_VERSION >= 14
    return compiler->builder.CreateLoad(compiler->builder.getInt64Ty(), alloc, identifier);
}


Value *NodeIfElse::llvm_codegen(LLVMCompiler *compiler) {

    

    Value *cond = condition->llvm_codegen(compiler);
    if (!cond) return nullptr;

    compiler->level++;

    cond = compiler->builder.CreateICmpSLT(
        compiler->builder.getInt64(0),
        cond,
        "ifcond"
    );



    Function *current_func = compiler->builder.GetInsertBlock()->getParent();

    BasicBlock *then_bb = BasicBlock::Create(
        *compiler->context,
        "then",
        current_func
    );
    BasicBlock *else_bb = BasicBlock::Create(
        *compiler->context,
        "else"
    );
    BasicBlock *merge_bb = BasicBlock::Create(
        *compiler->context,
        "ifcont"
    );


    compiler->builder.CreateCondBr(cond, then_bb, else_bb);
    compiler->builder.SetInsertPoint(then_bb);



    Value *then_val = if_block->llvm_codegen(compiler);
    if (!then_val) return nullptr;

    StoreInst *store1 = dyn_cast<StoreInst>(then_val);
    if (store1)
        then_val = store1->getValueOperand();


    compiler->builder.CreateBr(merge_bb);
    then_bb = compiler->builder.GetInsertBlock();

    current_func->getBasicBlockList().push_back(else_bb);
    compiler->builder.SetInsertPoint(else_bb);

    Value *else_val = else_block->llvm_codegen(compiler);
    if (!else_val) return nullptr;

    StoreInst *store2 = dyn_cast<StoreInst>(else_val);
    if (store2)
        else_val = store2->getValueOperand();
 
    
    compiler->builder.CreateBr(merge_bb);
    else_bb = compiler->builder.GetInsertBlock();

    current_func->getBasicBlockList().push_back(merge_bb);
    compiler->builder.SetInsertPoint(merge_bb);

    PHINode *phi_node = compiler->builder.CreatePHI(
        compiler->builder.getInt64Ty(),
        2,
        "iftmp"
    );

    // printf("Type: %u\n", phi_node->getType()->getTypeID());
    // printf("Type: %u\n", then_bb->getType()->getTypeID());


    // printf("Type: %u\n", phi_node->getNumOperands() - 1);

    // cast 'then_val' variable to type of DoubleTyID
    // then_val = compiler->builder.CreateFPCast(then_val, compiler->builder.getDoubleTy(), "iftmp");
    // else_val = compiler->builder.CreateFPCast(else_val, compiler->builder.getDoubleTy(), "iftmp");

    phi_node->addIncoming(then_val, then_bb);
    phi_node->addIncoming(else_val, else_bb);

    compiler->level--;

    return phi_node;

}

Value *NodeReturn::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    return compiler->builder.CreateRet(expr);
}

Value *NodeFunCall::llvm_codegen(LLVMCompiler *compiler) {
    // Function *func = compiler->module.getFunction(identifier);

    // std::vector<Value *> args;
    // for(auto node : list) {
    //     args.push_back(node->llvm_codegen(compiler));
    // }

    // return compiler->builder.CreateCall(func, args, "calltmp");

    return nullptr;
}

Value *NodeFunDef::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}

// codegen for non AST nodes.
Value *NodeParamDecl::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}
Value *NodeParamPass::llvm_codegen(LLVMCompiler *compiler) {
    return nullptr;
}
#undef MAIN_FUNC