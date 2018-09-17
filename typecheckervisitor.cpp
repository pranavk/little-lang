#include <iostream>
#include <cassert>

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
        // check if this expression is an int value
        if (!var.expr->result->isIntValue())
            throw Exception("Expected an expression with value int");
        if (!_symTab->addSymbol(var.name, {SymbolType::Array}))
            throw Exception("redeclaring array " + var.name, ExceptionType::Type);
    } 
}

void Visitor::TypecheckerVisitor::visit(AST::PrintStmt *stmt)
{
    for (auto& expr : stmt->args) 
    {
        expr->accept(this);
        if (!expr->result->isStringValue() && !expr->result->isIntValue()) 
            throw Exception("only string literals and int identifiers allowed in print args", ExceptionType::Type);
    }
}

void Visitor::TypecheckerVisitor::visit(AST::IfStmt *stmt)
{
    stmt->cond->accept(this);
    if (!stmt->cond->result->isBoolValue())
        throw Exception("Only bool value allowed as if stmt condition.", ExceptionType::Type);

    stmt->trueStmt->accept(this);

    if (stmt->falseStmt)
    {
        stmt->falseStmt->accept(this);  
    }
}

void Visitor::TypecheckerVisitor::visit(AST::WhileStmt *stmt)
{
    stmt->cond->accept(this);
    if (!stmt->cond->result->isBoolValue())
        throw Exception("Only bool value allowed as while stmt condition.", ExceptionType::Type);
     
    stmt->body->accept(this);
}


void Visitor::TypecheckerVisitor::visit(AST::ForStmt *stmt)
{
    stmt->ident->accept(this);
    if (!stmt->ident->result->isIntValue())
        throw Exception("Id expected in for stmt before colon", ExceptionType::Type);
    
    stmt->container->accept(this);
    if (!stmt->container->result->isArrayValue())
        throw Exception("array expected in for stmt after colon", ExceptionType::Type);
    
    stmt->body->accept(this);
}

void Visitor::TypecheckerVisitor::visit(AST::ReturnStmt *stmt)
{
    SymbolType retExprType = SymbolType::Void;
    SymbolType expectedRetType = _symTab->getEnclosingFnRetType();
    if (stmt->returnExpr)
    {
        stmt->returnExpr->accept(this);
        auto m = stmt->returnExpr->result->getType();
        retExprType = TokenToSymbolType(m);
    }

    if (retExprType != expectedRetType)
        throw Exception("Return type mismatch from fn def", ExceptionType::Type);
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
    expr->result = std::make_unique<AST::IntValue>();
}

void Visitor::TypecheckerVisitor::visit(AST::IdExpr *expr)
{
    SymbolInfo* sym = nullptr;
    if (!(sym = _symTab->hasSymbol(expr->name)))
        throw Exception("undefined symbol " + expr->name, ExceptionType::Type);

    expr->result = AST::createValue(SymbolToTokenType(sym->_type));
}

void Visitor::TypecheckerVisitor::visit(AST::LiteralExpr *expr)
{
    std::cout << "Literal expr" << std::endl;
}

void Visitor::TypecheckerVisitor::visit(AST::StringLiteralExpr *expr)
{
    expr->result = std::make_unique<AST::StringValue>();
}

void Visitor::TypecheckerVisitor::visit(AST::TernaryExpr* expr)
{
    expr->condExpr->accept(this);
    if (!expr->condExpr->result->isBoolValue())
        throw Exception("first arg to ternary expr should be bool type");   
    
    expr->trueExpr->accept(this);
    Token trueType = expr->trueExpr->result->getType();

    expr->falseExpr->accept(this);
    Token falseType = expr->falseExpr->result->getType();

    if (trueType != falseType)
        throw Exception("ternary: true and false expression should be same type");
    
    expr->result = AST::createValue(trueType);
}

void Visitor::TypecheckerVisitor::visit(AST::BinopExpr* expr)
{
    const Token op = expr->op;

    expr->leftExpr->accept(this);
    Token lType = expr->leftExpr->result->getType();

    expr->rightExpr->accept(this);
    Token rType = expr->rightExpr->result->getType();

    if (lType != rType) {
        throw Exception("binop operands's type mismatch.");
    }
    if (isCompOp(op))
        expr->result = AST::createValue(Token::Type_bool);
    else if (isBinOp(op))
        expr->result = AST::createValue(Token::Type_int);
}

void Visitor::TypecheckerVisitor::visit(AST::UnaryExpr *expr)
{
    const Token op = expr->type;
    expr->expr->accept(this);
    if (op == Token::Op_bang) {
        if (!expr->expr->result->isBoolValue())
            throw Exception("Bool expected with bang operator");
    } else if (op == Token::Op_minus) {
        if (!expr->expr->result->isIntValue())
            throw Exception("Only int allowed with minus unary op");
    } else
        assert("Wrong operator found in unary Expr.");

    expr->result = AST::createValue(expr->expr->result->getType());
}

void Visitor::TypecheckerVisitor::visit(AST::SizeofExpr *expr)
{
    expr->idExpr->accept(this);
    if (!expr->idExpr->result->isArrayValue())
        throw Exception("Sizeof argument should be an array value");
}

void Visitor::TypecheckerVisitor::visit(AST::InputExpr *expr)
{
    
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayExpr *expr)
{
    expr->expr->accept(this);
    if (!expr->expr->result->isIntValue())   
        throw Exception("Only int allowed as array expression argument");
    expr->result = AST::createValue(Token::Type_array);
}

void Visitor::TypecheckerVisitor::visit(AST::FnCallExpr *expr)
{
    if (!_symTab->hasSymbol(expr->name))
        throw Exception("calling undefined function : " + expr->name, ExceptionType::Type);

    for (auto& arg: expr->fnArgs)
    {
        arg->accept(this); // TODO: check arg with fn definition formal params types
    }
    SymbolType fnRetType = _symTab->getFnRetType(expr->name);
    expr->result = AST::createValue(SymbolToTokenType(fnRetType));
}

void Visitor::TypecheckerVisitor::visit(AST::Program* program)
{
    // add functions to symbol table in first pass
    for (auto &fnDef : program->fnDefinitions)
    {
        if (!_symTab->addSymbol(fnDef->proto->fnName, 
                                {SymbolType::Function, TokenToSymbolType(fnDef->proto->fnType)}))
            throw Exception("redefining function :" + fnDef->proto->fnName, ExceptionType::Type);
    }

    // exactly one main should be there.
    if (!_symTab->hasSymbolInCurrentScope("main"))
        throw Exception("one main function required", ExceptionType::Type);

    // iterate over individual funcs
    for (auto& fnDef : program->fnDefinitions)
    {
        // following hack is because return statement wants to know the type of the function in which it's enclosed in
        // And we cannot make use of top-level symbol table (which contains all the functions) because we don't know the name of the function
        // in which the return statement is enclosed.
        // functions have a completely new scope with only internal function name in that scope
        EnterScope();
        _symTab->addSymbol("lilfn_ " + fnDef->proto->fnName, 
                                {SymbolType::Function, 
                                TokenToSymbolType(fnDef->proto->fnType)});
        EnterScope();
        for(auto& param : fnDef->proto->fnParams)
        {
            if(!_symTab->addSymbol(param.name, {TokenToSymbolType(param.type)}))
                throw Exception("redeclaring variable : " + param.name, ExceptionType::Type);
        }
        fnDef->body->accept(this);
        LeaveScope();
        LeaveScope();
    }
}