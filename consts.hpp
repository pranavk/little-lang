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
    Err             = 0,

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
    Op_neq          = 19,
    Op_eqeq         = 20,
    Op_gt           = 21,
    Op_gte          = 22,
    Op_lt           = 23,
    Op_lte          = 24,
    Op_colon        = 25,
    Op_question     = 26,
    Op_bang         = 27,
    Comma           = 28,

    PL              = 29, // paren left - (
    PR              = 30, // paren right - )
    CL              = 31, // curly left - {
    CR              = 32, // curly right - }
    SL              = 33, // square left - [
    SR              = 34, // square right - ]

    Print           = 35,
    Input           = 36,
    If              = 37,
    While           = 38,
    For             = 39,
    Return          = 40,
    Sizeof          = 41,
};

extern std::string strval;

int yylex();
void yyrestart(FILE* fp);
