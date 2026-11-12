%{
#include <stdio.h>

int int_count = 0;
int float_count = 0;

%}

%option noyywrap

%%

[0-9]+\.[0-9]+ {float_count++;printf("Found Float: %s\n",yytext);}
[0-9]+ {int_count++; printf("Found Integer: %s\n",yytext);}


%%

int main() {

    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error opening file!\n");
        return 1;
    }
    

    yyin = fp;

    yylex();

    printf("INTEGERS: %d\n", int_count);
    printf("FLOATING NUMBERS: %d\n", float_count);
    
    fclose(fp);
    return 0;
}