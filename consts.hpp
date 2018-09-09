#pragma once

#include <string>

enum class ExceptionType 
{
    Parser
};

class Exception : public std::runtime_error 
{
    ExceptionType type;
    public:
        Exception(std::string msg) :
            std::runtime_error(msg), type(ExceptionType::Parser) { };
        void print() {
            std::cout << "parser error: " << std::string(what()) << std::endl;
        }
};

enum class Token
{
    Number,
    Id,

    Type_array,
    Type_int,
    Type_bool,
    Type_void,

    Literal_true,
    Literal_false,

    EOL,

    Op_add, // can be any of the op
    Op_minus,
    Op_mult,
    Op_divide,

    PL, // paren left - (
    PR, // paren right - )
    CL, // curly left - {
    CR, // curly right - }
    SL, // square left - [
    SR, // square right - ]
    Comma
};

extern std::string strval;

int yylex();
void yyrestart(FILE* fp);
