#pragma once

#include <string>
#include <iostream>

enum class ExceptionType 
{
    Parser,
    Type
};

class Exception : public std::runtime_error 
{
    ExceptionType _type;
    public:
        Exception(std::string msg, ExceptionType type = ExceptionType::Parser) :
            std::runtime_error(msg), _type(type) { };
        void print() {
            std::cerr << (_type == ExceptionType::Parser ? "parser error: " : "type error: ") << std::string(what()) << std::endl;
        }
};

enum class Token
{
    End             = 0,

    Number          = 1,
    Id              = 2,

    Type_array      = 3,
    Type_int        = 4,
    Type_bool       = 5,
    Type_void       = 6,

    Literal_true    = 7,
    Literal_false   = 8,
    Literal_string  = 9,

    EOL             = 10,

    Op_add          = 11, // can be any of the op
    Op_minus        = 12,
    Op_mult         = 13,
    Op_divide       = 14,
    Op_exp          = 15,
    Op_mod          = 16,
    Op_and          = 17,
    Op_or           = 18,
    Op_assignment   = 19,
    Op_neq          = 20,
    Op_eqeq         = 21,
    Op_gt           = 22,
    Op_gte          = 23,
    Op_lt           = 24,
    Op_lte          = 25,
    Op_colon        = 26,
    Op_question     = 27,
    Op_bang         = 28,
    Comma           = 29,

    PL              = 30, // paren left - (
    PR              = 31, // paren right - )
    CL              = 32, // curly left - {
    CR              = 33, // curly right - }
    SL              = 34, // square left - [
    SR              = 35, // square right - ]

    Print           = 36,
    Input           = 37,
    If              = 38,
    Else            = 39,
    While           = 40,
    For             = 41,
    Return          = 42,
    Sizeof          = 43
};

extern int yylineno;
extern char* yytext;

int yylex();
void yyrestart(FILE* fp);
