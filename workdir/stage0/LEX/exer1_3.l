%{
#include <stdio.h>
%}

%option noyywrap

%%

A[a-zA-Z]{2}   { printf("Matched: %s\n", yytext); }
[a-zA-Z]+      ; /* Ignore other words */
[ \t\n]        ; /* Ignore whitespace */
.              ; /* Ignore any other punctuation/characters */

%%

int main() {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error opening file!\n");
        return 1;
    }

    yyin = fp;
    yylex();

    fclose(fp);
    return 0;
}