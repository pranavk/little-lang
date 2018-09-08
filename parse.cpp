#include <consts.hpp>
#include <string>
#include <iostream>
#include <baseast.hpp>
#include <memory>
#include "lex.yy.c"

// check lexer.l to see how strval is used
std::string strval;

int nexttok = -1;
std::string nextval;

int peek()
{
    if (nexttok == -1)
    {
	    nexttok = static_cast<int>(yylex());
	    nextval = strval;
	    std::cout << "nexttok: " << nexttok << "(" << nextval << ")" << std::endl;
    }

    return nexttok;
}

bool match(Token tok)
{
    bool match = false;
    if (tok == peek())
    {
	    nexttok = -1;
	    nextval = "";
	    match = true;
    }
    return match;
}

std::unique_ptr<AST::FunctionDefinition> parseFnDef() 
{

}

bool parseFn(AST::FunctionDefinition& fnDef) 
{

    return true;
}

bool parseProgram() 
{
    bool result = true;
    
    // gather all function definition in first parse
    std::vector<std::unique_ptr<AST::FunctionDefinition>> fnDefinitions;
    while (auto fnDef = parseFnDef())
    {
        fnDefinitions.push_back(std::move(fnDef));
    }

    // parse each function's body now
  

    return result;
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
    while (yylex() != EOF)
    {

    }

    // second pass
    rewind(fp);
    yyrestart(fp);
    while (yylex() != EOF);

    fclose(fp);
   
    return 0;
}

