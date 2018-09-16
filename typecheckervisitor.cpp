#include <iostream>

#include "baseast.hpp"
#include "visitor.hpp"
#include "symtab.hpp"

void Visitor::TypecheckerVisitor::visit(AST::StmtBlockStmt *stmtBlock)
{
    EnterScope();
    for (auto &stmt : stmtBlock->stmt_list)
    {
        stmt->accept(this);
    }
    LeaveScope();
}

void Visitor::TypecheckerVisitor::visit(AST::VarDeclStmt *stmt)
{
    for (auto& var : stmt->decls) 
    {
        if (!_symTab->addSymbol(var.name, {TokenToSymbolType(var.type)}))
            throw Exception("redeclaring variable " + var.name, ExceptionType::Type);
    }
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayDeclStmt *stmt)
{ 
    for (auto& var : stmt->decls) 
    {
        var.expr->accept(this);
        if (!_symTab->addSymbol(var.name, {SymbolType::Array}))
            throw Exception("redeclaring array " + var.name, ExceptionType::Type);
    } 
}

void Visitor::TypecheckerVisitor::visit(AST::PrintStmt *stmt)
{
    for (auto& expr : stmt->args) 
    {
        expr->accept(this);
    }
   
}

void Visitor::TypecheckerVisitor::visit(AST::IfStmt *stmt)
{
    stmt->cond->accept(this);
 
    stmt->trueStmt->accept(this);
    

    if (stmt->falseStmt)
    {
        
        
        stmt->falseStmt->accept(this);
        
    }
}

void Visitor::TypecheckerVisitor::visit(AST::WhileStmt *stmt)
{
    
    
    
    stmt->cond->accept(this);
    

    
    
    stmt->body->accept(this);
    
}


void Visitor::TypecheckerVisitor::visit(AST::ForStmt *stmt)
{
    
    
    
    
    stmt->ident->accept(this);
    

    
    
    stmt->container->accept(this);
    

    
    
    stmt->body->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::ReturnStmt *stmt)
{
    

    if (stmt->returnExpr)
    {
        
        
        stmt->returnExpr->accept(this);
        
    }
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayAssignment *stmt)
{
    stmt->idxExpr->accept(this);
    

    

    
    
    stmt->expr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::VarAssignment *stmt)
{
    
    

    
    
    stmt->expr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::BaseExpr* expr)
{
    
}

void Visitor::TypecheckerVisitor::visit(AST::NumExpr *expr)
{
}

void Visitor::TypecheckerVisitor::visit(AST::IdExpr *expr)
{
    if (!_symTab->hasSymbol(expr->name))
        throw Exception("undefined symbol " + expr->name, ExceptionType::Type);
}

void Visitor::TypecheckerVisitor::visit(AST::LiteralExpr *expr)
{
    std::cout << "Literal expr" << std::endl;
}

void Visitor::TypecheckerVisitor::visit(AST::StringLiteralExpr *expr)
{
}

void Visitor::TypecheckerVisitor::visit(AST::TernaryExpr* expr)
{
    
    
    
    expr->condExpr->accept(this);
    

    
    
    expr->trueExpr->accept(this);
    

    
    
    expr->falseExpr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::BinopExpr* expr)
{
    
    
    
    expr->leftExpr->accept(this);
    

    

    
    
    expr->rightExpr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::UnaryExpr *expr)
{
    
    

    
    expr->expr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::SizeofExpr *expr)
{
    
    

    
    expr->idExpr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::InputExpr *expr)
{
    
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayExpr *expr)
{
    
    

    
    
    expr->expr->accept(this);
    
}

void Visitor::TypecheckerVisitor::visit(AST::FnCallExpr *expr)
{
    if (!_symTab->hasSymbol(expr->name))
        throw Exception("calling undefined function : " + expr->name, ExceptionType::Type);

    for (auto& arg: expr->fnArgs)
    {
        arg->accept(this);   
    }
}

void Visitor::TypecheckerVisitor::visit(AST::Program* program)
{
    // add functions to symbol table in first pass
    for (auto &fnDef : program->fnDefinitions)
    {
        if (!_symTab->addSymbol(fnDef->proto->fnName, {SymbolType::Function}))
            throw Exception("redefining function :" + fnDef->proto->fnName, ExceptionType::Type);
    }

    // iterate over individual funcs
    for (auto& fnDef : program->fnDefinitions)
    {
        // add formal params to symbol table too
        EnterScope();
        for(auto& param : fnDef->proto->fnParams)
        {
            if(!_symTab->addSymbol(param.name, {TokenToSymbolType(param.type)}))
                throw Exception("redeclaring variable : " + param.name, ExceptionType::Type);
        }
        fnDef->body->accept(this);
        LeaveScope();
    }
}