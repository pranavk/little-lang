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

    class FnParam {
        public:
        Token type;
        std::string name;
    };

    class FunctionPrototype {
        public:
        Token fnType;
        std::string fnName;
        std::vector<FnParam> fnParams;

        FunctionPrototype(int fnType, 
                          std::string& name, 
                          std::vector<FnParam>& params)
            : fnType(static_cast<Token>(fnType)), 
              fnName(name), 
              fnParams(params) { }
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
            for (auto arg: proto->fnParams) {
                std::cout << arg.name << " " << static_cast<int>(arg.type);
                std::cout << std::endl;
            }
        }
    };

    class BaseValue {
        public:
        virtual ~BaseValue() = default;
    };

    class StringValue : public BaseValue {
        std::string value;
        public:
        StringValue(std::string& value) : value(value) { }
    };

    class BoolValue : public BaseValue {
        bool value;
        public:
        BoolValue(bool value) : value(value) { }
    };

    class IntValue : public BaseValue {
        int value;
        public:
        IntValue(int value) : value(value) { }
    };

    // various types of statements
    class BaseExpr : public BaseStmt {
        std::unique_ptr<BaseValue> result;
        public:
        BaseExpr(std::unique_ptr<BaseValue> result)
            : result(std::move(result)) { }
        BaseExpr() {}
    };

    // expr -> [ID]
    class IdExpr : public BaseExpr {
        std::string name;
        public:
        IdExpr(std::string& name) : name(name) { }
    };

    // stmt -> { stmt* }
    class StmtBlockStmt : public BaseStmt {
        public:
        std::vector<std::unique_ptr<AST::BaseStmt>> stmt_list;
    };

    class VarDecl {
        public:
        Token type;
        std::string name;
    };

    class ArrayExpr;
    // array id[expr]
    class ArrayDecl {
        public:
        std::string name;
        std::unique_ptr<BaseExpr> expr;
    };

    class VarDeclStmt : public BaseStmt {
        public:
        std::vector<VarDecl> decls;
    };

    class ArrayDeclStmt : public BaseStmt {
        public:
        std::vector<ArrayDecl> decls;
    };

    class PrintStmt : public BaseStmt {
        std::vector<std::unique_ptr<BaseExpr>> args;
    };

    class IdExpr;

    class IfStmt : public BaseStmt {
        std::unique_ptr<BaseExpr> cond;
        std::unique_ptr<StmtBlockStmt> trueStmt;
        std::unique_ptr<StmtBlockStmt> falseStmt;
        public:
        IfStmt(std::unique_ptr<BaseExpr>& cond,
               std::unique_ptr<StmtBlockStmt>& trueStmt,
               std::unique_ptr<StmtBlockStmt>& falseStmt)
               : cond(std::move(cond))
               , trueStmt(std::move(trueStmt))
               , falseStmt(std::move(falseStmt)) { }
    };

    class WhileStmt : public BaseStmt {
        std::unique_ptr<BaseExpr> cond;
        std::unique_ptr<StmtBlockStmt> body;
        public:
        WhileStmt(std::unique_ptr<BaseExpr>& cond,
                  std::unique_ptr<StmtBlockStmt>& body)
                  : cond(std::move(cond))
                  , body(std::move(body)) { }
    };

    class ForStmt : public BaseStmt {
        std::unique_ptr<BaseExpr> ident;
        std::unique_ptr<BaseExpr> container;
        std::unique_ptr<StmtBlockStmt> body;
        public:
        ForStmt(std::unique_ptr<BaseExpr>& ident,
                std::unique_ptr<BaseExpr>& container,
                std::unique_ptr<StmtBlockStmt>& body)
                : ident(std::move(ident))
                , container(std::move(container))
                , body(std::move(body)) { }
    };

    class ReturnStmt : public BaseStmt {
        std::unique_ptr<BaseExpr> returnExpr;
        public:
        ReturnStmt(std::unique_ptr<BaseExpr>& returnExpr)
                    : returnExpr(std::move(returnExpr)) { }
    };

    class ArrayAssignment : public BaseStmt {
        // name[idxExpr] = expr
        std::string name;
        std::unique_ptr<BaseExpr> idxExpr;
        std::unique_ptr<BaseExpr> expr;
        public:
        ArrayAssignment(std::string& name, 
                        std::unique_ptr<BaseExpr>& idxExpr,
                        std::unique_ptr<BaseExpr>& expr)
                        : name(name)
                        , idxExpr(std::move(idxExpr))
                        , expr(std::move(expr)) { }
    };

    class VarAssignment : public BaseStmt {
        // name = expr
        std::string name;
        std::unique_ptr<BaseExpr> expr;
        public:
        VarAssignment(std::string& name,
                      std::unique_ptr<BaseExpr>& expr)
                      : name(name)
                      , expr(std::move(expr)) { }
    };

    // expr -> [number]
    class NumExpr : public BaseExpr {
    };

    class LiteralExpr : public BaseExpr {
    
    };

 
    class TernaryExpr : public BaseExpr {
        std::unique_ptr<BaseExpr> condExpr;
        std::unique_ptr<BaseExpr> trueExpr;
        std::unique_ptr<BaseExpr> falseExpr;
        public:
        TernaryExpr(std::unique_ptr<BaseExpr> condExpr,
                    std::unique_ptr<BaseExpr> trueExpr,
                    std::unique_ptr<BaseExpr> falseExpr)
                    : condExpr(std::move(condExpr))
                    , trueExpr(std::move(trueExpr))
                    , falseExpr(std::move(falseExpr)) { }
    };

    class BinopExpr : public BaseExpr {
        std::unique_ptr<BaseExpr> leftExpr;
        std::unique_ptr<BaseExpr> rightExpr;
        public:
        BinopExpr(std::unique_ptr<BaseExpr> leftExpr,
                  std::unique_ptr<BaseExpr> rightExpr) 
                  : leftExpr(std::move(leftExpr))
                  , rightExpr(std::move(rightExpr)) { }
    };

    class UnaryExpr : public BaseExpr {
        Token type;
        std::unique_ptr<BaseExpr> expr;
        // (type expr)
        public:
        UnaryExpr(Token type,
                  std::unique_ptr<BaseExpr>& expr)
                  : type(type)
                  , expr(std::move(expr)) { }
    };

    class SizeofExpr : public BaseExpr {
        std::string name;
        public:
        SizeofExpr(std::string name) : name(name) { }
    };

    class InputExpr : public BaseExpr {

    };

    // expr -> arr[expr]
    class ArrayExpr : public BaseExpr {
        std::string name;
        std::unique_ptr<BaseExpr> expr;
        public:
        ArrayExpr(std::string name, std::unique_ptr<BaseExpr> expr)
                : name(name)
                , expr(std::move(expr)) { }
    };

    // expr -> fun(bool a, int b)
    class FnCallExpr : public BaseExpr {
        std::string name; // fn name
        std::vector<std::unique_ptr<BaseExpr>> fnArgs;
        public:
        FnCallExpr(std::string name, 
                   std::vector<std::unique_ptr<BaseExpr>>& fnArgs1)
                   : name(name)
                   {
                       fnArgs.clear();
                       for (int i = 0; i < fnArgs1.size(); i++) 
                       {
                           fnArgs.push_back(std::move(fnArgs1[i]));
                       }
                   }
    };
}