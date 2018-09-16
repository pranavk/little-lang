#include <string>
#include <iostream>
#include <memory>
#include <set>
#include <cstring>
#include <cassert>

#include "consts.hpp"
#include "baseast.hpp"
#include "visitor.hpp"

struct TokenInfo {
    int lineno;
    std::string token;
    Token type;
};

// contains all the tokens
static std::vector<TokenInfo> tokens;

int curTok = -1;
int curTokIdx = -1;
std::string curVal;

static std::set<std::string> args;
bool isOn(const std::string& opt) {
    return args.find(opt) != args.end();
}

int getNextTok()
{
	TokenInfo curTokObj = tokens[++curTokIdx];
    curTok = static_cast<int>(curTokObj.type);
	curVal = curTokObj.token;
    return curTok;
}

void updateTokenIdx(int tokIdx) {
    curTokIdx = tokIdx - 1;
    getNextTok();
}

void skipEOLs()
{
    while (curTok == static_cast<int>(Token::EOL) && getNextTok());
}

bool match(int tok)
{  
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
            break;
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
            break;
        default:
            res = false;
    }

    return res;
}

bool strict_match(Token tok) 
{
    bool res = false;
    if (!(res = match(tok)))
        throw Exception("Parse error near line: " + std::to_string(tokens[curTokIdx].lineno) +
                        "\tExpected " + std::to_string(static_cast<int>(tok)) +
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

std::unique_ptr<AST::BaseStmt> parseVarDecls(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (isTokenType(curTok) && curTok != static_cast<int>(Token::Type_array)) {
        Token varType = static_cast<Token>(curTok);
        std::unique_ptr<AST::VarDeclStmt> varDeclStmt(new AST::VarDeclStmt);

        match(curTok); // consume the type
        while (curTok == static_cast<int>(Token::Id)) 
        {
            std::string varName = curVal;
            match(Token::Id); // consume 'id'

            varDeclStmt->decls.push_back({varType, varName});

            // check if you got a comma without skipping EOLs
            if (curTok == static_cast<int>(Token::Comma))
                match(Token::Comma);
        }

        if (varDeclStmt->decls.size() >= 1)
            return varDeclStmt;
    }

    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseExpr(int tokIdx);

bool parseFnArgs(int tokIdx, std::vector<std::unique_ptr<AST::BaseExpr>>& fnArgs)
{
    updateTokenIdx(tokIdx);

    if (match(Token::PL))
    {
        bool first = true;
        while (curTok != static_cast<int>(Token::PR))
        {
            // consume comma if it's not the first iteration.
            if (!first && !match(Token::Comma))
                return false;

            auto expr = parseExpr(curTokIdx);
            fnArgs.push_back(std::move(expr));

            first = false;
        }

        if (match(Token::PR))
            return true;
    }
    return false;
}

std::unique_ptr<AST::BaseExpr> parseTrueExpr(int tokIdx) {
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseExpr> res = nullptr;
    if (match(Token::Literal_true)) {
        res = std::make_unique<AST::BaseExpr>(std::make_unique<AST::BoolValue>(true));
        return res;
    }

    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseFalseExpr(int tokIdx) {
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseExpr> res = nullptr;
    if (match(Token::Literal_false)) {
        res = std::make_unique<AST::BaseExpr>(std::make_unique<AST::BoolValue>(false));
        return res;
    }

    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseNumberExpr(int tokIdx) {
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseExpr> res = nullptr;
    int val = std::atoi(curVal.c_str());
    if (match(Token::Number)) {
        auto result = std::make_unique<AST::IntValue>(val);
        res = std::make_unique<AST::BaseExpr>(std::move(result)); 
        return res;
    }
    
    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseIdentExpr(int tokIdx) {
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseExpr> res = nullptr;
    auto ident = curVal;
    if (match(Token::Id)) {
        res = std::make_unique<AST::IdExpr>(ident); 
        return res;
    }
    
    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseTernaryExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::PL)) {
        auto condE = parseExpr(curTokIdx);
        if (condE && match(Token::Op_question)) {
            auto trueE = parseExpr(curTokIdx);
            if (trueE && match(Token::Op_colon)) {
                auto falseE = parseExpr(curTokIdx);
                if (match(Token::PR)) {
                    return std::make_unique<AST::TernaryExpr>(std::move(condE),
                                                              std::move(trueE),
                                                              std::move(falseE)); 
                }
            }
        }
    }

    return nullptr;
}


std::unique_ptr<AST::BaseExpr> parseSizeofExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::Sizeof) && match(Token::PL)) {
        auto idExpr = parseExpr(curTokIdx);
        if (idExpr && match(Token::PR))
            return std::make_unique<AST::SizeofExpr>(idExpr);
    }

    return nullptr;
}


std::unique_ptr<AST::BaseExpr> parseInputExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::Input) && match(Token::PL) && match(Token::PR)) {
        return std::make_unique<AST::InputExpr>();
    }

    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseArrayExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::Id)) {
        std::string name = curVal;
        if (match(Token::SL)) {
            auto expr = parseExpr(curTokIdx);
            if (expr && match(Token::SR))
                return std::make_unique<AST::ArrayExpr>(name, std::move(expr));
        }
    }

    return nullptr;
}


std::unique_ptr<AST::BaseExpr> parseFncallExpr(int tokIdx) {
    updateTokenIdx(tokIdx);
    std::string name = curVal;
    if (match(Token::Id)) {
        std::vector<std::unique_ptr<AST::BaseExpr>> fnArgs;
        if (curTok == static_cast<int>(Token::PL) && parseFnArgs(curTokIdx, fnArgs))
            return std::make_unique<AST::FnCallExpr>(name, fnArgs);
    }

    return nullptr;
}


std::unique_ptr<AST::BaseExpr> parseBinopExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::PL)) {
        auto lExpr = parseExpr(curTokIdx);
        if (isBinOp()) {
            Token op = static_cast<Token>(curTok);
            if (match(curTok)) { // consume binop
                auto rExpr = parseExpr(curTokIdx);
                if (rExpr && match(Token::PR))
                    return std::make_unique<AST::BinopExpr>(std::move(lExpr),
                                                            op,
                                                            std::move(rExpr));
            }
        }
    }

    return nullptr;
}


std::unique_ptr<AST::BaseExpr> parseUnaryopExpr(int tokIdx) {
    updateTokenIdx(tokIdx);

    if (match(Token::PL)) {
        if (isUnaryOp()) {
            Token op = static_cast<Token>(curTok);
            if (match(curTok)) { // consume unaryop
                auto rExpr = parseExpr(curTokIdx);
                if (rExpr && match(Token::PR))
                    return std::make_unique<AST::UnaryExpr>(op, rExpr);
            }
        }
    }

    return nullptr;
}

std::unique_ptr<AST::BaseExpr> parseExpr(int tokIdx) 
{
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseExpr> res = nullptr;

    if (res = parseTrueExpr(tokIdx)) {
        return res;
    } else if (res = parseFalseExpr(tokIdx)) {
        return res;
    } else if (res = parseNumberExpr(tokIdx)) {
        return res;
    } else if (res = parseTernaryExpr(tokIdx)) {
        return res;
    } else if (res = parseSizeofExpr(tokIdx)) {
        return res;
    } else if (res = parseInputExpr(tokIdx)) {
        return res;
    } else if (res = parseArrayExpr(tokIdx)) { // ident[expr]
        return res;
    } else if (res = parseFncallExpr(tokIdx)) { // ident(expr)
        return res;
    } else if (res = parseIdentExpr(tokIdx)) { // ident
        return res;
    } else if (res = parseBinopExpr(tokIdx)) {
        return res;
    } else if (res = parseUnaryopExpr(tokIdx)) {
        return res;
    }

    updateTokenIdx(tokIdx);
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseArrayDecls(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::Type_array))
    {
        match(Token::Type_array); // consume 'array'
        const Token type = Token::Type_array;
        std::unique_ptr<AST::ArrayDeclStmt> arrayDeclStmt(new AST::ArrayDeclStmt);
        bool first = true;
        while (curTok != static_cast<int>(Token::EOL))
        {
            if (!first && !match(Token::Comma))
                return nullptr;

            std::string varName = curVal;
            match(Token::Id); // consume 'id'

            if (match(Token::SL)) {
                auto arrayExpr = parseExpr(curTokIdx);
                if (match(Token::SR)) {
                    arrayDeclStmt->decls.push_back({varName, std::move(arrayExpr)});
                    first = false;
                }
            }
        }

        if (arrayDeclStmt->decls.size() >= 1) {
            return arrayDeclStmt;
        }
    }

    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseVarAssignment(int tokIdx)
{    
    updateTokenIdx(tokIdx);
    
    std::string identName = curVal;
    if (match(Token::Id) && match(Token::Op_assignment)) {
        auto rhsExpr = parseExpr(curTokIdx);
        if (rhsExpr) {
            return std::make_unique<AST::VarAssignment>(identName, rhsExpr);
        }
    }
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseArrayAssignment(int tokIdx)
{
    updateTokenIdx(tokIdx);
    
    std::string identName = curVal;
    if (match(Token::Id) && match(Token::SL)) {
        auto idxExpr = parseExpr(curTokIdx);
        if (idxExpr && match(Token::SR) && match(Token::Op_assignment)) {
            auto expr = parseExpr(curTokIdx);
            if (expr) {
                return std::make_unique<AST::ArrayAssignment>(identName, idxExpr, expr);
            }
        }
    }

    return nullptr;
}

bool parsePrintArgs(std::vector<std::unique_ptr<AST::BaseExpr>>& printArgs)
{
    bool ret = false;
    if (match(Token::PL))
    {
        bool first = true;
        ret = true;
        while (!match(Token::PR))
        {
            if (!first && !match(Token::Comma))
                return false;

            std::unique_ptr<AST::BaseExpr> res = nullptr;
            if (curTok == static_cast<int>(Token::Literal_string))
            {
                std::string val = curVal;
                match(curTok); // consume string literal
                res.reset(new AST::StringLiteralExpr(val));
            }
            else if (res = parseExpr(curTokIdx))
            {
                // yay!
            }

            first = false;
            if (!res)
                return false;
            printArgs.push_back(std::move(res));      
        }
    }

    // atleast one argument is required
    ret = ret && printArgs.size() > 0;
    return ret;
}

std::unique_ptr<AST::BaseStmt> parsePrintStmt(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::Print)) {
        std::vector<std::unique_ptr<AST::BaseExpr>> args;
        if (parsePrintArgs(args)) {
            return std::make_unique<AST::PrintStmt>(args);
        }
    }

    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseStmt(int tokIdx);
std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock(int tokIdx);

std::unique_ptr<AST::BaseStmt> parseIfStmt(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::If) && match(Token::PL)) {
        auto condExpr = parseExpr(curTokIdx);
        if (match(Token::PR)) {
            auto body = parseStmtBlock(curTokIdx);
            std::unique_ptr<AST::StmtBlockStmt> elseStmt = nullptr;
            if (match(Token::Else)) {
                elseStmt = parseStmtBlock(curTokIdx);
            }
            if (body)
                return std::make_unique<AST::IfStmt>(condExpr, body, elseStmt);     
        }
    }

    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseWhileStmt(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::While) && match(Token::PL)) {
        auto condExpr = parseExpr(curTokIdx);
        if (condExpr && match(Token::PR)) {
            auto body = parseStmtBlock(curTokIdx);
            if (body)
                return std::make_unique<AST::WhileStmt>(condExpr, body);
        }
    }
   
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseForStmt(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::For) && match(Token::PL)) {
        auto idExpr = parseExpr(curTokIdx);
        if (idExpr && match(Token::Op_colon)) {
            auto containerExpr = parseExpr(curTokIdx);
            if (containerExpr && match(Token::PR)) {
                auto body = parseStmtBlock(curTokIdx);
                if (body) {
                    return std::make_unique<AST::ForStmt>(idExpr, containerExpr, body);
                }
            }
        }
    }
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseReturnStmt(int tokIdx)
{
    updateTokenIdx(tokIdx);

    if (match(Token::Return)) {
        auto expr = parseExpr(curTokIdx);
        return std::make_unique<AST::ReturnStmt>(expr);
    }

    return nullptr;
}

std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock();

std::unique_ptr<AST::BaseStmt> parseStmt(int tokIdx) 
{
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::BaseStmt> res = nullptr;
    if (res = parseStmtBlock(tokIdx)) {
        return res;
    }
    if (res = parseVarDecls(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parseArrayDecls(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parsePrintStmt(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parseIfStmt(tokIdx))
    {
        return res;
    }
    if (res = parseWhileStmt(tokIdx))
    {
        return res;
    }
    if (res = parseForStmt(tokIdx))
    {
        return res;
    }
    if (res = parseVarAssignment(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parseArrayAssignment(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parseExpr(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
    if (res = parseReturnStmt(tokIdx))
    {
        if (match(Token::EOL))
            return res;
    }
   
    throw Exception("Parse error near line: " + std::to_string(tokens[curTokIdx].lineno) +
                    " token: " + tokens[curTokIdx].token);
}

std::unique_ptr<AST::StmtBlockStmt> parseStmtBlock(int tokIdx)
{
    updateTokenIdx(tokIdx);
    std::unique_ptr<AST::StmtBlockStmt> blockStmt = nullptr;

    if (match(Token::CL) && match(Token::EOL))
    {
        blockStmt.reset(new AST::StmtBlockStmt);
        skipEOLs();
        while (curTok != static_cast<int>(Token::CR))
        {
            auto stmt = parseStmt(curTokIdx);
            if (stmt)
                blockStmt->stmt_list.push_back(std::move(stmt));
            skipEOLs();
        }
    }

    if (match(static_cast<int>(Token::CR)))
        return blockStmt;
    return nullptr;
}

std::unique_ptr<AST::BaseStmt> parseFnBody()
{
    return parseStmtBlock(curTokIdx);
}

bool parseFnParams(std::vector<AST::FnParam>& args)
{
    // consume left parenthesis first
    bool res = match(static_cast<int>(Token::PL));
    bool first = true;
    while (res && curTok != static_cast<int>(Token::PR)) 
    {
        // consume a "," if it's not the first iteration
        if (!first && !match(Token::Comma))
            return false;

        AST::FnParam param;
        res = false;
        if (isTokenType(curTok))
        {
            param.type = static_cast<Token>(curTok);
            if (getNextTok() == static_cast<int>(Token::Id))
            {
                param.name = curVal;
                res = true;
            }
        }

        first = false;
        if (res) {
            args.push_back(param);
            getNextTok();
        }
    }

    res = res && match(static_cast<int>(Token::PR));

    return res;
}

std::unique_ptr<AST::FunctionDefinition> parseFnDef() 
{
    std::vector<AST::FnParam> params;
    std::string fnName;
    int fnType;
    if (!isTokenType(fnType = curTok) || !match(curTok))
        throw Exception("Unexpected type found.");
    
    fnName = curVal;
    if (!match(Token::Id))
        throw Exception("Bad function name");

    if (!parseFnParams(params))
        throw Exception("Cannot parse function arguments.");

    std::unique_ptr<AST::FunctionPrototype> proto(
        new AST::FunctionPrototype(fnType, fnName, params));
    auto stmt = parseFnBody();
    if (!stmt)
        throw Exception("Couldn't parse function body.");

    return std::make_unique<AST::FunctionDefinition>(std::move(proto), std::move(stmt));
}

void parseProgram(AST::Program& program) 
{
    skipEOLs();
    while (auto fnDef = parseFnDef())
    {
        program.fnDefinitions.push_back(std::move(fnDef));
        if (!match(Token::EOL))
            throw Exception("New line required after fn body.");
        skipEOLs();
        if (match(Token::End))
            break;
    }
}

int main(int argc, char* argv[]) {
    std::string filename;
    for (int i = 1; i < argc; i++) {
        std::string tmp = std::string(argv[i]);
        if (tmp.length() > 2 && tmp[0] == '-' && tmp[1] == '-') {
            args.insert(tmp.substr(2));
        } else {
            filename = tmp;
        }
    }

    if (isOn("help")) {
        std::cout << argv[0] << " OPTIONS FILENAME" << std::endl;
        std::cout << "OPTIONS" << std::endl;
        std::cout << "\t--print-ast\tPrints the AST of the parsed program" << std::endl;
        std::cout << "\t--help\t\tPrints this help" << std::endl;
        return 0;
    }
    if (filename.empty()) {
        std::cerr << "error: provide file name as argument" << std::endl;
        return 1;
    }
    std::cout << "Parsing file: " << filename << " ... ";

    // get all the tokens in the buffer
    FILE* fp = nullptr;
    if (!filename.empty())
        fp = fopen(filename.c_str(), "r");
    yyrestart(fp);

    int token = -1;
    while ((token = yylex()) != 0) {
        tokens.push_back({yylineno, std::string(yytext), static_cast<Token>(token)});
    }
    tokens.push_back({yylineno, "EOF", Token::End});

    fclose(fp);

    AST::Program programNode;
    updateTokenIdx(0);
    try {
        parseProgram(programNode);
        if (isOn("print-ast")) {
            Visitor::PrintASTVisitor printVisitor;
            printVisitor.visit(&programNode);
        }
        std::cout << std::endl << "Typechecking ..." << std::endl;
        Visitor::TypecheckerVisitor typecheckerVisitor;
        typecheckerVisitor.visit(&programNode);
    } catch(Exception& exc) {
        std::cerr << curTok << " " << curVal << std::endl;
        std::cout << "NOK" << std::endl;
        exc.print();
        return 1;   
    }

    std::cout << "OK" << std::endl;
   
    return 0;
}

