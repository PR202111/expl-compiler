%{
#include <stdio.h>

int lines = 0;
int words = 0;
int chars = 0;

%}

%option noyywrap

%%

\n          { lines++; chars++; }
[a-zA-Z]+   { words++; chars += yyleng; }
.           { chars++; }

%%

int main() {

    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error opening file!\n");
        return 1;
    }
    

    yyin = fp;

    yylex();

    printf("Lines: %d\n", lines);
    printf("Words: %d\n", words);
    printf("Characters: %d\n", chars);
    
    fclose(fp);
    return 0;
}