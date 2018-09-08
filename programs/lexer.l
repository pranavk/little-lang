%{
#include <stdio.h>
%}

NUMBER [0-9]
ALPHAS [a-zA-Z]
ID {ALPHAS}+{NUMBER}*

%option noyywrap

%%

{NUMBER}+ {
printf("Saw a number: %s\n", yytext);
return 1;
}

{ALPHAS}+ {
printf("Saw a alpha: %s\n", yytext);
return 2;
}

{ID} {
printf("Saw an ID: %s\n", yytext);
return 3;
}

%%

int main(void) {
   yylex();
   return 0;
}
