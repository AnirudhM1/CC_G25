#include "symbol.hh"

SymbolTable::SymbolTable(SymbolTable *parent) {
    this->parent = parent;
}

bool SymbolTable::contains(std::string key) {
    return table.find(key) != table.end();
}

bool SymbolTable::contains_up(std::string key) {
    if (this->contains(key)) {
        return true;
    } else if (this->parent != NULL) {
        return this->parent->contains_up(key);
    } else {
        return false;
    }
}

void SymbolTable::insert(std::string key, std::string type) {
    table[key] = type;
}

SymbolTable* SymbolTable::add_scope() {
    SymbolTable *new_table = new SymbolTable(this);
    return new_table;
}

SymbolTable* SymbolTable::remove_scope() {
    return this->parent;
}

SymbolTableContainer::SymbolTableContainer() {
    current_scope = new SymbolTable(NULL);
}

void SymbolTableContainer::add_scope() {
    current_scope = current_scope->add_scope();
}

void SymbolTableContainer::remove_scope() {
    current_scope = current_scope->remove_scope();
}

void SymbolTableContainer::insert(std::string key, std::string type) {
    current_scope->insert(key, type);
}

bool SymbolTableContainer::contains(std::string key) {
    return current_scope->contains(key);
}

bool SymbolTableContainer::contains_up(std::string key) {
    return current_scope->contains_up(key);
}

std::string SymbolTableContainer::get_type(std::string key) {
    return current_scope->table[key];
}

void SymbolTableContainer::insert_fun(std::string key, std::vector<NodeDecl::DataType> args) {
    fun_table[key] = args;
}

bool SymbolTableContainer::check_fun(std::string key, std::vector<NodeDecl::DataType> args) {
    if (fun_table.find(key) == fun_table.end()) {
        return false;
    } else {
        std::vector<NodeDecl::DataType> arg_types = fun_table[key];
        if (arg_types.size() != args.size()) {
            return false;
        } else {
            for (int i = 0; i < (int)arg_types.size(); i++) {
                switch(arg_types[i]) {
                    case NodeDecl::DataType::SHORT:
                        if (args[i] != NodeDecl::DataType::SHORT) {
                            return false;
                        }
                        break;
                    case NodeDecl::DataType::INT:
                        if (args[i] == NodeDecl::DataType::LONG) {
                            return false;
                        }
                        break;
                    case NodeDecl::DataType::LONG:
                        break;
                }
            }
            return true;
        }
    }
}

bool SymbolTableContainer::check_fun(std::string key, int num_args) {
    if (fun_table.find(key) == fun_table.end()) {
        return false;
    } else {
        std::vector<NodeDecl::DataType> arg_types = fun_table[key];
        return (int)arg_types.size() == num_args;
    }
}
