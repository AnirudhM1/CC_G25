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
