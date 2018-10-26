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
    Err             = -1,
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
    Sizeof          = 43,
    Abort           = 44
};

inline bool isUnaryOp(int tok)
{
    bool res = false;
    switch(static_cast<Token>(tok))
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

inline bool isTokenType(int tok)
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

inline bool isBinOp(Token tok) {
    return static_cast<int>(tok);
}

inline bool isBinOp(int tok)
{
    bool res = false;
    switch(static_cast<Token>(tok))
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

inline bool isAndOr(Token op) {
    bool res = false;
    switch(op) {
        case Token::Op_and:
        case Token::Op_or:
            res = true;
            break;
        default:
            res = false;
    }

    return res;
}

inline bool isCompOp(Token op) {
    bool res = false;
    switch(op) {
        case Token::Op_neq:
        case Token::Op_eqeq:
        case Token::Op_gt:
        case Token::Op_gte:
        case Token::Op_lt:
        case Token::Op_lte:
        case Token::Op_bang:
            res = true;
            break;
        default:
            res = false;
    }

    return res;
}

inline bool isEqOrNeq(Token op) {
    bool res = false;
    switch(op) {
        case Token::Op_eqeq:
        case Token::Op_neq:
            res = true;
            break;
        default:
            res = false;
            break;
    }

    return res;
}

extern int yylineno;
extern char* yytext;
extern std::string tokenVal;

int yylex();
void yyrestart(FILE* fp);
