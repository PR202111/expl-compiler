%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(char const *s);
%}

%union {
    char *str;
}

%token <str> ID
%token NEWLINE

%left '+' '-'
%left '*' '/'

%%
start : expr NEWLINE  { printf("\n"); exit(0); }
      ;

expr  : expr '+' expr { printf("+ "); }
      | expr '-' expr { printf("- "); }
      | expr '*' expr { printf("* "); }
      | expr '/' expr { printf("/ "); }
      | '(' expr ')'
      | ID            { printf("%s ", $1); }
      ;
%%

void yyerror(char const *s) {
    printf("Error: %s\n", s);
}

int main() {
    yyparse();
    return 0;
}