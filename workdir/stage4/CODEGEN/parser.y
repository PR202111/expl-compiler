%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symboltable.h"

int yylex(void);
void yyerror(char const *s);
int codeGen(struct tnode *t, FILE *target_file);
void PrintSymbolTable();

int DeclType = -1;

%}

%union {
    struct tnode *no;
}

%token DECL ENDDECL INT STR
%token START END READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token <no> NUM ID STRING
%token LT GT LE GE EQ NE

%type <no> E Slist Stmt Program Variable

%left LT GT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right '&' USTAR    

%%

Program : Declarations START Slist END ';' {
            PrintSymbolTable();
            $$ = $3;
            FILE *out = fopen("output.xsm", "w");
            
            
            fprintf(out, "0\n2056\n0\n0\n0\n0\n0\n0\n"); 
            fprintf(out, "BRKP\n");
            
            extern int memory_binding;
            fprintf(out, "MOV SP, %d\n", memory_binding); 
            
            codeGen($$, out);
            
            // Example of how your termination code should look:
            int temp = getReg();
            fprintf(out, "MOV R%d, \"Exit\"\n", temp);
            fprintf(out, "PUSH R%d\n", temp);
            fprintf(out, "PUSH R%d\n", temp);
            fprintf(out, "PUSH R%d\n", temp);
            fprintf(out, "PUSH R%d\n", temp);
            fprintf(out, "PUSH R%d\n", temp);
            fprintf(out, "INT 10\n"); // Inline interrupt for Exit
            freeReg();
            
            fclose(out);
            exit(0);
        }
        | START END ';' { exit(0); }
        ;

Declarations : DECL DeclList ENDDECL | DECL ENDDECL ;
DeclList : DeclList Decl | Decl ;
Decl : Type VarList ';' ;

Type : INT { DeclType = inttype; } 
     | STR { DeclType = strtype; } 
     ;

VarList : VarList ',' Var | Var ;

Var : ID { 
        Install($1->varname, DeclType, 1, -1); 
      }
    | '*' ID { 
        // If DeclType is 1 (int), int* is 3. If 2 (str), str* is 4.
        Install($2->varname, DeclType + 2, 1, -1); 
      }
    | ID '[' NUM ']' { 
        Install($1->varname, DeclType, $3->val, -1); 
      }
    | ID '[' NUM ']' '[' NUM ']' { 
        // Size = rows * cols. Col size is stored to help calculate offset later.
        Install($1->varname, DeclType, $3->val * $6->val, $6->val); 
      }
    ;

Variable : ID { 
            $1->Gentry = LookUp($1->varname);
            if($1->Gentry == NULL) { printf("Error: Undeclared Variable '%s'\n", $1->varname); exit(1); }
            $1->type = $1->Gentry->type;
            $$ = $1;
         }
         | ID '[' E ']' { 
            struct Gsymbol *entry = LookUp($1->varname);
            if(entry == NULL) { printf("Error: Undeclared Array '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, entry->type, $1->varname, NODE_ARRAY, NULL, $3, NULL, NULL);
            $$->Gentry = entry;
         }
         | ID '[' E ']' '[' E ']' { 
            struct Gsymbol *entry = LookUp($1->varname);
            if(entry == NULL) { printf("Error: Undeclared 2D Array '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, entry->type, $1->varname, NODE_ARRAY2D, NULL, $3, $6, NULL);
            $$->Gentry = entry;
         }
         | '*' E %prec USTAR {
            int base_type = -1;
            if ($2->type == intptrtype) base_type = inttype;
            else if ($2->type == strptrtype) base_type = strtype;
            else { printf("Type Error: Cannot dereference a non-pointer.\n"); exit(1); }
            $$ = createTree(0, base_type, NULL, NODE_DEREF, NULL, $2, NULL, NULL);
         }
         ;

Slist   : Slist Stmt { $$ = createTree(0, -1, NULL, NODE_CONN, NULL, $1, NULL, $2); }
        | Stmt { $$ = $1; }
        ;

Stmt    : Variable '=' E ';' { $$ = createTree(0, -1, NULL, NODE_ASSIGN, "=", $1, NULL, $3); }
        | READ '(' Variable ')' ';' { $$ = createTree(0, -1, NULL, NODE_READ, NULL, $3, NULL, NULL); }
        | WRITE '(' E ')' ';' { $$ = createTree(0, -1, NULL, NODE_WRITE, NULL, $3, NULL, NULL); }
        | IF '(' E ')' THEN Slist ELSE Slist ENDIF ';' { $$ = createTree(0, -1, NULL, NODE_IF, NULL, $3, $6, $8); }
        | IF '(' E ')' THEN Slist ENDIF ';' { $$ = createTree(0, -1, NULL, NODE_IF, NULL, $3, $6, NULL); }
        | WHILE '(' E ')' DO Slist ENDWHILE ';' { $$ = createTree(0, -1, NULL, NODE_WHILE, NULL, $3, $6, NULL); }
        | BREAK ';' { $$ = createTree(0, -1, NULL, NODE_BREAK, NULL, NULL, NULL, NULL); }
        | CONTINUE ';' { $$ = createTree(0, -1, NULL, NODE_CONTINUE, NULL, NULL, NULL, NULL); }
        ;

E       : E '+' E { $$ = createTree(0, inttype, NULL, NODE_OP, "+", $1, NULL, $3); }
        | E '-' E { $$ = createTree(0, inttype, NULL, NODE_OP, "-", $1, NULL, $3); }
        | E '*' E { $$ = createTree(0, inttype, NULL, NODE_OP, "*", $1, NULL, $3); }
        | E '/' E { $$ = createTree(0, inttype, NULL, NODE_OP, "/", $1, NULL, $3); }
        | E '%' E { $$ = createTree(0, inttype, NULL, NODE_OP, "%", $1, NULL, $3); }
        
        | E LT E  { $$ = createTree(0, booltype, NULL, NODE_LT, "<", $1, NULL, $3); }
        | E GT E  { $$ = createTree(0, booltype, NULL, NODE_GT, ">", $1, NULL, $3); }
        | E LE E  { $$ = createTree(0, booltype, NULL, NODE_LE, "<=", $1, NULL, $3); }
        | E GE E  { $$ = createTree(0, booltype, NULL, NODE_GE, ">=", $1, NULL, $3); }
        | E EQ E  { $$ = createTree(0, booltype, NULL, NODE_EQ, "==", $1, NULL, $3); }
        | E NE E  { $$ = createTree(0, booltype, NULL, NODE_NE, "!=", $1, NULL, $3); }
        
        | '&' Variable { 
            int ptr_type = -1;
            if ($2->type == inttype) ptr_type = intptrtype;
            else if ($2->type == strtype) ptr_type = strptrtype;
            $$ = createTree(0, ptr_type, NULL, NODE_AMPER, NULL, $2, NULL, NULL); 
        }
        
        | '(' E ')' { $$ = $2; }
        | NUM       { $$ = $1; }
        | STRING    { $$ = $1; }
        | Variable  { $$ = $1; }
        ;

%%

void yyerror(char const *s) {
    printf("yyerror: %s\n", s);
}

extern FILE* yyin; 

int main(int argc, char* argv[]) {
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    yyparse();
    return 0;
}