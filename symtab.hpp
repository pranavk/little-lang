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
Token      SymbolToTokenType(SymbolType type);

struct SymbolInfo
{
    SymbolType _type;
    SymbolType _returnType; // only used if _type = Function.
};

class SymbolTable
{
    std::unique_ptr<SymbolTable> _parent;
    std::unordered_map<std::string, SymbolInfo> _table;

  public:
    SymbolTable(std::unique_ptr<SymbolTable>&& parent) 
                    : _parent(std::move(parent)) { 
    }

    std::unique_ptr<SymbolTable>& getParent() {
        return _parent;
    }

    void addChild(std::unique_ptr<SymbolTable>& child) {
  //      _children.insert(std::move(child));
    }

    SymbolInfo* hasSymbolInCurrentScope(const std::string& name) {
        // only check for symbol existence in current scope
        SymbolInfo* res = nullptr;
        auto it = _table.find(name);
        if (it != _table.end()) {
            res = &it->second;
        }

        return res;
    }

    SymbolInfo* hasSymbol(const std::string& name) {
        // check if this symbol exists in our symbol table chain
        SymbolInfo* res = nullptr;
        SymbolTable* symTab = this;
        do {
            res = symTab->hasSymbolInCurrentScope(name);
            symTab = symTab->getParent().get();
        } while (!res && symTab != nullptr);

        return res;
    }

    SymbolType getEnclosingFnRetType() {
        SymbolTable* symTab = this;
        while (symTab->getParent().get() != nullptr) {
            if (symTab->_table.size() == 1) {
                auto it = symTab->_table.begin();
                if (it->first.substr(0, 6) == "lilfn_") {
                    return it->second._returnType;
                }
            }

            symTab = symTab->getParent().get();
        }

        return SymbolType::Err;
    }

    bool addSymbol(const std::string& name, SymbolInfo&& info) {
        if (!hasSymbol(name)) {
            auto res = _table.emplace(name, info);
            return res.second;
        }
        return false;
    }
};