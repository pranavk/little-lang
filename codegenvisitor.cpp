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

}

void Visitor::CodegenVisitor::visit(AST::IfStmt *stmt)
{
    /*
    stmt->cond->accept(this);
    if (!stmt->cond->llvmVal)
        throw Exception("Can't convert if condition to llvm type");

    llvm::Value* condV = _Builder.CreateICmpEQ(stmt->cond->llvmVal,
                                               llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 1));
    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(_TheContext, "then", func);

    if (stmt->falseStmt) {
        llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(_TheContext, "else", func);
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(_TheContext, "ifcont");
    } else {
        llvm::BasicBlock* contBB = llvm::BasicBlock::Create(_TheContext, "ifcont");
        _Builder.CreateCondBr(condV, thenBB, contBB);
        _Builder.SetInsertPoint(thenBB);
        stmt->trueStmt->accept(this);
        _Builder.CreateBr(contBB);
        thenBB = _Builder.GetInsertBlock();

        func->getBasicBlockList().push_back(contBB);
        _Builder.SetInsertPoint(contBB);
        llvm::PHINode* pN = _Builder.CreatePHI();
    }
    */
}

void Visitor::CodegenVisitor::visit(AST::WhileStmt *stmt)
{

}


void Visitor::CodegenVisitor::visit(AST::ForStmt *stmt)
{

}

void Visitor::CodegenVisitor::visit(AST::ReturnStmt *stmt)
{
    stmt->returnExpr->accept(this);
    _Builder.CreateRet(stmt->returnExpr->llvmVal);
}

void Visitor::CodegenVisitor::visit(AST::AbortStmt *stmt)
{

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
}

void Visitor::CodegenVisitor::visit(AST::TernaryExpr* expr)
{

}

void Visitor::CodegenVisitor::visit(AST::BinopExpr* expr)
{
    const Token op = expr->op;
    expr->leftExpr->accept(this);
    expr->rightExpr->accept(this);
    llvm::Value* lhsV = expr->leftExpr->llvmVal;
    llvm::Value* rhsV = expr->rightExpr->llvmVal;
    if (!lhsV || !rhsV)
        throw Exception("malformed binary expression");

    switch(op) {
        case Token::Op_and:
            expr->llvmVal = _Builder.CreateAnd(lhsV, rhsV);
            break;
        case Token::Op_or:
            expr->llvmVal = _Builder.CreateOr(lhsV, rhsV);
            break;
        case Token::Op_neq:
            expr->llvmVal = _Builder.CreateICmpNE(lhsV, rhsV);
            break;
        case Token::Op_eqeq:
            expr->llvmVal = _Builder.CreateICmpEQ(lhsV, rhsV);
            break;
        case Token::Op_gt:
            expr->llvmVal = _Builder.CreateICmpSGT(lhsV, rhsV);
            break;
        case Token::Op_gte:
            expr->llvmVal = _Builder.CreateICmpSGE(lhsV, rhsV);
            break;
        case Token::Op_lt:
            expr->llvmVal = _Builder.CreateICmpSLT(lhsV, rhsV);
            break;
        case Token::Op_lte:
            expr->llvmVal = _Builder.CreateICmpSLE(lhsV, rhsV);
            break;
        case Token::Op_add:
            expr->llvmVal = _Builder.CreateAdd(lhsV, rhsV);
            break;
        case Token::Op_minus:
            expr->llvmVal = _Builder.CreateSub(lhsV, rhsV);
            break;
        case Token::Op_mult:
            expr->llvmVal = _Builder.CreateMul(lhsV, rhsV);
            break;
        case Token::Op_divide:
            expr->llvmVal = _Builder.CreateSDiv(lhsV, rhsV);
            break;
        case Token::Op_exp:
            // what's the instruction in llvm for exp?
        default:
            throw Exception("codegen: binop not handled");
    }
}

void Visitor::CodegenVisitor::visit(AST::UnaryExpr *expr)
{
    const Token op = expr->type;
    expr->expr->accept(this);

    switch(op) {
        case Token::Op_bang:
            expr->llvmVal = _Builder.CreateNot(expr->expr->llvmVal);
        break;
        case Token::Op_minus:
            expr->llvmVal = _Builder.CreateNeg(expr->expr->llvmVal);
        break;
        default:
            throw Exception("codegen: unindentified unary op");
    }
}

void Visitor::CodegenVisitor::visit(AST::SizeofExpr *expr)
{

}

void Visitor::CodegenVisitor::visit(AST::InputExpr *expr)
{
}

void Visitor::CodegenVisitor::visit(AST::ArrayExpr *expr)
{

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