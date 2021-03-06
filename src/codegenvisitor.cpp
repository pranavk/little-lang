#include <iostream>
#include <cassert>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>

#include "utils.hpp"
#include "baseast.hpp"
#include "visitor.hpp"
#include "symtab.hpp"

namespace IntrinsicFn {
    constexpr auto printstring = "__l_print_string";
    constexpr auto printint = "__l_print_int";
    constexpr auto abort = "__l_abort";
    constexpr auto pow = "__l_pow";
    constexpr auto input = "__l_input";
};

// static variable init
llvm::LLVMContext Visitor::CodegenVisitor::_TheContext;
std::unique_ptr<llvm::Module> Visitor::CodegenVisitor::_TheModule = llvm::make_unique<llvm::Module>("lilang", Visitor::CodegenVisitor::_TheContext);
llvm::IRBuilder<> Visitor::CodegenVisitor::_Builder(Visitor::CodegenVisitor::_TheContext);
std::map<std::string, llvm::AllocaInst*> Visitor::CodegenVisitor::_NamedValues;

llvm::Value* Visitor::CodegenVisitor::UpdateToIntWidth(llvm::Value* val, size_t toWidth) {
    if (!val->getType()->isIntegerTy())
        return nullptr;

    llvm::Value* res = val;
    unsigned oldWidth = val->getType()->getIntegerBitWidth();
    if (oldWidth == 1)
        return res;

    if (oldWidth < toWidth) {
        res = _Builder.CreateSExt(val, llvm::Type::getIntNTy(_TheContext, toWidth));
    } else if (oldWidth > toWidth) {
        res = _Builder.CreateTrunc(val, llvm::Type::getIntNTy(_TheContext, toWidth));
    }
    return res;
}

llvm::Type* Visitor::CodegenVisitor::CreateLLVMType(const Token type)
{
    unsigned intWidth = 64;
    std::string width = Lilang::Settings::get().getOptionValue("width");
    if (!width.empty()) {
        intWidth = std::stoul(width);
    }
    switch(type) {
        case Token::Type_int:
            return llvm::Type::getIntNTy(_TheContext, intWidth);
        break;
        case Token::Type_bool:
            return llvm::Type::getInt1Ty(_TheContext);
        break;
        case Token::Type_void:
            return llvm::Type::getVoidTy(_TheContext);
        break;
        case Token::Type_array:
            return llvm::StructType::get(_TheContext, {llvm::Type::getIntNTy(_TheContext, intWidth),
                                                llvm::PointerType::get(CreateLLVMType(Token::Type_int), 0)});
        default:
            return nullptr;
    }
}

llvm::AllocaInst* Visitor::CodegenVisitor::CreateEntryBlockAlloca(llvm::Function* func,
                                                                  const std::string& varName,
                                                                  const Token type)
{
    llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    auto llvmType = CreateLLVMType(type);
    auto allocaInst = tmpB.CreateAlloca(CreateLLVMType(type), 0, varName.c_str());
    if (llvmType->isSized()) {
        const llvm::DataLayout& dl = _TheModule->getDataLayout();
        uint64_t bitsSize = dl.getTypeSizeInBits(llvmType);
        tmpB.CreateCall(_TheModule->getFunction("llvm.memset.p0i8.i64"), {tmpB.CreateBitCast(allocaInst, llvm::Type::getInt8PtrTy(_TheContext)),
                                                                        llvm::ConstantInt::get(llvm::Type::getInt8Ty(_TheContext), 0),
                                                                         llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), bitsSize/8),
                                                                        llvm::ConstantInt::getFalse(_TheContext)});
    }
    return allocaInst;
}

llvm::AllocaInst* Visitor::CodegenVisitor::CreateAllocaArray(llvm::Function* func,
                                                             const std::string& varName,
                                                             llvm::Value* arrSize)
{

    llvm::Value* dataAlloca = _Builder.CreateAlloca(CreateLLVMType(Token::Type_int), arrSize, varName.c_str());
    llvm::AllocaInst* arrayAlloca = _Builder.CreateAlloca(CreateLLVMType(Token::Type_array));
    llvm::Value* sizePtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 0)});
    _Builder.CreateStore(arrSize, sizePtr);
    auto dataType = CreateLLVMType(Token::Type_int);
    llvm::Value* dataPtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(dataType, 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 1)});

    // memset 0 in the array
    if (dataType->isSized()) {
        const llvm::DataLayout& dl = _TheModule->getDataLayout();
        uint64_t bitsSize = dl.getTypeSizeInBits(dataType);
        auto len = _Builder.CreateMul(llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), (bitsSize / 8)), arrSize);
        _Builder.CreateCall(_TheModule->getFunction("llvm.memset.p0i8.i64"), {_Builder.CreateBitCast(dataAlloca, llvm::Type::getInt8PtrTy(_TheContext)),
                                                                        llvm::ConstantInt::get(llvm::Type::getInt8Ty(_TheContext), 0),
                                                                        len,
                                                                        llvm::ConstantInt::getFalse(_TheContext)});    }
    _Builder.CreateStore(dataAlloca, dataPtr);
    return arrayAlloca;
}

void Visitor::CodegenVisitor::declareRuntimeFns()
{
    if (!(_TheModule->getFunction(IntrinsicFn::printstring)))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(CreateLLVMType(Token::Type_void),
                                                               llvm::Type::getInt8PtrTy(_TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                IntrinsicFn::printstring, _TheModule.get());
    }

    if (!(_TheModule->getFunction(IntrinsicFn::printint)))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(CreateLLVMType(Token::Type_void),
                                                               llvm::Type::getInt64Ty(_TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                IntrinsicFn::printint, _TheModule.get());
    }

    if (!(_TheModule->getFunction(IntrinsicFn::input)))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(_TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      IntrinsicFn::input, _TheModule.get());
    }

    if (!(_TheModule->getFunction(IntrinsicFn::pow)))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(CreateLLVMType(Token::Type_int),
                                                               {CreateLLVMType(Token::Type_int), CreateLLVMType(Token::Type_int)},
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      IntrinsicFn::pow, _TheModule.get());
    }

    if (!(_TheModule->getFunction(IntrinsicFn::abort)))
    {
        llvm::FunctionType *funcType = llvm::FunctionType::get(CreateLLVMType(Token::Type_void),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      IntrinsicFn::abort, _TheModule.get());
    }

    // stacksave, stackrestore used for dynamic array allocation/deallocation
    if (!(_TheModule->getFunction("llvm.stacksave"))) {
        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(_TheContext),
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                               "llvm.stacksave", _TheModule.get());
    }

    if (!(_TheModule->getFunction("llvm.stackrestore"))) {
        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_TheContext),
                                                               {llvm::Type::getInt8PtrTy(_TheContext)},
                                                               false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                               "llvm.stackrestore", _TheModule.get());
    }

    // all variables should be initialized to zero
    if (!_TheModule->getFunction("llvm.memset.p0i8.i64")) {
        llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_TheContext),
                                                            {llvm::Type::getInt8PtrTy(_TheContext),
                                                             llvm::Type::getInt8Ty(_TheContext),
                                                             llvm::Type::getInt64Ty(_TheContext),
                                                             llvm::Type::getInt1Ty(_TheContext)},
                                                             false);
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                               "llvm.memset.p0i8.i64", _TheModule.get());
    }
}

void Visitor::CodegenVisitor::visit(AST::StmtBlockStmt *stmtBlock)
{
    AST::ReturnStmt* returnStmt = nullptr;
    _scope.push(stmtBlock);
    for (auto &stmt : stmtBlock->stmt_list)
    {
        stmt->accept(this);
        if ((returnStmt = dynamic_cast<AST::ReturnStmt*>(stmt.get()))) {
            break; // no need to process other stmts now
        }
    }

    // deallocate any array declaration
    while (!_arrayDecls.empty())
    {
        auto decl = _arrayDecls.top();
        if (decl.first != stmtBlock)
            break;

        _Builder.CreateCall(_TheModule->getFunction("llvm.stackrestore"), {decl.second});
        _arrayDecls.pop();
    }

    _scope.pop();

    if (returnStmt) {
        if (returnStmt->returnExpr)
            _Builder.CreateRet(returnStmt->returnExpr->llvmVal);
        else
            _Builder.CreateRetVoid();
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
        var.expr->accept(this);
        llvm::Value* stacksavecall = _Builder.CreateCall(_TheModule->getFunction("llvm.stacksave"));
        AST::StmtBlockStmt* curScope = _scope.top();
        _arrayDecls.push({curScope, stacksavecall});
        llvm::AllocaInst* alloca = CreateAllocaArray(func, var.name, var.expr->llvmVal);
        _NamedValues[var.name] = alloca;
    }
}

void Visitor::CodegenVisitor::visit(AST::PrintStmt *stmt)
{
    for (auto& arg : stmt->args) {
        arg->accept(this);
        if (auto strExpr = dynamic_cast<AST::StringLiteralExpr*>(arg.get())) {
            _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::printstring), {strExpr->llvmVal});
        } else {
            arg->llvmVal = UpdateToIntWidth(arg->llvmVal, 64);
            _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::printint), {arg->llvmVal});
        }
    }
}

void Visitor::CodegenVisitor::visit(AST::IfStmt *stmt)
{
    stmt->cond->accept(this);
    if (!stmt->cond->llvmVal)
        throw Exception("Can't convert if condition to llvm type");

    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(_TheContext, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(_TheContext, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(_TheContext, "ifcont");

    if (stmt->falseStmt) {
        _Builder.CreateCondBr(stmt->cond->llvmVal, thenBB, elseBB);
    } else {
        _Builder.CreateCondBr(stmt->cond->llvmVal, thenBB, mergeBB);
    }

    _Builder.SetInsertPoint(thenBB);
    stmt->trueStmt->accept(this);
    if (!_Builder.GetInsertBlock()->getTerminator())
        _Builder.CreateBr(mergeBB);
    thenBB = _Builder.GetInsertBlock();

    if (stmt->falseStmt) {
        func->getBasicBlockList().push_back(elseBB);
        _Builder.SetInsertPoint(elseBB);
        stmt->falseStmt->accept(this);
        if (!_Builder.GetInsertBlock()->getTerminator())
            _Builder.CreateBr(mergeBB);
        elseBB = _Builder.GetInsertBlock();
    }

    func->getBasicBlockList().push_back(mergeBB);
    _Builder.SetInsertPoint(mergeBB);
}

void Visitor::CodegenVisitor::visit(AST::WhileStmt *stmt)
{
    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* loopHdr = llvm::BasicBlock::Create(_TheContext, "loop.header", func);
    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(_TheContext, "loop.body", func);
    llvm::BasicBlock* afterLoop = llvm::BasicBlock::Create(_TheContext, "loop.after", func);

    _Builder.CreateBr(loopHdr);

    _Builder.SetInsertPoint(loopHdr);
    stmt->cond->accept(this);
    _Builder.CreateCondBr(stmt->cond->llvmVal, loopBody, afterLoop);

    _Builder.SetInsertPoint(loopBody);
    stmt->body->accept(this);
    _Builder.CreateBr(loopHdr);

    _Builder.SetInsertPoint(afterLoop);
}

void Visitor::CodegenVisitor::visit(AST::ForStmt *stmt)
{
    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* loopHdr = llvm::BasicBlock::Create(_TheContext, "forloop.header", func);
    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(_TheContext, "forloop.body", func);
    llvm::BasicBlock* afterLoop = llvm::BasicBlock::Create(_TheContext, "forloop.after", func);

    AST::IdExpr* idExpr = dynamic_cast<AST::IdExpr*>(stmt->ident.get());
    AST::IdExpr* contExpr = dynamic_cast<AST::IdExpr*>(stmt->container.get());
    if (!idExpr || !contExpr)
        throw Exception("For stmt doesn't contain any valid id or container expr.");

    // declare the variable in for statement
    llvm::AllocaInst* forIdentAlloca = _Builder.CreateAlloca(CreateLLVMType(Token::Type_int), 0, idExpr->name);
    _NamedValues[idExpr->name] = forIdentAlloca;

    llvm::Value* arrayAlloca = _NamedValues[contExpr->name];
    llvm::Value* sizePtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 0)});
    llvm::Value* dataPtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 1)});
    llvm::Value* data = _Builder.CreateLoad(dataPtr);
    llvm::Value* size = _Builder.CreateLoad(sizePtr);
    llvm::Value* counterAlloca = _Builder.CreateAlloca(CreateLLVMType(Token::Type_int));
    _Builder.CreateStore(llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), counterAlloca);
    _Builder.CreateBr(loopHdr);

     // move to the loop header BB
    _Builder.SetInsertPoint(loopHdr);
    llvm::Value* counterV = _Builder.CreateLoad(counterAlloca);
    llvm::Value* condV = _Builder.CreateICmpSLT(counterV, size);
    _Builder.CreateCondBr(condV, loopBody, afterLoop);

    _Builder.SetInsertPoint(loopBody);
    llvm::Value* elePtr = _Builder.CreateGEP(data, counterV);
    _Builder.CreateStore(_Builder.CreateLoad(elePtr), _NamedValues[idExpr->name]);
    stmt->body->accept(this);
    counterV = _Builder.CreateAdd(counterV, llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 1));
    _Builder.CreateStore(counterV, counterAlloca);
    _Builder.CreateBr(loopHdr);

    _Builder.SetInsertPoint(afterLoop);
}

void Visitor::CodegenVisitor::visit(AST::ReturnStmt *stmt)
{
    if (stmt->returnExpr) {
        stmt->returnExpr->accept(this);
        stmt->returnExpr->llvmVal = UpdateToIntWidth(stmt->returnExpr->llvmVal, 64);
    }
}

void Visitor::CodegenVisitor::visit(AST::AbortStmt *stmt)
{
    stmt->llvmVal = _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::abort));
    _Builder.CreateUnreachable();
}

void Visitor::CodegenVisitor::visit(AST::ArrayAssignment *stmt)
{
    stmt->idxExpr->accept(this);
    llvm::Value* arrayAlloca = _NamedValues[stmt->name];

    llvm::Value* sizePtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 0)});
    llvm::Value* size = _Builder.CreateLoad(sizePtr);

    llvm::Value* dataPtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 1)});
    llvm::Value* data = _Builder.CreateLoad(dataPtr);

    llvm::Value* arrayLessThan = _Builder.CreateICmpSLT(stmt->idxExpr->llvmVal, size);

    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* gt0checkBB = llvm::BasicBlock::Create(_TheContext, "gt0checkbb", func);
    llvm::BasicBlock* failBB = llvm::BasicBlock::Create(_TheContext, "failbb", func);
    llvm::BasicBlock* fullyPassBB = llvm::BasicBlock::Create(_TheContext, "fullypassbb", func);

    _Builder.CreateCondBr(arrayLessThan, gt0checkBB, failBB);

    _Builder.SetInsertPoint(failBB);
    _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::abort));
    _Builder.CreateUnreachable();

    _Builder.SetInsertPoint(gt0checkBB);
    llvm::Value* arrayGreaterThan = _Builder.CreateICmpSGE(stmt->idxExpr->llvmVal, llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0));
    _Builder.CreateCondBr(arrayGreaterThan, fullyPassBB, failBB);

    _Builder.SetInsertPoint(fullyPassBB);

    llvm::Value* elePtr = _Builder.CreateGEP(data, stmt->idxExpr->llvmVal);
    stmt->expr->accept(this);
    _Builder.CreateStore(stmt->expr->llvmVal, elePtr);
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

void Visitor::CodegenVisitor::visit(AST::TrueExpr *expr)
{
    expr->llvmVal = llvm::ConstantInt::get(llvm::Type::getInt1Ty(_TheContext), 1);
}

void Visitor::CodegenVisitor::visit(AST::FalseExpr *expr)
{
    expr->llvmVal = llvm::ConstantInt::get(llvm::Type::getInt1Ty(_TheContext), 0);
}

void Visitor::CodegenVisitor::visit(AST::NumExpr *expr)
{
    expr->llvmVal = llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), expr->val);
}

void Visitor::CodegenVisitor::visit(AST::IdExpr *expr)
{
    if (!_NamedValues[expr->name])
        throw Exception("No such variable found");
    expr->llvmVal = _Builder.CreateLoad(_NamedValues[expr->name]);
}

void Visitor::CodegenVisitor::visit(AST::LiteralExpr *expr)
{
    std::cout << "Literal expr" << std::endl;
}

void Visitor::CodegenVisitor::visit(AST::StringLiteralExpr *expr)
{
    expr->llvmVal = _Builder.CreateGlobalStringPtr(expr->val);
}

void Visitor::CodegenVisitor::visit(AST::TernaryExpr* expr)
{
    expr->condExpr->accept(this);
    expr->trueExpr->accept(this);
    expr->falseExpr->accept(this);
    expr->llvmVal = _Builder.CreateSelect(expr->condExpr->llvmVal, expr->trueExpr->llvmVal, expr->falseExpr->llvmVal);
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
        case Token::Op_mod:
            expr->llvmVal = _Builder.CreateSRem(lhsV, rhsV);
            break;
        case Token::Op_exp:
        {
            expr->llvmVal = _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::pow), {lhsV, rhsV});
        }
        break;
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
    AST::IdExpr* arrId = dynamic_cast<AST::IdExpr*>(expr->idExpr.get());
    if (!arrId)
        throw Exception("found unexpected array identifier");

    llvm::Value* arrayAlloca = _NamedValues[arrId->name];
    llvm::Value* sizePtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 0)});
    expr->llvmVal = _Builder.CreateLoad(sizePtr);
}

void Visitor::CodegenVisitor::visit(AST::InputExpr *expr)
{
    auto val = _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::input));
    expr->llvmVal = UpdateToIntWidth(val, Lilang::Settings::get().getWidth());
}

void Visitor::CodegenVisitor::visit(AST::ArrayExpr *expr)
{
    expr->expr->accept(this);
    llvm::Value* arrayAlloca = _NamedValues[expr->name];
    llvm::Value* sizePtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 0)});
    llvm::Value* size = _Builder.CreateLoad(sizePtr);

    llvm::Value* dataPtr = _Builder.CreateGEP(arrayAlloca, {llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(_TheContext), 1)});
    llvm::Value* data = _Builder.CreateLoad(dataPtr);

    llvm::Value* arrayLessThan = _Builder.CreateICmpSLT(expr->expr->llvmVal, size);

    llvm::Function* func = _Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* gt0checkBB = llvm::BasicBlock::Create(_TheContext, "gt0checkbb", func);
    llvm::BasicBlock* failBB = llvm::BasicBlock::Create(_TheContext, "failbb", func);
    llvm::BasicBlock* fullyPassBB = llvm::BasicBlock::Create(_TheContext, "fullypassbb", func);

    _Builder.CreateCondBr(arrayLessThan, gt0checkBB, failBB);

    _Builder.SetInsertPoint(failBB);
    _Builder.CreateCall(_TheModule->getFunction(IntrinsicFn::abort));
    _Builder.CreateUnreachable();

    _Builder.SetInsertPoint(gt0checkBB);
    llvm::Value* arrayGreaterThan = _Builder.CreateICmpSGE(expr->expr->llvmVal, llvm::ConstantInt::get(CreateLLVMType(Token::Type_int), 0));
    _Builder.CreateCondBr(arrayGreaterThan, fullyPassBB, failBB);

    _Builder.SetInsertPoint(fullyPassBB);
    llvm::Value* elePtr = _Builder.CreateGEP(data, expr->expr->llvmVal);
    expr->llvmVal = _Builder.CreateLoad(elePtr);
}

void Visitor::CodegenVisitor::visit(AST::FnCallExpr *expr)
{
    llvm::Function* func = nullptr;
    if(!(func = _TheModule->getFunction(expr->name)))
        throw Exception("Cannot find func : " + expr->name);

    std::vector<llvm::Value*> llvmArgs;
    // codegen all function args
    for (auto& arg : expr->fnArgs) {
        arg->accept(this);
        llvmArgs.push_back(arg->llvmVal);
    }

    expr->llvmVal = _Builder.CreateCall(func, llvmArgs);
}

void Visitor::CodegenVisitor::visit(AST::Program* program)
{
    declareRuntimeFns();
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

    // set 'main' linkage to external
    _TheModule->getFunction("main")->setLinkage(llvm::Function::ExternalLinkage);

    bool err = false;
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

        // for non-void functions, typechecker should have already rejected programs without
        // return/abort statements at end of the function.
        if (!_Builder.GetInsertBlock()->getTerminator() && fnDef->proto->fnType == Token::Type_void) {
            _Builder.CreateRetVoid();
        }
        err = llvm::verifyFunction(*func, &llvm::errs());
    }

    err = llvm::verifyModule(*_TheModule.get(), &llvm::errs()) || err;
    if (err) {
        throw Exception("LLVM IR verification failed.");
    }
}