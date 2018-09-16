#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "consts.hpp"

enum class SymbolType {
    Err      = -1,
    Void     = 0,

    Function = 1,

    Array    = 2,
    Int      = 3,
    Bool     = 4,
};

SymbolType TokenToSymbolType(Token type);

struct SymbolInfo
{
    SymbolType _type;
};

class SymbolTable
{
    std::unique_ptr<SymbolTable> _parent;
 //   std::unordered_set<std::unique_ptr<SymbolTable>> _children;
    std::unordered_map<std::string, SymbolInfo> _table;

  public:
    SymbolTable(std::unique_ptr<SymbolTable>&& parent) 
                    : _parent(std::move(parent)) { 
        if (_parent) {
     //       _parent->addChild();
        }
    }

    std::unique_ptr<SymbolTable>& getParent() {
        return _parent;
    }

    void addChild(std::unique_ptr<SymbolTable>& child) {
  //      _children.insert(std::move(child));
    }

    bool hasSymbolInCurrentScope(const std::string& name) {
        // only check for symbol existence in current scope
        return _table.find(name) != _table.end();
    }

    bool hasSymbol(const std::string& name) {
        // check if this symbol exists in our symbol table chain
        SymbolTable* symTab = this;
        do {
            if (symTab->hasSymbolInCurrentScope(name)) {
                return true;
            }

            symTab = symTab->getParent().get();
        } while (symTab != nullptr);

        return false;
    }

    bool addSymbol(const std::string& name, SymbolInfo&& info) {
        if (!hasSymbol(name)) {
            auto res = _table.emplace(name, info);
            return res.second;
        }
        return false;
    }
};