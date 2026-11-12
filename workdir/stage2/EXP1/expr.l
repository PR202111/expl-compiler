%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "exprtree.h"
#include "y.tab.h" // Yacc generated header
%}

%%

"begin" { return BEGIN_TOK; }
"end"   { return END_TOK; }
"read"  { return READ; }
"write" { return WRITE; }

[a-z]   { 
            yylval.str_val = strdup(yytext); // Pass the variable name to Yacc
            return ID; 
        }

[0-9]+  { 
            yylval.int_val = atoi(yytext); // Pass the integer value to Yacc
            return NUM; 
        }

"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return MUL; }
"/"     { return DIV; }
"="     { return ASSIGN; }
";"     { return SEMICOLON; }
"("     { return '('; }
")"     { return ')'; }

[ \t\n] { /* Ignore whitespace */ }

.       { 
            printf("Unknown character: %s\n", yytext); 
            exit(1); 
        }

%%

int yywrap(void) {
    return 1;
}