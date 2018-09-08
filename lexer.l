%{
#include <stdio.h>
%}

DIGIT [0-9]
ALPHA [a-zA-Z_]

ID {ALPHA}+({ALPHA}|{DIGIT})*
NUMBER [-]?{DIGIT}+

%%

"("                        { printf("("); }
")"                        { printf(")"); }
"["                        { printf("["); }
"]"                        { printf("]"); }
"{"                        { printf("{"); }
"}"                        { printf("}"); }
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
","                        { printf(","); }
\".*\"                     { printf("(slit:%s)", yytext); }

"bool"                     { printf("bool"); }
"int"                      { printf("int"); }
"array"                    { printf("array"); }
"void"                     { printf("void"); }

"true"                     { printf("true"); }
"false"                    { printf("false"); }
"sizeof"                   { printf("sizeof"); }
"input"                    { printf("input;"); }
"print"                    { printf("print"); }
"if"                       { printf("if"); }
"while"                    { printf("while"); }
"for"                      { printf("for"); }
"return"                   { printf("return"); }

\n                         { printf("<EOL>\n"); }
[\t\r ]+                   { printf("%s", yytext); }

;.*                        { printf("<Comment:\"%s\">", yytext); }
{NUMBER}                   { printf("(%s:NUMBER)", yytext); }
{ID}                       { printf("(%s:ID)", yytext); }

.                          { printf("(%s:ERR)", yytext); }

%%

int main(void) {
   yylex();
   return 0;
}
