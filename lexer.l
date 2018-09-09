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
"["                        { printf("["); }
"]"                        { printf("]"); }
"{"                        { printf("{"); return static_cast<int>(Token::CL); }
"}"                        { printf("}"); return static_cast<int>(Token::CR); }
"+"                        { printf("+"); }
"-"                        { printf("-"); }
"*"                        { printf("*"); }
"^"                        { printf("^"); }
"/"                        { printf("/"); }
"%"                        { printf("%"); }
"&"                        { printf("&"); }
"|"                        { printf("|"); }
"="                        { printf("="); }
"=="                       { printf("=="); }
">"                        { printf(">"); }
">="                       { printf(">="); }
"<"                        { printf("<"); }
"<="                       { printf("<="); }
":"                        { printf(":"); }
"?"                        { printf("?"); }
"!"                        { printf("!"); }
","                        { printf(","); return static_cast<int>(Token::Comma); }
\".*\"                     { printf("(slit:%s)", yytext); }

"bool"                     { printf("bool"); strval = "bool"; return static_cast<int>(Token::Type_bool); }
"int"                      { printf("int"); strval = "int"; return static_cast<int>(Token::Type_int); }
"array"                    { printf("array"); strval = "array"; return static_cast<int>(Token::Type_array); }
"void"                     { printf("void"); strval = "void"; return static_cast<int>(Token::Type_void); }

"true"                     { printf("true"); }
"false"                    { printf("false"); }
"sizeof"                   { printf("sizeof"); }
"input"                    { printf("input;"); }
"print"                    { printf("print"); }
"if"                       { printf("if"); }
"while"                    { printf("while"); }
"for"                      { printf("for"); }
"return"                   { printf("return"); }

\n                         { printf("<EOL>\n"); return static_cast<int>(Token::EOL); }
[\t\r ]+                   { printf("%s", yytext); }

;.*                        { printf("<Comment:\"%s\">", yytext); }
{NUMBER}                   { printf("(%s:NUMBER)", yytext); strval = std::string(yytext); return static_cast<int>(Token::Number); }
{ID}                       { printf("(%s:ID)", yytext); strval = std::string(yytext); return static_cast<int>(Token::Id); }
<<EOF>>                    { return -1; }

.                          { printf("(%s:ERR)", yytext); }

%%