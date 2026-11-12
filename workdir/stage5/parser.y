%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symboltable.h"

int yylex(void);
void yyerror(char const *s);
extern FILE* yyin;

int DeclType = -1;
struct Paramstruct* current_param_list = NULL;

void addParam(char* name, int type);
void PrintGST();
void PrintLST(const char* scopeName);

%}

%union {
    struct tnode *no;
}

%token DECL ENDDECL INT STR MAIN RETURN
%token START END READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token <no> NUM ID STRING
%token LT GT LE GE EQ NE

%type <no> E Slist Stmt RetStmt Variable ArgList

%left LT GT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right '&' USTAR    

%%

Program : GDeclBlock FDefBlock MainBlock {
            PrintGST();
            exit(0); 
        }
        | GDeclBlock MainBlock { 
          PrintGST();
          exit(0); 
        }
        ;

GDeclBlock : DECL GDeclList ENDDECL 
           | DECL ENDDECL 
           | 
           ;

GDeclList  : GDeclList GDecl | GDecl ;
GDecl      : Type GIdList ';' ;

Type       : INT { DeclType = inttype; } 
           | STR { DeclType = strtype; } 
           ;

GIdList    : GIdList ',' GId | GId ;

GId : ID { GInstall($1->varname, DeclType, 1, -1, NULL); }
    | '*' ID { GInstall($2->varname, DeclType + 2, 1, -1, NULL); }
    | ID '[' NUM ']' { GInstall($1->varname, DeclType, $3->val, -1, NULL); }
    | ID '[' NUM ']' '[' NUM ']' { GInstall($1->varname, DeclType, $3->val * $6->val, $6->val, NULL); }
    | ID '(' ParamList ')' { GInstall($1->varname, DeclType, -1, -1, current_param_list); current_param_list = NULL; }
    ;

ParamList : ParamList ',' Param
          | Param
          | { current_param_list = NULL; }
          ;

Param : Type ID { addParam($2->varname, DeclType); }
      | Type '*' ID { addParam($3->varname, DeclType + 2); }
      ;

LDeclBlock : DECL LDecList ENDDECL 
           | DECL ENDDECL 
           | 
           ;

LDecList   : LDecList LDecl | LDecl ;
LDecl      : Type IdList ';' ;

IdList     : IdList ',' ID { LInstall($3->varname, DeclType, 0); }
           | IdList ',' '*' ID { LInstall($4->varname, DeclType + 2, 0); }
           | ID { LInstall($1->varname, DeclType, 0); }
           | '*' ID { LInstall($2->varname, DeclType + 2, 0); }
           ;

FDefBlock : FDefBlock FDef | FDef ;

FDef : Type ID '(' ParamList ')' '{' {
        VerifyFunctionSignature($2->varname, DeclType, current_param_list);
        
        struct Paramstruct* temp = current_param_list;
        while(temp != NULL) {
            LInstall(temp->name, temp->type, 1);
            temp = temp->next;
        }
    } LDeclBlock START Slist RetStmt END '}' {
        
        struct tnode* body = createTree(0, -1, NULL, NODE_CONN, NULL, $10, NULL, $11);
        
        FILE* out = fopen("output.xsm", "a");
        generateFuncCode($2->varname, body, out);
        fclose(out);

        PrintLST($2->varname);
        ClearLST();
        current_param_list = NULL;
    }
    ;

MainBlock : INT MAIN '(' ')' '{' {
        GInstall("main", inttype, -1, -1, NULL); 
    } LDeclBlock START Slist RetStmt END '}' {
        
        struct tnode* body = createTree(0, -1, NULL, NODE_CONN, NULL, $9, NULL, $10);
        
        FILE* out = fopen("output.xsm", "a"); 
        generateFuncCode("main", body, out);
        fclose(out);

        PrintLST("main");
        ClearLST();

        FILE* header = fopen("header.xsm", "w");
        fprintf(header, "0\n2056\n0\n0\n0\n0\n0\n0\n"); 
        extern int memory_binding;
        fprintf(header, "MOV SP, %d\n", memory_binding);
        
        fprintf(header, "MOV R0, 0\n");
        fprintf(header, "PUSH R0\n");
        
        struct Gsymbol* main_entry = GLookUp("main");
        fprintf(header, "CALL F%d\n", main_entry->flabel);
        
        fprintf(header, "MOV R0, \"Exit\"\n");
        fprintf(header, "PUSH R0\n");
        fprintf(header, "PUSH R0\n");
        fprintf(header, "PUSH R0\n");
        fprintf(header, "PUSH R0\n");
        fprintf(header, "PUSH R0\n");
        fprintf(header, "CALL 0\n");
        fclose(header);

        system("cat header.xsm output.xsm > temp.xsm && mv temp.xsm output.xsm && rm header.xsm");
    }
    ;

RetStmt : RETURN E ';' { $$ = createTree(0, -1, NULL, NODE_RET, NULL, $2, NULL, NULL); } ;

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
        | ID '(' ArgList ')' ';' 
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
        
        | ID '(' ArgList ')' {
            struct Gsymbol* g = GLookUp($1->varname);
            if(g == NULL) { printf("Error: Undefined function '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, g->type, $1->varname, NODE_FUNCALL, NULL, NULL, NULL, NULL);
            $$->Gentry = g;
            $$->arglist = $3;
        }
        ;

ArgList : ArgList ',' E { 
            struct tnode* temp = $1;
            while(temp->mid != NULL) temp = temp->mid;
            temp->mid = $3;
            $$ = $1;
        }
        | E { $$ = $1; }
        | { $$ = NULL; }
        ;

Variable : ID {
            struct Lsymbol* local = LLookUp($1->varname);
            if (local) {
                $$ = createTree(0, local->type, $1->varname, NODE_ID, NULL, NULL, NULL, NULL);
                $$->Lentry = local;
                $$->Gentry = NULL;
            } else {
                struct Gsymbol* global = GLookUp($1->varname);
                if (!global) { printf("Error: Undeclared Variable '%s'\n", $1->varname); exit(1); }
                $$ = createTree(0, global->type, $1->varname, NODE_ID, NULL, NULL, NULL, NULL);
                $$->Gentry = global;
                $$->Lentry = NULL;
            }
         }
         | ID '[' E ']' { 
            struct Gsymbol *entry = GLookUp($1->varname);
            if(entry == NULL) { printf("Error: Undeclared Array '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, entry->type, $1->varname, NODE_ARRAY, NULL, $3, NULL, NULL);
            $$->Gentry = entry;
         }
         | ID '[' E ']' '[' E ']' { 
            struct Gsymbol *entry = GLookUp($1->varname);
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

%%

void yyerror(char const *s) {
    printf("yyerror: %s\n", s);
}

void addParam(char* name, int type) {
    struct Paramstruct* param = (struct Paramstruct*)malloc(sizeof(struct Paramstruct));
    param->name = strdup(name);
    param->type = type;
    param->next = NULL;
    
    if (current_param_list == NULL) {
        current_param_list = param;
    } else {
        struct Paramstruct* temp = current_param_list;
        while(temp->next != NULL) temp = temp->next;
        temp->next = param;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    
    FILE *out = fopen("output.xsm", "w");
    fclose(out);

    yyparse();
    return 0;
}