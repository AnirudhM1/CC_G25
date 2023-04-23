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
    bool has_Main = false;
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

    NodeStmts *stmts = (NodeStmts*)root;

    for(auto stmt : stmts->list) {
        NodeFunDef *func_node = (NodeFunDef*)stmt;
        std::string func_name = func_node->name;
        if(func_name == "main") {
            // main function
            has_Main=true;
            builder.SetInsertPoint(main_func_entry_bb);
            func_node->llvm_codegen(this);
        }
        else {
            // other functions
            func_node->llvm_codegen(this);
        }
    }
    if(!has_Main) {
        std::cerr << "Error: No main function found" << std::endl;
        exit(1);
    }
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

    // printf("exp: %u\n", expr->getType()->getTypeID());

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
    int level = compiler->level;
    while(level > 0) {
        if(compiler->locals[level][identifier] != nullptr) {
            break;
        }
        level--;
    }
    AllocaInst *alloc = compiler->locals[level][identifier];

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

    BasicBlock *then_ret_bb = BasicBlock::Create(
        *compiler->context,
        "then_ret"
    );

    BasicBlock *else_ret_bb = BasicBlock::Create(
        *compiler->context,
        "else_ret"
    );


    compiler->builder.CreateCondBr(cond, then_bb, else_bb);
    compiler->builder.SetInsertPoint(then_bb);




    // Value *then_val = if_block->llvm_codegen(compiler);
    Value *then_val = nullptr;
    NodeStmts *if_block_stmts = (NodeStmts*)if_block;
    bool then_returned = false;

    for(auto node : if_block_stmts->list) {
        then_val = node->llvm_codegen(compiler);
        if (node->type == Node::NodeType::RT) {
            then_returned = true;
            break;
        }
    }

    // printf("then_val: %u", then_val->getType()->getTypeID());
    // printf("then_val: %s", if_block->to_string().c_str());

    if (!then_val) return nullptr;

    StoreInst *store1 = dyn_cast<StoreInst>(then_val);
    if (store1)
        then_val = store1->getValueOperand();

    if (!then_returned)
        compiler->builder.CreateBr(merge_bb);

    then_bb = compiler->builder.GetInsertBlock();

    current_func->getBasicBlockList().push_back(else_bb);
    compiler->builder.SetInsertPoint(else_bb);

    // Value *else_val = else_block->llvm_codegen(compiler);
    Value *else_val = nullptr;
    NodeStmts *else_block_stmts = (NodeStmts*)else_block;
    bool else_returned = false;

    for(auto node : else_block_stmts->list) {
        else_val = node->llvm_codegen(compiler);
        if (node->type == Node::NodeType::RT) {
            else_returned = true;
            break;
        }
    }



    if (!else_val) return nullptr;

    StoreInst *store2 = dyn_cast<StoreInst>(else_val);
    if (store2)
        else_val = store2->getValueOperand();
 
    if(!else_returned)
        compiler->builder.CreateBr(merge_bb);

    else_bb = compiler->builder.GetInsertBlock();

    current_func->getBasicBlockList().push_back(merge_bb);
    compiler->builder.SetInsertPoint(merge_bb);

    
    if(then_returned && else_returned) {
        return nullptr;
    }
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

    // printf("TYPE: %u\n", then_val->getType()->getTypeID());
    // printf("TYPE: %u\n", else_val->getType()->getTypeID());
    // printf("TYPE: %u\n", phi_node->getType()->getTypeID());

    // std::cout<<then_returned<<" "<<else_returned<<std::endl;

    if (!then_returned)
        phi_node->addIncoming(then_val, then_bb);
    if (!else_returned)
        phi_node->addIncoming(else_val, else_bb);

    compiler->level--;

    return phi_node;

}

Value *NodeReturn::llvm_codegen(LLVMCompiler *compiler) {
    Value *expr = expression->llvm_codegen(compiler);

    compiler->builder.CreateRet(expr);
    return expr;
}

Value *NodeFunCall::llvm_codegen(LLVMCompiler *compiler) {
    Function *func = compiler->module.getFunction(name);

    std::vector<Value *> args;
    for(auto node : parameters) {
        args.push_back(node->llvm_codegen(compiler));
    }

    return compiler->builder.CreateCall(func, args, "calltmp");

}

Value *NodeFunDef::llvm_codegen(LLVMCompiler *compiler) {

    if(name == "main") {
        // function defination for 'main' is defined in the compile function
        // so we don't need to create it again.

        NodeStmts *stmts = (NodeStmts*)block;

        Node *last = nullptr;

        for(auto node : stmts->list) {
            node->llvm_codegen(compiler);
            last = node;
        }

        if(last == nullptr || last->type != Node::NodeType::RT) {
            compiler->builder.CreateRet(compiler->builder.getInt64(0));
        }

        return nullptr;
    }

    // Otherwise create the function defination

    std::vector<Type *> args;
    for(auto type : parameter_types) {
        args.push_back(compiler->builder.getInt64Ty());
    }


    FunctionType *func_type = FunctionType::get(
        compiler->builder.getInt64Ty(),
        args,
        false
    );

    Function *func = Function::Create(
        func_type,
        Function::ExternalLinkage,
        name,
        &compiler->module
    );


    BasicBlock *bb = BasicBlock::Create(
        *compiler->context,
        "entry",
        func
    );

    compiler->builder.SetInsertPoint(bb);

    IRBuilder<> temp_builder(
        &func->getEntryBlock(),
        func->getEntryBlock().begin()
    );


    // set the function parameters in the locals map
    unsigned int i = 0;
    for (Function::arg_iterator arg = func->arg_begin(); arg != func->arg_end(); ++arg, ++i) {
        arg->setName(parameter_names[i]);
    }

    i = 0;
    for(Function::arg_iterator arg = func->arg_begin(); arg != func->arg_end(); ++arg, ++i) {
        std::string identifier = parameter_names[i];
        AllocaInst *alloc = temp_builder.CreateAlloca(
            compiler->builder.getInt64Ty(),
            0, 
            identifier
        );

        compiler->locals[compiler->level][identifier] = alloc;
        compiler->builder.CreateStore(func->arg_begin() + i, alloc);
    }


    Node *last = nullptr;

    NodeStmts *stmts = (NodeStmts*)block;

    for(auto node : stmts->list) {
        node->llvm_codegen(compiler);
        last = node;
    }

    if(last == nullptr || last->type != Node::NodeType::RT) {
        compiler->builder.CreateRet(compiler->builder.getInt64(0));
    }

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