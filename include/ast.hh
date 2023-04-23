#ifndef AST_HH
#define AST_HH

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

struct LLVMCompiler;

/**
Base node class. Defined as `abstract`.
*/
struct Node {
    enum NodeType {
        BIN_OP, INT_LIT, STMTS, ASSN, DBG, IDENT, IF_ELSE, NON_AST, FN, RT
    } type;

    virtual std::string to_string() = 0;
    virtual llvm::Value *llvm_codegen(LLVMCompiler *compiler) = 0;
};

/**
    Node for list of statements
*/
struct NodeStmts : public Node {
    std::vector<Node*> list;

    NodeStmts();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for binary operations
*/
struct NodeBinOp : public Node {
    enum Op {
        PLUS, MINUS, MULT, DIV
    } op;

    Node *left, *right;

    NodeBinOp(Op op, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for integer literals
*/
struct NodeInt : public Node {
    long value;

    NodeInt(long val);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for variable assignments
*/
struct NodeDecl : public Node {
    std::string identifier;
    Node *expression;

    enum DataType {
        INT, SHORT, LONG
    } datatype;

    NodeDecl(std::string id, Node *expr, DataType dtype);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for `dbg` statements
*/
struct NodeDebug : public Node {
    Node *expression;

    NodeDebug(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for idnetifiers
*/
struct NodeIdent : public Node {
    std::string identifier;

    NodeIdent(std::string ident);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for If-Else statements
*/
struct NodeIfElse : public Node {
    Node *condition;
    Node *if_block;
    Node *else_block;

    NodeIfElse(Node *cond, Node *ifb, Node *elseb);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for return statement
*/
struct NodeReturn : public Node {
    Node *expression;

    NodeReturn(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for function declarations
*/
struct NodeFunDef : public Node {
    std::string name;
    NodeDecl::DataType return_type;
    std::vector<NodeDecl::DataType> parameter_types;
    std::vector<std::string> parameter_names;
    Node *block;
    Node *return_expression;

    NodeFunDef(std::string id, std::vector<NodeDecl::DataType> parameter_types, std::vector<std::string> names, Node *block, NodeDecl::DataType ret);
    NodeFunDef(std::string id, std::vector<NodeDecl::DataType> parameter_types, std::vector<std::string> names, Node *block, NodeDecl::DataType ret, Node *ret_expression);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for function calls
*/
struct NodeFunCall : public Node {
    std::string name;
    std::vector<Node*> parameters;

    NodeFunCall(std::string id, std::vector<Node*> parameters);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for declaring params in a function. This is a special node that is not used in the AST.
*/
struct NodeParamDecl : public Node {
    std::vector<NodeDecl::DataType> list;
    std::vector<std::string> nodes;

    NodeParamDecl(std::string identifier, NodeDecl::DataType dtype);
    NodeParamDecl(Node* node, std::string identifier, NodeDecl::DataType dtype);
    std::vector<NodeDecl::DataType> get_type_list();
    std::vector<std::string> get_node_list();
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for pasing params to a function. This is a special node that is not used in the AST.
*/
struct NodeParamPass : public Node {
    std::vector<Node*> list;

    NodeParamPass(Node *expr);
    NodeParamPass(Node *node, Node *expr);
    std::vector<Node*> get_list();
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

#endif