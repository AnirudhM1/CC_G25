#include "ast.hh"

#include <string>
#include <vector>

NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
}

std::string NodeBinOp::to_string() {
    std::string out = "(";
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }

    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';

    return out;
}

NodeInt::NodeInt(long val) {
    type = INT_LIT;
    value = val;
    // printf("Created node with value %ld\n", value);
}

std::string NodeInt::to_string() {
    return std::to_string(value);
}

NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}

std::string NodeStmts::to_string() {
    std::string out = "(begin";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

NodeDecl::NodeDecl(std::string id, Node *expr, NodeDecl::DataType dtype) {
    type = ASSN;
    datatype = dtype;
    identifier = id;
    expression = expr;
}

std::string NodeDecl::to_string() {
    return "(let " + identifier + " " + expression->to_string() + ")";
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident) {
    identifier = ident;
}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeIfElse::NodeIfElse(Node *cond, Node *iftrue, Node *iffalse) {
    type = IF_ELSE;
    condition = cond;
    if_block = iftrue;
    else_block = iffalse;
}
std::string NodeIfElse::to_string() {
    return "(if-else " + condition->to_string() + " " + if_block->to_string() + " " + else_block->to_string() + ")";
}

NodeReturn::NodeReturn(Node *expr) {
    type = RT;
    expression = expr;
}
std::string NodeReturn::to_string() {
    return "(ret " + expression->to_string() + ")";
}

NodeFunDef::NodeFunDef(std::string identifier, std::vector<NodeDecl::DataType> types, Node *body, NodeDecl::DataType ret) {
    type = FN;
    name = identifier;
    return_type = ret;
    parameter_types = types;
    block = body;
    return_expression = nullptr;
}
NodeFunDef::NodeFunDef(std::string identifier, std::vector<NodeDecl::DataType> types, Node *body, NodeDecl::DataType ret, Node *ret_expr) {
    type = FN;
    name = identifier;
    return_type = ret;
    parameter_types = types;
    block = body;
    return_expression = ret_expr;
}
std::string NodeFunDef::to_string() {
    std::string type_string = "(";
    for(auto i : parameter_types) {
        switch(i) {
            case NodeDecl::INT: type_string += "int "; break;
            case NodeDecl::SHORT: type_string += "short "; break;
            case NodeDecl::LONG: type_string += "long "; break;
        }
    }
    type_string += ")";
    std::string ret_type;
    switch(return_type) {
        case NodeDecl::INT: ret_type = "int"; break;
        case NodeDecl::SHORT: ret_type = "short"; break;
        case NodeDecl::LONG: ret_type = "long"; break;
    }
    // std::string ret_string;
    // if(return_expression == nullptr) {
    //     ret_string = "ret:(0)";
    // } else {
    //     ret_string = "ret:" + return_expression->to_string();
    // }
    return "(fn " + name + ":" + ret_type + " " + type_string + " " + block->to_string() + " " + ")";
}

NodeFunCall::NodeFunCall(std::string identifier, std::vector<Node*> params) {
    type = FN;
    name = identifier;
    parameters = params;
}
std::string NodeFunCall::to_string() {
    std::string out = "(call " + name;
    for(auto i : parameters) {
        out += " " + i->to_string();
    }
    out += ")";
    return out;
}

NodeParamDecl::NodeParamDecl(std::string identifier, NodeDecl::DataType dtype) {
    type = NON_AST;
    list.push_back(dtype);
    nodes.push_back(identifier);
}
NodeParamDecl::NodeParamDecl(Node *node, std::string identifier, NodeDecl::DataType dtype) {
    type = NON_AST;
    for(auto i : ((NodeParamDecl*)node)->list) {
        list.push_back(i);
    }
    for(auto i : ((NodeParamDecl*)node)->nodes) {
        nodes.push_back(i);
    }
    list.push_back(dtype);
    nodes.push_back(identifier);
}
std::vector<NodeDecl::DataType> NodeParamDecl::get_type_list() {
    return list;
}
std::vector<std::string> NodeParamDecl::get_node_list() {
    return nodes;
}
std::string NodeParamDecl::to_string() {
    return nullptr; // Not a part of the AST. Hence this function should never be called.
}

NodeParamPass::NodeParamPass(Node *expression) {
    type = NON_AST;
    list.push_back(expression);
}
NodeParamPass::NodeParamPass(Node *node, Node *expression) {
    type = NON_AST;
    for(auto i : ((NodeParamPass*)node)->list) {
        list.push_back(i);
    }
    list.push_back(expression);
}
std::vector<Node*> NodeParamPass::get_list() {
    return list;
}
std::string NodeParamPass::to_string() {
    return nullptr; // Not a part of the AST. Hence this function should never be called.
}