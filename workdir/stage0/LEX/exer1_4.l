%{
#include <stdio.h>
%}

%option noyywrap

%%


\/\/[^\n]* {printf("Single-line comment: %s\n", yytext);}

\/\*[^*]*\*+([^/*][^*]*\*+)*\/ {
    printf("Multi-line comment:%s\n", yytext);
}


.   ;
\n  ;

%%

int main() {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error opening input.txt\n");
        return 1;
    }

    yyin = fp;
    yylex();

    fclose(fp);
    return 0;
}