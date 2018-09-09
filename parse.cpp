#include <string>
#include <iostream>
#include <memory>

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

bool parseFnArgs(std::vector<AST::FnArg>& args)
{
    // consume left parenthesis first
    bool res = match(static_cast<int>(Token::PL));

    while (res && curTok != static_cast<int>(Token::PR)) 
    {
        // consume a "," if it's there
        match(static_cast<int>(Token::Comma));

        AST::FnArg arg;
        res = false;
        if (isTokenType(curTok))
        {
            arg.type = static_cast<Token>(curTok);
            if (getNextTok() == static_cast<int>(Token::Id))
            {
                arg.name = strval;
                res = true;
            }
        }

        if (res) {
            args.push_back(arg);
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
    std::vector<AST::FnArg> args;
    std::string fnName;
    int fnType;
    if (isTokenType(fnType = curTok) && 
        (getNextTok() == static_cast<int>(Token::Id)) && !((fnName = curVal).empty()) &&
        getNextTok() && parseFnArgs(args))
    {
        std::unique_ptr<AST::FunctionPrototype> proto(
            new AST::FunctionPrototype(fnType, fnName, args));
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

void parseStmt() 
{
    
}

std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock()
{
    match(static_cast<int>(Token::CL));
    while (curTok != static_cast<int>(Token::CR)) 
    {
        parseStmt();
    }

    match(static_cast<int>(Token::CR));

    return nullptr;
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
        exc.print();
        return 1;
    }

    // second pass
    rewind(fp);
    yyrestart(fp);
    try {
        parseProgram2();
    } catch(Exception& exc) {
        exc.print();
        return 1;
    }

    fclose(fp);
   
    return 0;
}

