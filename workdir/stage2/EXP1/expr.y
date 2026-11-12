%{
#include <stdio.h>
#include <stdlib.h>
#include "exprtree.h"

int yylex(void);
void yyerror(char const *s);


void evaluate(struct tnode* t); 
%}

%union {
    int int_val;
    char* str_val;
    struct tnode* n;
}


%token BEGIN_TOK END_TOK READ WRITE 
%token PLUS MINUS MUL DIV ASSIGN SEMICOLON
%token <int_val> NUM
%token <str_val> ID


%type <n> Program Slist Stmt InputStmt OutputStmt AsgStmt E


%left PLUS MINUS
%left MUL DIV

%%

Program : BEGIN_TOK Slist END_TOK {
            $$ = $2;
            printf("AST Built Successfully!\n");
            evaluate($$); // Call our evaluator on the root node
            exit(0);
        }
        | BEGIN_TOK END_TOK {
            printf("Empty Program\n");
            exit(0);
        }
        ;

Slist   : Slist Stmt {
            // A connector node joins two statements together
            $$ = createTree(0, 0, NULL, NODE_CONN, NULL, $1, $2);
        }
        | Stmt {
            $$ = $1;
        }
        ;

Stmt    : InputStmt  { $$ = $1; }
        | OutputStmt { $$ = $1; }
        | AsgStmt    { $$ = $1; }
        ;

InputStmt : READ '(' ID ')' SEMICOLON {
            struct tnode* id_node = createTree(0, 0, $3, NODE_ID, NULL, NULL, NULL);
            $$ = createTree(0, 0, NULL, NODE_READ, NULL, id_node, NULL);
        }
        ;

OutputStmt : WRITE '(' E ')' SEMICOLON {
            $$ = createTree(0, 0, NULL, NODE_WRITE, NULL, $3, NULL);
        }
        ;

AsgStmt : ID ASSIGN E SEMICOLON {
            struct tnode* id_node = createTree(0, 0, $1, NODE_ID, NULL, NULL, NULL);
            $$ = createTree(0, 0, NULL, NODE_ASSIGN, "=", id_node, $3);
        }
        ;

E       : E PLUS E  { $$ = createTree(0, 0, NULL, NODE_OP, "+", $1, $3); }
        | E MINUS E { $$ = createTree(0, 0, NULL, NODE_OP, "-", $1, $3); }
        | E MUL E   { $$ = createTree(0, 0, NULL, NODE_OP, "*", $1, $3); }
        | E DIV E   { $$ = createTree(0, 0, NULL, NODE_OP, "/", $1, $3); }
        | '(' E ')' { $$ = $2; }
        | NUM       { $$ = createTree($1, 0, NULL, NODE_NUM, NULL, NULL, NULL); }
        | ID        { $$ = createTree(0, 0, $1, NODE_ID, NULL, NULL, NULL); }
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