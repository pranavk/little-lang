#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace AST {
    class BaseStmt {
        public:
        virtual ~BaseStmt() = default;
    };

    class FnArg {
        std::string type;
        std::string name;
    };

    class FunctionPrototype {
        std::string fnName;
        std::vector<FnArg> fnArgs;
    };

    class FunctionDefinition {
        FunctionPrototype proto;
        BaseStmt body;

        FunctionDefinition(FunctionPrototype& proto, BaseStmt& body)
        : proto(proto), body(body) { }
    };

    class BaseValue {
        virtual ~BaseValue() = default;
    };

    class BoolValue : public BaseValue {
        bool value;
    };

    class IntValue : public BaseValue {
        int value;
    };

    // various types of statements
    class BaseExpr : public BaseStmt {
        BaseValue result;
        public:
        virtual ~BaseExpr() = default;
    };

    // expr -> [number]
    class NumExpr : public BaseExpr {
    };

    // expr -> [ID]
    class IdExpr : public BaseExpr {
        std::string name;
    };

    // expr -> fun(bool a, int b)
    class FnCallExpr : public BaseExpr {
        std::string name; // fn name
        std::vector<FnArg> fnArgs;
    };

    // expr -> arr[expr]
    class ArrayExpr : public BaseExpr {
        std::string name;
        BaseExpr index;
    };
}