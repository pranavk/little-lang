#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <llvm/IR/Value.h>

#include "consts.hpp"
#include "visitor.hpp"

namespace AST {
    class BaseStmt {
        public:
        virtual ~BaseStmt() = default;
        llvm::Value* llvmVal = nullptr;
        virtual void accept(Visitor::BaseVisitor* v)  { };
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
        std::unique_ptr<StmtBlockStmt> body;

        FunctionDefinition(std::unique_ptr<FunctionPrototype> proto,
                           std::unique_ptr<StmtBlockStmt> body)
            : proto(std::move(proto)), body(std::move(body)) { }

        void print();
    };

    class Program {
        public:
        std::vector<std::unique_ptr<AST::FunctionDefinition>> fnDefinitions;
    };

    class BaseValue {
        public:
        virtual ~BaseValue() = default;

        virtual bool isArrayValue() { return false; }
        virtual bool isIntValue() { return false; }
        virtual bool isBoolValue() { return false; }
        virtual bool isStringValue() { return false; }
        virtual bool isVoidValue() { return false; }

        virtual Token getType() { return Token::Type_void; }
    };

    class StringValue : public BaseValue {
        public:
        bool isStringValue() override { return true; }
        Token getType() override { return Token::Literal_string; }
    };

    class ArrayValue : public BaseValue {
        public:
        bool isArrayValue() override { return true; }
        Token getType() override { return Token::Type_array; }
    };

    class BoolValue : public BaseValue {
        public:
        bool isBoolValue() override { return true; }
        Token getType() override { return Token::Type_bool; }
    };

    class IntValue : public BaseValue {
        public:
        bool isIntValue() override { return true; }
        Token getType() override { return Token::Type_int; }
    };

    class VoidValue : public BaseValue {
        public:
        bool isVoidValue() override { return true; }
        Token getType() override { return Token::Type_void; }
    };

    std::unique_ptr<BaseValue> createValue(Token type);

    // various types of statements
    class BaseExpr : public BaseStmt {
        public:
        std::unique_ptr<BaseValue> result;
        BaseExpr(std::unique_ptr<BaseValue> result)
            : result(std::move(result)) { }
        BaseExpr() {}

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    // expr -> [ID]
    class IdExpr : public BaseExpr {
        public:
        std::string name;
        IdExpr(std::string& name) : name(name) { }
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    // stmt -> { stmt* }
    class StmtBlockStmt : public BaseStmt {
        public:
        std::vector<std::unique_ptr<AST::BaseStmt>> stmt_list;

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
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

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class ArrayDeclStmt : public BaseStmt {
        public:
        std::vector<ArrayDecl> decls;

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class PrintStmt : public BaseStmt {
        public:
        std::vector<std::unique_ptr<BaseExpr>> args;
        PrintStmt(std::vector<std::unique_ptr<BaseExpr>>& vec);

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class IdExpr;

    class IfStmt : public BaseStmt {
        public:
        std::unique_ptr<BaseExpr> cond;
        std::unique_ptr<StmtBlockStmt> trueStmt;
        std::unique_ptr<StmtBlockStmt> falseStmt;
        IfStmt(std::unique_ptr<BaseExpr>& cond,
               std::unique_ptr<StmtBlockStmt>& trueStmt,
               std::unique_ptr<StmtBlockStmt>& falseStmt)
               : cond(std::move(cond))
               , trueStmt(std::move(trueStmt))
               , falseStmt(std::move(falseStmt)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class WhileStmt : public BaseStmt {
        public:
        std::unique_ptr<BaseExpr> cond;
        std::unique_ptr<StmtBlockStmt> body;
        public:
        WhileStmt(std::unique_ptr<BaseExpr>& cond,
                  std::unique_ptr<StmtBlockStmt>& body)
                  : cond(std::move(cond))
                  , body(std::move(body)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class ForStmt : public BaseStmt {
        public:
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

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class ReturnStmt : public BaseStmt {
        public:
        std::unique_ptr<BaseExpr> returnExpr;
        public:
        ReturnStmt(std::unique_ptr<BaseExpr>& returnExpr)
                    : returnExpr(std::move(returnExpr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class AbortStmt : public BaseStmt {
        public:
        std::vector<std::unique_ptr<BaseExpr>> args;
        AbortStmt(std::vector<std::unique_ptr<BaseExpr>>& vec);

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class ArrayAssignment : public BaseStmt {
        public:
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

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class VarAssignment : public BaseStmt {
        public:
        // name = expr
        std::string name;
        std::unique_ptr<BaseExpr> expr;
        public:
        VarAssignment(std::string& name,
                      std::unique_ptr<BaseExpr>& expr)
                      : name(name)
                      , expr(std::move(expr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    // expr -> [number]
    class NumExpr : public BaseExpr {
        public:
        int val;
        NumExpr(int val) : val(val) {}
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class TrueExpr : public BaseExpr {
        public:
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class FalseExpr : public BaseExpr {
        public:
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class LiteralExpr : public BaseExpr {

    };

    class StringLiteralExpr : public BaseExpr {
        public:
        std::string val;
        StringLiteralExpr(std::string& name)
            : val(name) { }
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class TernaryExpr : public BaseExpr {
        public:
        std::unique_ptr<BaseExpr> condExpr;
        std::unique_ptr<BaseExpr> trueExpr;
        std::unique_ptr<BaseExpr> falseExpr;
        TernaryExpr(std::unique_ptr<BaseExpr> condExpr,
                    std::unique_ptr<BaseExpr> trueExpr,
                    std::unique_ptr<BaseExpr> falseExpr)
                    : condExpr(std::move(condExpr))
                    , trueExpr(std::move(trueExpr))
                    , falseExpr(std::move(falseExpr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class BinopExpr : public BaseExpr {
        public:
        std::unique_ptr<BaseExpr> leftExpr;
        Token op;
        std::unique_ptr<BaseExpr> rightExpr;
        BinopExpr(std::unique_ptr<BaseExpr> leftExpr,
                  Token op,
                  std::unique_ptr<BaseExpr> rightExpr)
                  : leftExpr(std::move(leftExpr))
                  , op(op)
                  , rightExpr(std::move(rightExpr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class UnaryExpr : public BaseExpr {
        public:
        Token type;
        std::unique_ptr<BaseExpr> expr;
        // (type expr)
        UnaryExpr(Token type,
                  std::unique_ptr<BaseExpr>& expr)
                  : type(type)
                  , expr(std::move(expr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class SizeofExpr : public BaseExpr {
        public:
        std::unique_ptr<AST::BaseExpr> idExpr;
        public:
        SizeofExpr(std::unique_ptr<BaseExpr>& idExpr)
            : idExpr(std::move(idExpr)) { }

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    class InputExpr : public BaseExpr {
        public:
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    // expr -> arr[expr]
    class ArrayExpr : public BaseExpr {
        public:
        std::string name;
        std::unique_ptr<BaseExpr> expr;
        public:
        ArrayExpr(std::string name, std::unique_ptr<BaseExpr> expr)
                : name(name)
                , expr(std::move(expr)) { }
        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };

    // expr -> fun(bool a, int b)
    class FnCallExpr : public BaseExpr {
        public:
        std::string name; // fn name
        std::vector<std::unique_ptr<BaseExpr>> fnArgs;
        public:
        FnCallExpr(std::string name,
                   std::vector<std::unique_ptr<BaseExpr>>& fnArgs1);

        void accept(Visitor::BaseVisitor* v) override { v->visit(this); }
    };
}