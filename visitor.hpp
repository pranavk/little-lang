#pragma once

#include <iostream>

namespace AST {
    class FunctionDefinition;


    class BaseStmt;
    class BaseExpr;
    class StmtBlockStmt;
    class VarDeclStmt;
    class ArrayDeclStmt;
    class PrintStmt;
    class IfStmt;
    class WhileStmt;
    class ForStmt;
    class ReturnStmt;
    class ArrayAssignment;
    class VarAssignment;

    class NumExpr;
    class IdExpr;
    class LiteralExpr;
    class StringLiteralExpr;
    class TernaryExpr;
    class BinopExpr;
    class UnaryExpr;
    class SizeofExpr;
    class InputExpr;
    class ArrayExpr;
    class FnCallExpr;
};

namespace Visitor {
    class BaseVisitor {
        public:
        virtual void visit(AST::BaseStmt* stmt) { std::cout << "BaseVisitor " << std::endl; };
        virtual void visit(AST::StmtBlockStmt* stmtBlock) = 0;
        virtual void visit(AST::VarDeclStmt*) = 0;
        virtual void visit(AST::ArrayDeclStmt*) = 0;
        virtual void visit(AST::PrintStmt* stmt) = 0;
        virtual void visit(AST::IfStmt*) = 0;
        virtual void visit(AST::WhileStmt*) = 0;
        virtual void visit(AST::ForStmt*) = 0;
        virtual void visit(AST::ReturnStmt*) = 0;
        virtual void visit(AST::ArrayAssignment*) = 0;
        virtual void visit(AST::VarAssignment*) = 0;

        virtual void visit(AST::BaseExpr*) = 0;
        virtual void visit(AST::NumExpr*) = 0;
        virtual void visit(AST::IdExpr*) = 0;
        virtual void visit(AST::LiteralExpr*) = 0;
        virtual void visit(AST::StringLiteralExpr*) = 0;
        virtual void visit(AST::TernaryExpr*) = 0;
        virtual void visit(AST::BinopExpr*) = 0;
        virtual void visit(AST::UnaryExpr*) = 0;
        virtual void visit(AST::SizeofExpr*) = 0;
        virtual void visit(AST::InputExpr*) = 0;
        virtual void visit(AST::ArrayExpr*) = 0;
        virtual void visit(AST::FnCallExpr*) = 0;

        virtual void visitProgramAST(std::vector<std::unique_ptr<AST::FunctionDefinition>>& fnDefs) = 0;
    };

    class PrintASTVisitor : public BaseVisitor {
        int _depth = 0;

        void print(const std::vector<std::string>& str);

        public:
        void visit(AST::BaseStmt* stmt) { std::cout << "PrintASTVistoor" << std::endl; }
        virtual void visit(AST::StmtBlockStmt* stmtBlock) override;
        virtual void visit(AST::VarDeclStmt*) override;
        virtual void visit(AST::ArrayDeclStmt*) override;
        virtual void visit(AST::PrintStmt* stmt) override;
        virtual void visit(AST::IfStmt*) override;
        virtual void visit(AST::WhileStmt*) override;
        virtual void visit(AST::ForStmt*) override;
        virtual void visit(AST::ReturnStmt*) override;
        virtual void visit(AST::ArrayAssignment*) override;
        virtual void visit(AST::VarAssignment*) override;

        virtual void visit(AST::BaseExpr*) override;
        virtual void visit(AST::NumExpr*) override;
        virtual void visit(AST::IdExpr*) override;
        virtual void visit(AST::LiteralExpr*) override;
        virtual void visit(AST::StringLiteralExpr*) override;
        virtual void visit(AST::TernaryExpr*) override;
        virtual void visit(AST::BinopExpr*) override;
        virtual void visit(AST::UnaryExpr*) override;
        virtual void visit(AST::SizeofExpr*) override;
        virtual void visit(AST::InputExpr*) override;
        virtual void visit(AST::ArrayExpr*) override;
        virtual void visit(AST::FnCallExpr*) override;

        void visitProgramAST(std::vector<std::unique_ptr<AST::FunctionDefinition>>& fnDefs);
    };
}