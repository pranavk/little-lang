#include <iostream>

#include "baseast.hpp"
#include "visitor.hpp"

void Visitor::TypecheckerVisitor::visit(AST::StmtBlockStmt *stmtBlock)
{
    for (auto &stmt : stmtBlock->stmt_list)
    {
        stmt->accept(this);
    }
}