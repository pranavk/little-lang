#pragma once

#include <memory>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

#include "consts.hpp"

namespace AST {
    class FunctionPrototype;
}

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

class SymbolInfo
{
    public:
    SymbolType _type;
    virtual AST::FunctionPrototype* getFnProto() { return nullptr; }
    SymbolInfo(SymbolType type) : _type(type) { }
};

class FnSymbolInfo : public SymbolInfo {
    AST::FunctionPrototype* _proto;

    public:
    AST::FunctionPrototype* getFnProto() override { return _proto; }
    FnSymbolInfo(AST::FunctionPrototype* proto)
            : _proto(proto)
            , SymbolInfo(SymbolType::Function) { }
};

class SymbolTable
{
    std::unique_ptr<SymbolTable> _parent;
    std::unordered_map<std::string, std::unique_ptr<SymbolInfo>> _table;

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
            res = it->second.get();
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

    AST::FunctionPrototype* getFnProto(const std::string& name) {
        // go to the top level sym table.
        SymbolTable* symTab = this;
        while (symTab->getParent().get() != nullptr) symTab = symTab->getParent().get();

        auto it = symTab->_table.find(name);
        if (it == symTab->_table.end() || it->second->_type != SymbolType::Function)
            throw Exception("Cannot find function name : " + name);
        assert(it->second->getFnProto());
        return it->second->getFnProto();
    }

    AST::FunctionPrototype* getEnclosingFnProto() {
        SymbolTable* symTab = this;
        while (symTab->getParent().get() != nullptr) {
            if (symTab->_table.size() == 1) {
                auto it = symTab->_table.begin();
                if (it->first.substr(0, 6) == "lilfn_") {
                    assert(it->second->getFnProto());
                    return it->second->getFnProto();
                }
            }

            symTab = symTab->getParent().get();
        }

        return nullptr;
    }

    bool addFnSymbol(const std::string& name, AST::FunctionPrototype* proto) {
        if (!hasSymbol(name)) {
            auto res = _table.emplace(name, std::make_unique<FnSymbolInfo>(proto));
            return res.second;
        }
        return false;
    }

    bool addSymbol(const std::string& name, Token type) {
        if (!hasSymbol(name)) {
            auto res = _table.emplace(name, std::make_unique<SymbolInfo>(TokenToSymbolType(type)));
            return res.second;
        }
        return false;
    }
};