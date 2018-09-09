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

"("                        { printf("("); return static_cast<int>(Token::PL); }
")"                        { printf(")"); return static_cast<int>(Token::PR); }
"["                        { printf("["); return static_cast<int>(Token::SL); }
"]"                        { printf("]"); return static_cast<int>(Token::SR); }
"{"                        { printf("{"); return static_cast<int>(Token::CL); }
"}"                        { printf("}"); return static_cast<int>(Token::CR); }
"+"                        { printf("+"); return static_cast<int>(Token::Op_add); }
"-"                        { printf("-"); return static_cast<int>(Token::Op_minus); }
"*"                        { printf("*"); return static_cast<int>(Token::Op_mult); }
"^"                        { printf("^"); return static_cast<int>(Token::Op_exp); }
"/"                        { printf("/"); return static_cast<int>(Token::Op_divide); }
"%"                        { printf("%"); return static_cast<int>(Token::Op_mod); }
"&"                        { printf("&"); return static_cast<int>(Token::Op_and); }
"|"                        { printf("|"); return static_cast<int>(Token::Op_or); }
"="                        { printf("="); return static_cast<int>(Token::Op_eq); }
"=="                       { printf("=="); return static_cast<int>(Token::Op_eqeq); }
">"                        { printf(">"); return static_cast<int>(Token::Op_gt); }
">="                       { printf(">="); return static_cast<int>(Token::Op_gte); }
"<"                        { printf("<"); return static_cast<int>(Token::Op_lt); }
"<="                       { printf("<="); return static_cast<int>(Token::Op_lte); }
":"                        { printf(":"); return static_cast<int>(Token::Op_colon); }
"?"                        { printf("?"); return static_cast<int>(Token::Op_question); }
"!"                        { printf("!"); return static_cast<int>(Token::Op_bang); }
","                        { printf(","); return static_cast<int>(Token::Comma); }
\".*\"                     { printf("(slit:%s)", yytext); return static_cast<int>(Token::Literal_string); }

"bool"                     { printf("bool"); strval = "bool"; return static_cast<int>(Token::Type_bool); }
"int"                      { printf("int"); strval = "int"; return static_cast<int>(Token::Type_int); }
"array"                    { printf("array"); strval = "array"; return static_cast<int>(Token::Type_array); }
"void"                     { printf("void"); strval = "void"; return static_cast<int>(Token::Type_void); }

"true"                     { printf("true"); return static_cast<int>(Token::Literal_true); }
"false"                    { printf("false"); return static_cast<int>(Token::Literal_false); }
"sizeof"                   { printf("sizeof"); return static_cast<int>(Token::Sizeof); }
"input"                    { printf("input;"); return static_cast<int>(Token::Input); }
"print"                    { printf("print"); return static_cast<int>(Token::Print); }
"if"                       { printf("if"); return static_cast<int>(Token::If); }
"else"                     { printf("else"); return static_cast<int>(Token::Else); }
"while"                    { printf("while"); return static_cast<int>(Token::While); }
"for"                      { printf("for"); return static_cast<int>(Token::For); }
"return"                   { printf("return"); return static_cast<int>(Token::Return); }

\n                         { printf("<EOL>\n"); return static_cast<int>(Token::EOL); }
[\t\r ]+                   { printf("%s", yytext); }

;.*                        { printf("<Comment:\"%s\">", yytext); }
{NUMBER}                   { printf("(%s:NUMBER)", yytext); strval = std::string(yytext); return static_cast<int>(Token::Number); }
{ID}                       { printf("(%s:ID)", yytext); strval = std::string(yytext); return static_cast<int>(Token::Id); }
<<EOF>>                    { return -1; }

.                          { printf("(%s:ERR)", yytext); }

%%