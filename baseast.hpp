#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "consts.hpp"

namespace AST {
    class BaseStmt {
        public:
        virtual ~BaseStmt() = default;
    };

    class FnArg {
        public:
        Token type;
        std::string name;
    };

    class FunctionPrototype {
        public:
        Token fnType;
        std::string fnName;
        std::vector<FnArg> fnArgs;

        FunctionPrototype(int fnType, 
                          std::string& name, 
                          std::vector<FnArg>& args)
            : fnType(static_cast<Token>(fnType)), 
              fnName(name), 
              fnArgs(args) { }
    };

    class FunctionDefinition {
        public:
        std::unique_ptr<FunctionPrototype> proto;
        std::unique_ptr<BaseStmt> body;

        FunctionDefinition(std::unique_ptr<FunctionPrototype> proto, 
                           std::unique_ptr<BaseStmt> body)
            : proto(std::move(proto)), body(std::move(body)) { }
    
        void print()
        {
            std::cout  << std::endl << "--------" << std::endl;
            std::cout << "FnName: " << proto->fnName << std::endl;
            std::cout << "FnType: " << static_cast<int>(proto->fnType) << std::endl;
            std::cout << "Args :" << std::endl;
            for (auto arg: proto->fnArgs) {
                std::cout << arg.name << " " << static_cast<int>(arg.type);
                std::cout << std::endl;
            }
        }
    };

    class BaseValue {
        public:
        virtual ~BaseValue() = default;
    };

    class BoolValue : public BaseValue {
        bool value;
        virtual ~BoolValue() = default;
    };

    class IntValue : public BaseValue {
        int value;
        public:
        virtual ~IntValue() = default;
    };

    // various types of statements
    class BaseExpr : public BaseStmt {
        BaseValue result;
        public:
        virtual ~BaseExpr() = default;
    };

    // stmt -> { stmt* }
    class StmtBlockStmt : public BaseStmt {
        std::vector<BaseStmt> stmt_list;
    };

    class VarDecl {
        public:
        Token type;
        std::string name;
    };

    class ArrayDecl {
        public:
        std::string name;
        int size;
    };

    class VarDeclStmt : public BaseStmt {
        std::vector<VarDecl> decls;
    };

    class ArrayDeclStmt : public BaseStmt {
        std::vector<ArrayDecl> decls;
    };

    // expr -> [number]
    class NumExpr : public BaseExpr {
        public:
        virtual ~NumExpr() = default;
    };

    // expr -> [ID]
    class IdExpr : public BaseExpr {
        std::string name;
        public:
        virtual ~IdExpr() = default;
    };

    // expr -> arr[expr]
    class ArrayExpr : public BaseExpr {
        std::string name;
        BaseExpr index;
        public:
        virtual ~ArrayExpr() = default;
    };

    // expr -> fun(bool a, int b)
    class FnCallExpr : public BaseExpr {
        std::string name; // fn name
        std::vector<FnArg> fnArgs;
        public:
        virtual ~FnCallExpr() = default;
    };  
}