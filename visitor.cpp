#include <iostream>

#include "baseast.hpp"
#include "visitor.hpp"

void Visitor::PrintASTVisitor::print(const std::vector<std::string>& vec)
{
    for (int i = 0; i < _depth; i++) 
    {
        std::cout << "\t";
    }
    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i];
    }
    std::cout << std::endl;
}

void Visitor::PrintASTVisitor::visit(AST::StmtBlockStmt *stmtBlock)
{
    _depth++;
    for (auto &stmt : stmtBlock->stmt_list)
    {
        stmt->accept(this);
    }
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::VarDeclStmt *stmt)
{
    print({"var declare"});
    _depth++;
    for (auto& var : stmt->decls) 
    {
        print({var.name, ":", 
               std::to_string(static_cast<int>(var.type))});
    }
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::ArrayDeclStmt *stmt)
{
    print({"array declare"});
    _depth++;
    for (auto& var : stmt->decls) 
    {
        print({var.name});
        _depth++;
        var.expr->accept(this);
        _depth--;
    }
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::PrintStmt *stmt)
{
    print({"print: "});
    _depth++;
    for (auto& expr : stmt->args) 
    {
        expr->accept(this);
    }
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::IfStmt *stmt)
{
    print({"If(condE) {trueE} [else { falseE}]"});
    print({"condE: "});
    _depth++;
    stmt->cond->accept(this);
    _depth--;

    print({"trueE: "});
    _depth++;
    stmt->trueStmt->accept(this);
    _depth--;

    if (stmt->falseStmt)
    {
        print({"falseE: "});
        _depth--;
        stmt->falseStmt->accept(this);
        _depth++;
    }
}

void Visitor::PrintASTVisitor::visit(AST::WhileStmt *stmt)
{
    print({"while (cond) body"});
    print({"cond"});
    _depth++;
    stmt->cond->accept(this);
    _depth--;

    print({"body"});
    _depth++;
    stmt->body->accept(this);
    _depth--;
}


void Visitor::PrintASTVisitor::visit(AST::ForStmt *stmt)
{
    print({"for (ident : container) body"});
    print({"ident:"});
    
    _depth++;
    stmt->ident->accept(this);
    _depth--;

    print({"container:"});
    _depth++;
    stmt->container->accept(this);
    _depth--;

    print({"body: "});
    _depth++;
    stmt->body->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::ReturnStmt *stmt)
{
    print({"return expr"});

    if (stmt->returnExpr)
    {
        print({"expr:"});
        _depth++;
        stmt->returnExpr->accept(this);
        _depth--;
    }
}

void Visitor::PrintASTVisitor::visit(AST::ArrayAssignment *stmt)
{
    print({"ident[idxExpr] = expr"});
    print({"ident:", stmt->name});

    print({"idxExpr:"});
    _depth++;
    stmt->idxExpr->accept(this);
    _depth--;

    print({"="});

    print({"expr:"});
    _depth++;
    stmt->expr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::VarAssignment *stmt)
{
    print({"name = expr"});
    print({"name: ", stmt->name});

    print({"expr:"});
    _depth++;
    stmt->expr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::BaseExpr* expr)
{
    print({expr->result->getVal()});
}

void Visitor::PrintASTVisitor::visit(AST::NumExpr *expr)
{
    print({expr->result->getVal()}); // this will print some \t's
}

void Visitor::PrintASTVisitor::visit(AST::IdExpr *expr)
{
    print({expr->name, ":ID"}); // this will print some \t's
}

void Visitor::PrintASTVisitor::visit(AST::LiteralExpr *expr)
{
    std::cout << "Literal expr" << std::endl;
}

void Visitor::PrintASTVisitor::visit(AST::StringLiteralExpr *expr)
{
    print({expr->val, " : string"});
}

void Visitor::PrintASTVisitor::visit(AST::TernaryExpr* expr)
{
    print({"(condE?trueE:falseE)"});
    print({"condE"});
    _depth++;
    expr->condExpr->accept(this);
    _depth--;

    print({"trueE"});
    _depth++;
    expr->trueExpr->accept(this);
    _depth--;

    print({"falseE"});
    _depth++;
    expr->falseExpr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::BinopExpr* expr)
{
    print({"(expr1 binop expr2)"});
    print({"expr1"});
    _depth++;
    expr->leftExpr->accept(this);
    _depth--;

    print({std::to_string(static_cast<int>(expr->op)), ":binOp"});

    print({"expr2"});
    _depth++;
    expr->rightExpr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::UnaryExpr *expr)
{
    print({"(unaryop expr)"});
    print({std::to_string(static_cast<int>(expr->type)), ":unaryop"});

    _depth++;
    expr->expr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::SizeofExpr *expr)
{
    print({"sizeof expr"});
    print({"expr: "});

    _depth++;
    expr->idExpr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::InputExpr *expr)
{
    print({"input expr"});
}

void Visitor::PrintASTVisitor::visit(AST::ArrayExpr *expr)
{
    print({"array expression : ident[expr]"});
    print({"ident:", expr->name});

    print({"expr:"});
    _depth++;
    expr->expr->accept(this);
    _depth--;
}

void Visitor::PrintASTVisitor::visit(AST::FnCallExpr *expr)
{
    print({"fn call expression: name(a, b, ...)"});

    print({"fn_name:", expr->name});
    print({"fn args:"});
    for (auto& arg: expr->fnArgs)
    {
        print({"argexpr:"});
        _depth++;
        arg->accept(this);
        _depth--;
    }
}

void Visitor::PrintASTVisitor::visitProgramAST(std::vector<std::unique_ptr<AST::FunctionDefinition>> &fnDefs)
{
    for (auto &fnDef : fnDefs)
    {
        fnDef->print();
        fnDef->body->accept(this);
    }
}
