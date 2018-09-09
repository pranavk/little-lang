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
    Number          = 1,
    Id              = 2,

    Type_array      = 3,
    Type_int        = 4,
    Type_bool       = 5,
    Type_void       = 6,

    Literal_true    = 7,
    Literal_false   = 8,

    EOL             = 9,

    Op_add          = 10, // can be any of the op
    Op_minus        = 11,
    Op_mult         = 12,
    Op_divide       = 13,
    Op_exp          = 14,
    Op_mod          = 15,
    Op_and          = 16,
    Op_or           = 17,
    Op_eq           = 18,
    Op_eqeq         = 19,
    Op_gt           = 20,
    Op_gte          = 21,
    Op_lt           = 22,
    Op_lte          = 23,
    Op_colon        = 24,
    Op_question     = 25,
    Op_bang         = 26,
    Comma           = 27,

    PL              = 28, // paren left - (
    PR              = 29, // paren right - )
    CL              = 30, // curly left - {
    CR              = 31, // curly right - }
    SL              = 32, // square left - [
    SR              = 33, // square right - ]

    Print           = 34,
    Input           = 35,
    If              = 36,
    While           = 37,
    For             = 38,
    Return          = 39,
    Sizeof          = 40
};

extern std::string strval;

int yylex();
void yyrestart(FILE* fp);
