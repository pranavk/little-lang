#include <string>

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
    SR // square right - ]
};

extern std::string strval;

int yylex();
