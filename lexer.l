%{
#include <stdio.h>
#include <iostream>
#include "consts.hpp"
%}

DIGIT [0-9]
ALPHA [a-zA-Z_]

ID {ALPHA}+({ALPHA}|{DIGIT})*
NUMBER [-]?{DIGIT}+

/* useful to tell user about lineno. when parser errs. */
%option yylineno 
/* behaves as if yywrap() returns 1 if following is defined. */
%option noyywrap

%%

\\\n                       { }
"("                        { return static_cast<int>(Token::PL); }
")"                        { return static_cast<int>(Token::PR); }
"["                        { return static_cast<int>(Token::SL); }
"]"                        { return static_cast<int>(Token::SR); }
"{"                        { return static_cast<int>(Token::CL); }
"}"                        { return static_cast<int>(Token::CR); }
"+"                        { return static_cast<int>(Token::Op_add); }
"-"                        { return static_cast<int>(Token::Op_minus); }
"*"                        { return static_cast<int>(Token::Op_mult); }
"^"                        { return static_cast<int>(Token::Op_exp); }
"/"                        { return static_cast<int>(Token::Op_divide); }
"%"                        { return static_cast<int>(Token::Op_mod); }
"&"                        { return static_cast<int>(Token::Op_and); }
"|"                        { return static_cast<int>(Token::Op_or); }
":="                       { return static_cast<int>(Token::Op_assignment); }
"!="                       { return static_cast<int>(Token::Op_neq); }
"=="                       { return static_cast<int>(Token::Op_eqeq); }
">"                        { return static_cast<int>(Token::Op_gt); }
">="                       { return static_cast<int>(Token::Op_gte); }
"<"                        { return static_cast<int>(Token::Op_lt); }
"<="                       { return static_cast<int>(Token::Op_lte); }
":"                        { return static_cast<int>(Token::Op_colon); }
"?"                        { return static_cast<int>(Token::Op_question); }
"!"                        { return static_cast<int>(Token::Op_bang); }
","                        { return static_cast<int>(Token::Comma); }
\"(\\.|[^\\"])*\"          { return static_cast<int>(Token::Literal_string); }

"bool"                     { return static_cast<int>(Token::Type_bool); }
"int"                      { return static_cast<int>(Token::Type_int); }
"array"                    { return static_cast<int>(Token::Type_array); }
"void"                     { return static_cast<int>(Token::Type_void); }

"true"                     { return static_cast<int>(Token::Literal_true); }
"false"                    { return static_cast<int>(Token::Literal_false); }
"sizeof"                   { return static_cast<int>(Token::Sizeof); }
"input"                    { return static_cast<int>(Token::Input); }
"print"                    { return static_cast<int>(Token::Print); }
"if"                       { return static_cast<int>(Token::If); }
"else"                     { return static_cast<int>(Token::Else); }
"while"                    { return static_cast<int>(Token::While); }
"for"                      { return static_cast<int>(Token::For); }
"return"                   { return static_cast<int>(Token::Return); }
"abort"                    { return static_cast<int>(Token::Abort); }

\n                         { return static_cast<int>(Token::EOL); }
[\t\r ]+                   { }

;.*                        { }
{NUMBER}                   { return static_cast<int>(Token::Number); }
{ID}                       { return static_cast<int>(Token::Id); }
<<EOF>>                    { return static_cast<int>(Token::End); }

.                          { }

%%
