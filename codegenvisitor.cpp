#include <iostream>
#include <cassert>

#include <llvm/IR/DerivedTypes.h>

#include "baseast.hpp"
#include "visitor.hpp"
#include "symtab.hpp"

// static variable init
llvm::LLVMContext Visitor::CodegenVisitor::_TheContext;
std::unique_ptr<llvm::Module> Visitor::CodegenVisitor::_TheModule = llvm::make_unique<llvm::Module>("lilang", Visitor::CodegenVisitor::_TheContext);
llvm::IRBuilder<> Visitor::CodegenVisitor::_Builder(Visitor::CodegenVisitor::_TheContext);
std::map<std::string, llvm::AllocaInst*> Visitor::CodegenVisitor::_NamedValues;

llvm::Type* Visitor::CodegenVisitor::CreateLLVMType(const Token type)
{
    switch(type) {
        case Token::Type_int:
            return llvm::Type::getInt128Ty(_TheContext);
        break;
        case Token::Type_bool:
            return llvm::Type::getInt1Ty(_TheContext);
        break;
        case Token::Type_void:
            return llvm::Type::getVoidTy(_TheContext);
        break;
        case Token::Type_array:
            return llvm::ArrayType::get(llvm::Type::getInt128Ty(_TheContext), 1000);
        default:
            return nullptr;
    }
}

llvm::AllocaInst* Visitor::CodegenVisitor::CreateEntryBlockAlloca(llvm::Function* func,
                                                                  const std::string& varName,
                                                                  const Token type)
{
    llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    return tmpB.CreateAlloca(CreateLLVMType(type), 0, varName.c_str());
}

void Visitor::CodegenVisitor::visit(AST::StmtBlockStmt *stmtBlock)
{
    for (auto &stmt : stmtBlock->stmt_list)
    {
        stmt->accept(this);
    }
}

void Visitor::CodegenVisitor::visit(AST::VarDeclStmt *stmt)
{
    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    for (auto& var : stmt->decls)
    {
        llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func, var.name, var.type);
        _NamedValues[var.name] = alloca;
    }
}

void Visitor::CodegenVisitor::visit(AST::ArrayDeclStmt *stmt)
{
    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    for (auto& var : stmt->decls)
    {
        llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func, var.name, Token::Type_array);
        _NamedValues[var.name] = alloca;
    }
}

void Visitor::CodegenVisitor::visit(AST::PrintStmt *stmt)
{
    for (auto& expr : stmt->args)
    {
        expr->accept(this);
        if (!expr->result->isStringValue() && !expr->result->isIntValue())
            throw Exception("only string literals and int identifiers allowed in print args", ExceptionType::Type);
    }
}

void Visitor::CodegenVisitor::visit(AST::IfStmt *stmt)
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

void Visitor::CodegenVisitor::visit(AST::WhileStmt *stmt)
{
    stmt->cond->accept(this);
    if (!stmt->cond->result->isBoolValue())
        throw Exception("Only bool value allowed as while stmt condition.", ExceptionType::Type);

    stmt->body->accept(this);
}


void Visitor::CodegenVisitor::visit(AST::ForStmt *stmt)
{
    stmt->ident->accept(this);
    if (!stmt->ident->result->isIntValue())
        throw Exception("Id expected in for stmt before colon", ExceptionType::Type);

    stmt->container->accept(this);
    if (!stmt->container->result->isArrayValue())
        throw Exception("array expected in for stmt after colon", ExceptionType::Type);

    stmt->body->accept(this);
}

void Visitor::CodegenVisitor::visit(AST::ReturnStmt *stmt)
{
    stmt->returnExpr->accept(this);
    _Builder.CreateRet(stmt->returnExpr->llvmVal);
}

void Visitor::CodegenVisitor::visit(AST::AbortStmt *stmt)
{
    for (auto& expr : stmt->args)
    {
        expr->accept(this);
        if (!expr->result->isStringValue() && !expr->result->isIntValue())
            throw Exception("only string or int allowed as abort stmt arguments", ExceptionType::Type);
    }
}

void Visitor::CodegenVisitor::visit(AST::ArrayAssignment *stmt)
{

}

void Visitor::CodegenVisitor::visit(AST::VarAssignment *stmt)
{
    stmt->expr->accept(this);
    if (!_NamedValues[stmt->name])
        throw Exception("No such name found in symbol table");

    _Builder.CreateStore(stmt->expr->llvmVal, _NamedValues[stmt->name]);
}

void Visitor::CodegenVisitor::visit(AST::BaseExpr* expr)
{

}

void Visitor::CodegenVisitor::visit(AST::NumExpr *expr)
{
    expr->llvmVal = llvm::ConstantInt::get(llvm::Type::getInt128Ty(_TheContext), expr->val);
}

void Visitor::CodegenVisitor::visit(AST::IdExpr *expr)
{
    if (!_NamedValues[expr->name])
        throw Exception("No such variable found");
    expr->llvmVal = _NamedValues[expr->name];
}

void Visitor::CodegenVisitor::visit(AST::LiteralExpr *expr)
{
    std::cout << "Literal expr" << std::endl;
}

void Visitor::CodegenVisitor::visit(AST::StringLiteralExpr *expr)
{
    expr->result = std::make_unique<AST::StringValue>();
}

void Visitor::CodegenVisitor::visit(AST::TernaryExpr* expr)
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

void Visitor::CodegenVisitor::visit(AST::BinopExpr* expr)
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

void Visitor::CodegenVisitor::visit(AST::UnaryExpr *expr)
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

void Visitor::CodegenVisitor::visit(AST::SizeofExpr *expr)
{
    expr->idExpr->accept(this);
    if (!expr->idExpr->result->isArrayValue())
        throw Exception("Sizeof argument should be an array value");
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::CodegenVisitor::visit(AST::InputExpr *expr)
{
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::CodegenVisitor::visit(AST::ArrayExpr *expr)
{
    expr->expr->accept(this);
    if (!expr->expr->result->isIntValue())
        throw Exception("Only int allowed as array expression argument");
    expr->result = AST::createValue(Token::Type_int);
}

void Visitor::CodegenVisitor::visit(AST::FnCallExpr *expr)
{



}

void Visitor::CodegenVisitor::visit(AST::Program* program)
{
    // add functions to symbol table in first pass
    for (auto &fnDef : program->fnDefinitions)
    {
        llvm::Function* func = _TheModule->getFunction(fnDef->proto->fnName);
        if (func)
            throw Exception("Function name " + fnDef->proto->fnName + " already exist in llvm module");

        std::vector<llvm::Type *> argTypes;
        for (auto &param : fnDef->proto->fnParams)
        {
            if (param.type == Token::Type_void)
                throw Exception("void type in fn param not allowed");
            argTypes.push_back(CreateLLVMType(param.type));
        }

        llvm::FunctionType* ft = llvm::FunctionType::get(CreateLLVMType(fnDef->proto->fnType), argTypes, false);
        func = llvm::Function::Create(ft, llvm::Function::PrivateLinkage, fnDef->proto->fnName, _TheModule.get());

        // set names for func params
        unsigned idx = 0;
        for (auto& arg : func->args()) {
            arg.setName(fnDef->proto->fnParams[idx++].name);
        }
    }

    // iterate over individual funcs
    for (auto& fnDef : program->fnDefinitions)
    {
        llvm::Function* func = _TheModule->getFunction(fnDef->proto->fnName);
        if (!func)
            assert(false && "Impossible that we couldn't find function in the module here");

        llvm::BasicBlock* bb = llvm::BasicBlock::Create(_TheContext, "entry", func);
        _Builder.SetInsertPoint(bb);

        _NamedValues.clear();
        unsigned idx = 0;
        for(auto& arg : func->args())
        {
            llvm::AllocaInst* alloca = CreateEntryBlockAlloca(func,
                                                              arg.getName(),
                                                              fnDef->proto->fnParams[idx++].type);
            _Builder.CreateStore(&arg, alloca);
            _NamedValues[arg.getName()] = alloca;
        }

        fnDef->body->accept(this);
        if (!fnDef->body->llvmVal) {
            func->eraseFromParent();
            throw Exception("Couldn't codegen for function body");
        }

        llvm::verifyFunction(*func);

        // print it
        llvm::errs() << *_TheModule;
    }
}