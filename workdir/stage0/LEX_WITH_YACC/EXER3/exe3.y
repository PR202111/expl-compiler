%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex();
void yyerror(char const *s);


char* make_prefix(char op, char* left, char* right) {
    
    char* res = malloc(strlen(left) + strlen(right) + 4);
    sprintf(res, "%c %s %s", op, left, right);
    free(left);
    free(right);
    return res;
}
%}

%union {
    char *str;
}

%token <str> ID
%type <str> expr   
%token NEWLINE

%left '+' '-'
%left '*' '/'

%%
start : expr NEWLINE  { printf("%s\n", $1); free($1); exit(0); }
      ;

expr  : expr '+' expr { $$ = make_prefix('+', $1, $3); }
      | expr '-' expr { $$ = make_prefix('-', $1, $3); }
      | expr '*' expr { $$ = make_prefix('*', $1, $3); }
      | expr '/' expr { $$ = make_prefix('/', $1, $3); }
      | '(' expr ')'  { $$ = $2; }
      | ID            { $$ = $1; }
      ;
%%

void yyerror(char const *s) {
    printf("Error: %s\n", s);
}

int main() {
    yyparse();
    return 0;
}