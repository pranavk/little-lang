#include <string>
#include <iostream>
#include <memory>
#include <cassert>

#include "consts.hpp"
#include "baseast.hpp"

// contains all functions 
static std::vector<std::unique_ptr<AST::FunctionDefinition>> fnDefinitions;

// check lexer.l to see how strval is used
std::string strval;

int curTok = -1;
std::string curVal;

int getNextTok()
{
	curTok = static_cast<int>(yylex());
	curVal = strval;
//	std::cout << "nexttok: " << curTok << "(" << curVal << ")" << std::endl;

    return curTok;
}

void skipEOLs()
{
    while (curTok == static_cast<int>(Token::EOL) && getNextTok());
}

bool match(int tok)
{
    // are we trying to match EOL?
    if (tok != static_cast<int>(Token::EOL))
        skipEOLs();
    
    bool match = false;
    if (tok == curTok)
    {
        // discard the curTok now
        getNextTok(); 
	    match = true;
    }
    return match;
}

bool match(Token tok) 
{
    return match(static_cast<int>(tok));
}

bool isBinOp()
{ 
    bool res = false;
    switch(static_cast<Token>(curTok))
    {
        case Token::Op_add:
        case Token::Op_minus:
        case Token::Op_mult:
        case Token::Op_divide:
        case Token::Op_exp:
        case Token::Op_mod:
        case Token::Op_and:
        case Token::Op_or:
        case Token::Op_eqeq:
        case Token::Op_neq:
        case Token::Op_gt:
        case Token::Op_gte:
        case Token::Op_lt:
        case Token::Op_lte:
            res = true;
        default:
            res = false;
    }

    return res;
}

bool isUnaryOp()
{
    bool res = false;
    switch(static_cast<Token>(curTok))
    {
        case Token::Op_bang:
        case Token::Op_minus:
            res = true;
        default:
            res = false;
    }

    return res;
}

bool strict_match(Token tok) 
{
    bool res = false;
    if (!(res = match(tok)))
        throw Exception("Expected " + std::to_string(static_cast<int>(tok)) +
                         ", found " + std::to_string(curTok));
    return res;
}

bool isTokenType(int tok)
{
    bool result;
    switch(tok) {
        case static_cast<int>(Token::Type_array):
        case static_cast<int>(Token::Type_int):
        case static_cast<int>(Token::Type_bool):
        case static_cast<int>(Token::Type_void):
            result = true;
        break;
        default:
            result = false;
    }

    return result;
}

bool parseFnParams(std::vector<AST::FnParam>& args)
{
    // consume left parenthesis first
    bool res = match(static_cast<int>(Token::PL));
    bool first = true;
    while (res && curTok != static_cast<int>(Token::PR)) 
    {
        // consume a "," if it's not the first iteration
        if (!first) strict_match(Token::Comma);

        AST::FnParam param;
        res = false;
        if (isTokenType(curTok))
        {
            param.type = static_cast<Token>(curTok);
            if (getNextTok() == static_cast<int>(Token::Id))
            {
                param.name = strval;
                res = true;
            }
        }

        first = false;
        if (res) {
            args.push_back(param);
            getNextTok();
        }
        else
            throw Exception("error parsing function arguments");
    }

    res = res && match(static_cast<int>(Token::PR));

    return res;
}

std::unique_ptr<AST::FunctionDefinition> parseFnDef() 
{
    std::vector<AST::FnParam> params;
    std::string fnName;
    int fnType;
    if (isTokenType(fnType = curTok) && 
        (getNextTok() == static_cast<int>(Token::Id)) && !((fnName = curVal).empty()) &&
        getNextTok() && parseFnParams(params))
    {
        std::unique_ptr<AST::FunctionPrototype> proto(
            new AST::FunctionPrototype(fnType, fnName, params));
        return std::make_unique<AST::FunctionDefinition>(std::move(proto), nullptr);
    }

    throw Exception("cannot parse function arguments");
    return nullptr;
}

bool skipFnBody()
{
    bool res = match(static_cast<int>(Token::CL));
    int parenDepth = 0;
    if (res)
        parenDepth = 1;

    while (res && parenDepth > 0) {
        if (curTok == static_cast<int>(Token::CL))
            parenDepth++;
        else if (curTok == static_cast<int>(Token::CR))
            parenDepth--;
        getNextTok();
    }

    return res;
}

void parseProgram1() 
{
    getNextTok(); // get first token of program
    skipEOLs(); // discard any EOLs
    int mainFns = 0;
    while (auto fnDef = parseFnDef())
    {
        if (fnDef->proto->fnName == "main")
            mainFns++;
        
        // TODO: check if another function we are pushing is different
        fnDefinitions.push_back(std::move(fnDef));
        if (!skipFnBody())
            throw Exception("couldn't skip function body during 1st parse.");
        skipEOLs();

        if (curTok == EOF)
            break;
    }

    if (mainFns != 1)
        throw Exception("Program should have one function named 'main'");
}

void skipToFnBody() {
    while (curTok != static_cast<int>(Token::CL) && getNextTok());
}

std::unique_ptr<AST::BaseStmt> parseVarDecls()
{
    Token varType = static_cast<Token>(curTok);
    match(curTok); // cosume 'type'

    std::unique_ptr<AST::VarDeclStmt> varDeclStmt(new AST::VarDeclStmt);
    while (curTok == static_cast<int>(Token::Id)) 
    {
        std::string varName = curVal;
        match(Token::Id); // consume 'id'

        varDeclStmt->decls.push_back({varType, varName});

        // check if you got a comma without skipping EOLs
        if (curTok == static_cast<int>(Token::Comma))
            match(Token::Comma);
    }

    if (curTok != static_cast<int>(Token::EOL))
        throw Exception("expected EOL; got something else.");
    else if (varDeclStmt->decls.size() < 1)
        throw Exception("at least one variable should be declared.");

    match(Token::EOL);
    return varDeclStmt;
}

std::unique_ptr<AST::BaseExpr> parseExpr();

void parseFnArgs(std::vector<std::unique_ptr<AST::BaseExpr>>& fnArgs)
{
    strict_match(Token::PL);
    bool first = true;
    while (curTok != static_cast<int>(Token::PR)) 
    {
        // consume comma if it's not the first iteration.
        if (!first) strict_match(Token::Comma);
        
        auto expr = parseExpr();
        fnArgs.push_back(std::move(expr));       

        first = false;
    }

    strict_match(Token::PR);
}

std::unique_ptr<AST::BaseExpr> parseVarArrayOrFncall()
{
    std::string identName = curVal;
    assert(!identName.empty());

    match(Token::Id); // consume ident
    switch(curTok)
    {
        case static_cast<int>(Token::EOL): // ident
            return std::make_unique<AST::IdExpr>(identName);
        case static_cast<int>(Token::SL): // ident[name]
        {
            strict_match(Token::SL);
            auto baseExpr = parseExpr();
            strict_match(Token::SR);
            return std::make_unique<AST::ArrayExpr>(identName, std::move(baseExpr));
        }
        case static_cast<int>(Token::PL): // ident(name)
        {
            std::vector<std::unique_ptr<AST::BaseExpr>> fnArgs;
            parseFnArgs(fnArgs);
            return std::make_unique<AST::FnCallExpr>(identName, fnArgs);
        }
        default:
            throw Exception("only expected either ident or ident[name] or ident(call)");
    }
}

std::unique_ptr<AST::BaseExpr> parseExpr() 
{
    switch(curTok)
    {
        case static_cast<int>(Token::Literal_true):
            strict_match(Token::Literal_true);
            return std::make_unique<AST::BaseExpr>(std::make_unique<AST::BoolValue>(true));
        case static_cast<int>(Token::Literal_false):
            strict_match(Token::Literal_false);
            return std::make_unique<AST::BaseExpr>(std::make_unique<AST::BoolValue>(false));
        case static_cast<int>(Token::Number):
        {
            strict_match(Token::Number);
            assert(!curVal.empty());
            auto result = std::make_unique<AST::IntValue>(std::atoi(curVal.c_str()));
            return std::make_unique<AST::BaseExpr>(std::move(result));
        }
        case static_cast<int>(Token::PL): // (expr ? expr : expr), (expr binop expr), (unaryop expr)
        {
            strict_match(Token::PL);
            auto primaryExpr = parseExpr();
            assert(primaryExpr);

            if (match(Token::Op_question))  // (expr ? expr : expr)
            {
                auto trueE = parseExpr();
                strict_match(Token::Op_colon);
                auto falseE = parseExpr();
                strict_match(Token::PR);
                return std::make_unique<AST::TernaryExpr>(std::move(primaryExpr), 
                                                          std::move(trueE), 
                                                          std::move(falseE));
            } 
            else if (isUnaryOp()) // (unaryop expr)
            {
                strict_match(static_cast<Token>(curTok));

                auto expr = parseExpr();
                assert(expr);

                strict_match(Token::PR);
            }
            else if (isBinOp()) // (expr binop expr)
            {   
                strict_match(static_cast<Token>(curTok)); // consume whatever binop we got

                auto rhsExpr = parseExpr();
                assert(rhsExpr);

                strict_match(Token::PR);
                return std::make_unique<AST::BinopExpr>(std::move(primaryExpr), 
                                                        std::move(rhsExpr));
            }
            
            throw Exception("expected either ternary, binop, or unaryop expresssion");
        }
        case static_cast<int>(Token::Sizeof):
        {
            strict_match(Token::Sizeof);
            strict_match(Token::PL);
            if (curVal.empty())
                throw Exception("expected identifier name to sizeof operator");
            auto expr = std::make_unique<AST::SizeofExpr>(curVal);
            strict_match(Token::Id);
            strict_match(Token::PR);

            return expr;
        }
        case static_cast<int>(Token::Input):
        {
            strict_match(Token::Input);
            strict_match(Token::PL);
            strict_match(Token::PR);
            return std::make_unique<AST::InputExpr>();
        }
        case static_cast<int>(Token::Id): // ident, ident[name], ident(args)
        {
            return parseVarArrayOrFncall();
        }
    }
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseArrayDecls()
{
    const Token type = Token::Type_array;
    match(Token::Type_array); // consume 'array'

    std::unique_ptr<AST::ArrayDeclStmt> arrayDeclStmt(new AST::ArrayDeclStmt);
    while (curTok == static_cast<int>(Token::Id)) 
    {
        std::string varName = curVal;
        match(Token::Id); // consume 'id'

        strict_match(Token::SL);
        auto arrayExpr = parseExpr();
        // parseExpr should throw exception if it isn't able to parse
        assert(arrayExpr != nullptr);

        strict_match(Token::SR);

        arrayDeclStmt->decls.push_back({varName, std::move(arrayExpr)});

        // consume if we got a comma
        match(Token::Comma);
    }

    if (curTok != static_cast<int>(Token::EOL))
        throw Exception("expected EOL; got something else.");
    else if (arrayDeclStmt->decls.size() < 1)
        throw Exception("at least one array should be declared.");

    match(Token::EOL);
    return arrayDeclStmt;
}

std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock();

std::unique_ptr<AST::BaseStmt> parseStmt() 
{
    switch(curTok) 
    {
        case static_cast<int>(Token::CL):
            return parseStmtBlock();
        case static_cast<int>(Token::Type_int):
        case static_cast<int>(Token::Type_bool):
        case static_cast<int>(Token::Type_void):
            return parseVarDecls();
        case static_cast<int>(Token::Type_array):
            return parseArrayDecls();
        case static_cast<int>(Token::Print):
            break;
        case static_cast<int>(Token::If):
            break;
        case static_cast<int>(Token::While):
            break;
        case static_cast<int>(Token::For):
            break;
        case static_cast<int>(Token::Id):
            {
                // two cases
            }
            break;
        case static_cast<int>(Token::Op_divide):
            // FIXME: how to add case for expression here
            break;  
        case static_cast<int>(Token::Return):
            break;
        case static_cast<int>(Token::EOL):
            strict_match(Token::EOL);
            return nullptr;
        default:
            throw Exception("unsupported syntax");
    }

    return nullptr;
}

std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock()
{
    match(static_cast<int>(Token::CL));
    strict_match(Token::EOL); // we want a '\n' no matter what

    std::unique_ptr<AST::StmtBlockStmt> blockStmt(new AST::StmtBlockStmt);
    while (curTok != static_cast<int>(Token::CR)) 
    {
        auto stmt = parseStmt();
        if (stmt)
            blockStmt->stmt_list.push_back(std::move(stmt));
    }

    match(static_cast<int>(Token::CR));

    return blockStmt;
}

std::unique_ptr<AST::BaseStmt> parseFnBody()
{
    return parseStmtBlock();
}

void parseProgram2()
{

    std::cout << "Number of function we have: " << fnDefinitions.size() << std::endl;
    for (int i = 0; i < fnDefinitions.size(); i++) {
        fnDefinitions[i]->print();
    }

    std::cout << "Pass 2: " << std::endl;
    getNextTok();
    for (int i = 0; i < fnDefinitions.size(); i++) {
        skipEOLs();
        bool res = false;
        if (match(static_cast<int>(fnDefinitions[i]->proto->fnType)) &&
            curTok == static_cast<int>(Token::Id) &&
            curVal == fnDefinitions[i]->proto->fnName) 
            {
                skipToFnBody();
                auto stmt = parseFnBody();
                fnDefinitions[i]->body = std::move(stmt);
            }
    }
}

int main(int argc, char* argv[]) {
    std::string filename;
    if (argc <= 1) {
        std::cerr << "error: provide file name as first argument" << std::endl;
        return 1;
    }
    filename = std::string(argv[1]);
    std::cout << "Parsing file: " << filename << std::endl;

    // first pass
    FILE* fp = nullptr;
    if (!filename.empty())
        fp = fopen(filename.c_str(), "r");
    yyrestart(fp);

    try {
        parseProgram1();
    } catch(Exception& exc) {
        std::cerr << curTok << " " << curVal << std::endl;
        exc.print();
        return 1;
    }

    // second pass
    rewind(fp);
    yyrestart(fp);
    try {
        parseProgram2();
    } catch(Exception& exc) {
        std::cerr << curTok << " " << curVal << std::endl;
        exc.print();
        return 1;
    }

    fclose(fp);
   
    return 0;
}

