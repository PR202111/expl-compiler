%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "exprtree.h"
    #include "exprtree.c"


    int yylex(void);
    void yyerror(char const *s);
%}

%union {
    struct tnode *no;
}

%token <no> NUM
%type <no> expr

%left '+' '-'
%left '*' '/'

%%

program : expr '\n' {
            printf("\n--- Tree Created Successfully ---\n");
            printf("Prefix : ");
            prefix($1);
            printf("\nPostfix: ");
            postfix($1);
            printf("\n");
            exit(0);
        }
        ;

expr : expr '+' expr    { $$ = makeOperatorNode('+', $1, $3); }
     | expr '-' expr    { $$ = makeOperatorNode('-', $1, $3); }
     | expr '*' expr    { $$ = makeOperatorNode('*', $1, $3); }
     | expr '/' expr    { $$ = makeOperatorNode('/', $1, $3); }
     | '(' expr ')'     { $$ = $2; }
     | NUM              { $$ = $1; }
     ;

%%

void yyerror(char const *s) {
    printf("yyerror: %s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}