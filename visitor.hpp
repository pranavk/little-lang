#pragma once

#include <iostream>
#include <cassert>
#include <unordered_map>
#include <memory>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include "consts.hpp"
#include "symtab.hpp"

namespace AST {
    class FunctionDefinition;
    class Program;

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
    class AbortStmt;
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
        virtual void visit(AST::AbortStmt*) = 0;
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

        virtual void visit(AST::Program*) = 0;
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
        virtual void visit(AST::AbortStmt*) override;
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

        virtual void visit(AST::Program*) override;
    };

    class TypecheckerVisitor : public BaseVisitor {
        std::unique_ptr<SymbolTable> _symTab;

        public:
        virtual void visit(AST::BaseStmt* stmt) { std::cout << "Typechecker visitor " << std::endl; };
        virtual void visit(AST::StmtBlockStmt* stmtBlock) override;
        virtual void visit(AST::VarDeclStmt*) override;
        virtual void visit(AST::ArrayDeclStmt*) override;
        virtual void visit(AST::PrintStmt* stmt) override;
        virtual void visit(AST::IfStmt*) override;
        virtual void visit(AST::WhileStmt*) override;
        virtual void visit(AST::ForStmt*) override;
        virtual void visit(AST::ReturnStmt*) override;
        virtual void visit(AST::AbortStmt*) override;
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

        virtual void visit(AST::Program*) override;

        TypecheckerVisitor() {
            _symTab = std::make_unique<SymbolTable>(nullptr);
        }
        ~TypecheckerVisitor() {

        }
        void EnterScope() {
            std::unique_ptr<SymbolTable> newSymTab = std::make_unique<SymbolTable>(std::move(_symTab));
            _symTab = std::move(newSymTab);
        }

        void LeaveScope() {
            _symTab = std::move(_symTab->getParent());
        }
    };

     class CodegenVisitor : public BaseVisitor {
        static llvm::LLVMContext _TheContext;
        static llvm::IRBuilder<> _Builder;
        static std::unique_ptr<llvm::Module> _TheModule;
        static std::map<std::string, llvm::AllocaInst*> _NamedValues;

        public:
        virtual void visit(AST::BaseStmt* stmt) { std::cout << "Typechecker visitor " << std::endl; };
        virtual void visit(AST::StmtBlockStmt* stmtBlock) override;
        virtual void visit(AST::VarDeclStmt*) override;
        virtual void visit(AST::ArrayDeclStmt*) override;
        virtual void visit(AST::PrintStmt* stmt) override;
        virtual void visit(AST::IfStmt*) override;
        virtual void visit(AST::WhileStmt*) override;
        virtual void visit(AST::ForStmt*) override;
        virtual void visit(AST::ReturnStmt*) override;
        virtual void visit(AST::AbortStmt*) override;
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

        virtual void visit(AST::Program*) override;

        static llvm::Type* CreateLLVMType(const Token type);

        static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func,
                                                        const std::string& varName,
                                                        const Token type);

        static llvm::Module* getModule() { return _TheModule.get(); }
    };
}