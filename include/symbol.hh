#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <set>
#include <string>
#include <unordered_map>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::unordered_map<std::string, std::string> table;
    SymbolTable *parent;

    SymbolTable(SymbolTable *parent);
    bool contains(std::string key);
    bool contains_up(std::string key);
    void insert(std::string key, std::string type);
    SymbolTable *add_scope();
    SymbolTable *remove_scope();
};

struct SymbolTableContainer {
    SymbolTable *current_scope;
    std::unordered_map<std::string, std::vector<NodeDecl::DataType>> fun_table;

    SymbolTableContainer();
    void add_scope();
    void remove_scope();
    void insert(std::string key, std::string type);
    bool contains(std::string key);
    bool contains_up(std::string key);
    std::string get_type(std::string key);
    void insert_fun(std::string key, std::vector<NodeDecl::DataType> args);
    bool check_fun(std::string key, std::vector<NodeDecl::DataType> args);
    bool check_fun(std::string key, int num_args);

};

#endif