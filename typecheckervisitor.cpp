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
        if (!_symTab->addSymbol(var.name, var.type))
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
        if (!_symTab->addSymbol(var.name, Token::Type_array))
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
    Token retExprType = Token::Type_void;
    auto m = _symTab->getEnclosingFnProto();
    Token expectedRetType = m->fnType;
    if (stmt->returnExpr)
    {
        stmt->returnExpr->accept(this);
        retExprType = stmt->returnExpr->result->getType();
    }

    if (retExprType != expectedRetType)
        throw Exception("Return type mismatch from fn def", ExceptionType::Type);
}

void Visitor::TypecheckerVisitor::visit(AST::AbortStmt *stmt)
{
    for (auto& expr : stmt->args)
    {
        expr->accept(this);
        if (!expr->result->isStringValue() && !expr->result->isIntValue())
            throw Exception("only string or int allowed as abort stmt arguments", ExceptionType::Type);
    }
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayAssignment *stmt)
{
    SymbolInfo* info = nullptr;
    if (!(info = _symTab->hasSymbol(stmt->name)))
        throw Exception("Assigning element to an undeclared array.");
    if (info->_type != SymbolType::Array)
        throw Exception("Only arrays can be indexed using [] operator");
    stmt->idxExpr->accept(this);
    if (!stmt->idxExpr->result->isIntValue())
        throw Exception("only int allowed as array index");
    stmt->expr->accept(this);
    if (!stmt->expr->result->isIntValue())
        throw Exception("rhs of array assignment must be int");
}

void Visitor::TypecheckerVisitor::visit(AST::VarAssignment *stmt)
{
    SymbolInfo* info = nullptr;
    if (!(info =_symTab->hasSymbol(stmt->name)))
        throw Exception("Undeclared variable is being assigned.");
    stmt->expr->accept(this);
    const Token rhsType = stmt->expr->result->getType();
    const Token lhsType = SymbolToTokenType(info->_type);
    if (lhsType != rhsType)
        throw Exception("unmatched types in var assignment.");
}

void Visitor::TypecheckerVisitor::visit(AST::BaseExpr* expr)
{

}

void Visitor::TypecheckerVisitor::visit(AST::TrueExpr *expr)
{
    expr->result = AST::createValue(Token::Type_bool);
}

void Visitor::TypecheckerVisitor::visit(AST::FalseExpr *expr)
{
    expr->result = AST::createValue(Token::Type_bool);
}

void Visitor::TypecheckerVisitor::visit(AST::NumExpr *expr)
{
    expr->result = AST::createValue(Token::Type_int);
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
        throw Exception("binop operands must be same type.");
    } else if (isAndOr(op)) {
        if (lType != Token::Type_bool)
            throw Exception("only bool operands allowed with & and |");
        expr->result = AST::createValue(Token::Type_bool);
    } else if (isEqOrNeq(op)) {
        if (lType != Token::Type_int && lType != Token::Type_bool)
            throw Exception("only int and bool allowed with == and !=");
        expr->result = AST::createValue(Token::Type_bool);
    } else if (isCompOp(op)) {
        if (lType != Token::Type_int)
            throw Exception("only int opearnds allowed with comparison operator");
        expr->result = AST::createValue(Token::Type_bool);
    } else if (isBinOp(op)) {
        if (lType != Token::Type_int)
            throw Exception("only int operands allowed with this operator.");
        expr->result = AST::createValue(Token::Type_int);
    } else
        assert(false && "unhandled operator found");
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
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::TypecheckerVisitor::visit(AST::InputExpr *expr)
{
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::TypecheckerVisitor::visit(AST::ArrayExpr *expr)
{
    expr->expr->accept(this);
    if (!expr->expr->result->isIntValue())
        throw Exception("Only int allowed as array expression argument");
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::TypecheckerVisitor::visit(AST::FnCallExpr *expr)
{
    if (!_symTab->hasSymbol(expr->name))
        throw Exception("calling undefined function : " + expr->name, ExceptionType::Type);

    AST::FunctionPrototype* fnProto = _symTab->getFnProto(expr->name);
    if (expr->fnArgs.size() != fnProto->fnParams.size())
        throw Exception("Function " + fnProto->fnName + " expects different number of arguments than given.");

    int i = 0;
    for (auto& arg: expr->fnArgs)
    {
        arg->accept(this);
        const Token argType = arg->result->getType();
        const Token paramType = fnProto->fnParams[i].type;
        if (argType != paramType)
            throw Exception("Fn call arg types mismatch.");
        i++;
    }
    expr->result = AST::createValue(fnProto->fnType);

}

void Visitor::TypecheckerVisitor::visit(AST::Program* program)
{
    // add functions to symbol table in first pass
    for (auto &fnDef : program->fnDefinitions)
    {
        if (!_symTab->addFnSymbol(fnDef->proto->fnName, fnDef->proto.get()))
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
        _symTab->addFnSymbol("lilfn_ " + fnDef->proto->fnName, fnDef->proto.get());
        EnterScope();
        for(auto& param : fnDef->proto->fnParams)
        {
            if(!_symTab->addSymbol(param.name, param.type))
                throw Exception("redeclaring variable : " + param.name, ExceptionType::Type);
        }
        fnDef->body->accept(this);

        // check if the last statement of non-void function body is return/abort or not
        auto& lastStmt = fnDef->body->stmt_list.back();
        // condition = last statement of "non-void" functions should be abort or return
        bool lastStmtConditionMet = true;
        if (fnDef->proto->fnType != Token::Type_void) {
            lastStmtConditionMet = false;
            if (dynamic_cast<AST::AbortStmt*>(lastStmt.get()) ||
                dynamic_cast<AST::ReturnStmt*>(lastStmt.get()))
                lastStmtConditionMet = true;
        }
        if (!lastStmtConditionMet)
            throw Exception("Last statement of non-void function should be return or abort.");

        LeaveScope();
        LeaveScope();
    }
}