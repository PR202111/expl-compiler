%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);
void yyerror(char const *s);

int evaluate(struct tnode *t); 
%}

%union {
    struct tnode *no;
}

%token START END READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token <no> NUM ID
%token LT GT LE GE EQ NE

%type <no> E Slist Stmt Program


%left LT GT LE GE EQ NE
%left '+' '-'
%left '*' '/'

%%

Program : START Slist END ';' {
            $$ = $2;
            printf("Parsing successful. Evaluating AST:\n");
            evaluate($$);
            exit(0);
        }
        | START END ';' {
            exit(0);
        }
        ;

Slist   : Slist Stmt {
            // NODE_CONN connects statements sequentially
            $$ = createTree(0, -1, NULL, NODE_CONN, NULL, $1, NULL, $2);
        }
        | Stmt {
            $$ = $1;
        }
        ;

Stmt    : ID '=' E ';' {
            $$ = createTree(0, -1, NULL, NODE_ASSIGN, "=", $1, NULL, $3);
        }
        | READ '(' ID ')' ';' {
            $$ = createTree(0, -1, NULL, NODE_READ, NULL, $3, NULL, NULL);
        }
        | WRITE '(' E ')' ';' {
            $$ = createTree(0, -1, NULL, NODE_WRITE, NULL, $3, NULL, NULL);
        }
        | IF '(' E ')' THEN Slist ELSE Slist ENDIF ';' {
            // Left = condition, Mid = Then-block, Right = Else-block
            $$ = createTree(0, -1, NULL, NODE_IF, NULL, $3, $6, $8);
        }
        | IF '(' E ')' THEN Slist ENDIF ';' {
            // Left = condition, Mid = Then-block, Right = NULL (No else)
            $$ = createTree(0, -1, NULL, NODE_IF, NULL, $3, $6, NULL);
        }
        | WHILE '(' E ')' DO Slist ENDWHILE ';' {
            // Left = condition, Mid = Loop-block, Right = NULL
            $$ = createTree(0, -1, NULL, NODE_WHILE, NULL, $3, $6, NULL);
        }
        

E       : E '+' E { $$ = createTree(0, inttype, NULL, NODE_OP, "+", $1, NULL, $3); }
        | E '-' E { $$ = createTree(0, inttype, NULL, NODE_OP, "-", $1, NULL, $3); }
        | E '*' E { $$ = createTree(0, inttype, NULL, NODE_OP, "*", $1, NULL, $3); }
        | E '/' E { $$ = createTree(0, inttype, NULL, NODE_OP, "/", $1, NULL, $3); }
        
        | E LT E  { $$ = createTree(0, booltype, NULL, NODE_LT, "<", $1, NULL, $3); }
        | E GT E  { $$ = createTree(0, booltype, NULL, NODE_GT, ">", $1, NULL, $3); }
        | E LE E  { $$ = createTree(0, booltype, NULL, NODE_LE, "<=", $1, NULL, $3); }
        | E GE E  { $$ = createTree(0, booltype, NULL, NODE_GE, ">=", $1, NULL, $3); }
        | E EQ E  { $$ = createTree(0, booltype, NULL, NODE_EQ, "==", $1, NULL, $3); }
        | E NE E  { $$ = createTree(0, booltype, NULL, NODE_NE, "!=", $1, NULL, $3); }
        
        | '(' E ')' { $$ = $2; }
        | NUM       { $$ = $1; }
        | ID        { $$ = $1; }
        ;

%%

void yyerror(char const *s) {
    printf("yyerror: %s\n", s);
}

extern FILE* yyin; 

int main(int argc, char* argv[]) {
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "r");
        if (fp) {
            yyin = fp;
        } else {
            printf("Error: Could not open file %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("Required a <filename> as agrument\n");
        return 1;
    }

    yyparse();
    return 0;
}