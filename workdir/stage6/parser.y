%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symboltable.h"
#include "typetable.h"

int yylex(void);
void yyerror(char const *s);
extern FILE* yyin;

struct Typetable* DeclType = NULL;
struct Paramstruct* current_param_list = NULL;
struct Fieldlist* current_field_list = NULL;
int field_index = 0;

void addParam(char* name, struct Typetable* type);
void addField(char* name, struct Typetable* type);
void PrintGST();
void PrintLST(const char* scopeName);
void PrintTypeTable();
%}

%union {
    struct tnode *no;
    struct Typetable *type;
    struct Fieldlist *field;
}


%token DECL ENDDECL INT STR MAIN RETURN TYPE ENDTYPE
%token START END READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token ALLOC FREE INITIALIZE NUL
%token <no> NUM ID STRING
%token LT GT LE GE EQ NE

%type <no> E Slist Stmt RetStmt Variable ArgList ArgListInner TypeDefName
%type <type> Type TypeName

%left LT GT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right '&' USTAR    
%left '.'

%%

Program : TypeDefBlock GDeclBlock FDefList MainBlock {
            PrintGST();
            exit(0); 
        }
        ;

TypeDefBlock : TYPE TypeDefList ENDTYPE { PrintTypeTable(); }
             | { TypeTableCreate(); /* Initialize default int, str, bool, void if empty */ }
             ;

TypeDefList  : TypeDefList TypeDef | TypeDef ;

TypeDefName  : ID {
                 TInstall($1->varname, 0, NULL);
                 $$ = $1;
             }
             ;

TypeDef      : TypeDefName '{' FieldDeclList '}' {
                 struct Typetable *type = TLookup($1->varname);
                 type->fields = current_field_list;
                 type->size = field_index;
                 
                 if (field_index > 8) {
                     printf("Error: User defined type %s has more than 8 fields.\n", type->name);
                     exit(1);
                 }
                 
                 current_field_list = NULL;
                 field_index = 0;
             }
             ;

FieldDeclList: FieldDeclList FieldDecl
             | FieldDecl
             ;

FieldDecl    : TypeName ID ';' { addField($2->varname, $1); }
             ;

GDeclBlock : DECL GDeclList ENDDECL 
           | DECL ENDDECL 
           | 
           ;

GDeclList  : GDeclList GDecl | GDecl ;
GDecl      : Type GIdList ';' ;

TypeName   : INT { $$ = TLookup("int"); } 
           | STR { $$ = TLookup("str"); } 
           | ID  { $$ = TLookup($1->varname); 
                   if ($$ == NULL) { printf("Error: Undefined type %s\n", $1->varname); exit(1); } 
                 }
           ;

/* Set global DeclType safely without overwriting it during params */
Type       : TypeName { $$ = $1; DeclType = $1; } ;

GIdList    : GIdList ',' GId | GId ;

GId : ID { GInstall($1->varname, DeclType, 1, -1, NULL); }
    | '*' ID { GInstall($2->varname, DeclType, 1, -1, NULL); }
    | ID '[' NUM ']' { GInstall($1->varname, DeclType, $3->val, -1, NULL); }
    | ID '[' NUM ']' '[' NUM ']' { GInstall($1->varname, DeclType, $3->val * $6->val, $6->val, NULL); }
    | ID '(' ParamList ')' { GInstall($1->varname, DeclType, -1, -1, current_param_list); current_param_list = NULL; }
    ;

ParamList : ParamListInner 
          | /* empty */ { current_param_list = NULL; } 
          ;

ParamListInner : ParamListInner ',' Param 
               | Param 
               ;

Param : TypeName ID { addParam($2->varname, $1); }
      | TypeName '*' ID { addParam($3->varname, $1); }
      ;

LDeclBlock : DECL LDecList ENDDECL 
           | DECL ENDDECL 
           | 
           ;

LDecList   : LDecList LDecl | LDecl ;
LDecl      : Type IdList ';' ;

IdList     : IdList ',' ID { LInstall($3->varname, DeclType, 0); }
           | IdList ',' '*' ID { LInstall($4->varname, DeclType, 0); }
           | ID { LInstall($1->varname, DeclType, 0); }
           | '*' ID { LInstall($2->varname, DeclType, 0); }
           ;

FDefList : FDefList FDef 
         | /* empty */ 
         ;

FDef : Type ID '(' ParamList ')' '{' {
        VerifyFunctionSignature($2->varname, $1, current_param_list);
        
        struct Paramstruct* temp = current_param_list;
        while(temp != NULL) {
            LInstall(temp->name, temp->type, 1);
            temp = temp->next;
        }
    } LDeclBlock START Slist RetStmt END '}' {
        
        struct tnode* body = createTree(0, NULL, NULL, NODE_CONN, NULL, $10, NULL, $11);
        FILE* out = fopen("output.xsm", "a");
        generateFuncCode($2->varname, body, out);
        fclose(out);

        PrintLST($2->varname);
        ClearLST();
        current_param_list = NULL;
    }
    ;

MainBlock : INT MAIN '(' ')' '{' {
        GInstall("main", TLookup("int"), -1, -1, NULL); 
    } LDeclBlock START Slist RetStmt END '}' {
        
        struct tnode* body = createTree(0, NULL, NULL, NODE_CONN, NULL, $9, NULL, $10);
        
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

RetStmt : RETURN E ';' { $$ = createTree(0, NULL, NULL, NODE_RET, NULL, $2, NULL, NULL); } ;

Slist   : Slist Stmt { $$ = createTree(0, NULL, NULL, NODE_CONN, NULL, $1, NULL, $2); }
        | Stmt { $$ = $1; }
        ;

Stmt    : Variable '=' E ';' { $$ = createTree(0, NULL, NULL, NODE_ASSIGN, "=", $1, NULL, $3); }
        | READ '(' Variable ')' ';' { $$ = createTree(0, NULL, NULL, NODE_READ, NULL, $3, NULL, NULL); }
        | WRITE '(' E ')' ';' { $$ = createTree(0, NULL, NULL, NODE_WRITE, NULL, $3, NULL, NULL); }
        | IF '(' E ')' THEN Slist ELSE Slist ENDIF ';' { $$ = createTree(0, NULL, NULL, NODE_IF, NULL, $3, $6, $8); }
        | IF '(' E ')' THEN Slist ENDIF ';' { $$ = createTree(0, NULL, NULL, NODE_IF, NULL, $3, $6, NULL); }
        | WHILE '(' E ')' DO Slist ENDWHILE ';' { $$ = createTree(0, NULL, NULL, NODE_WHILE, NULL, $3, $6, NULL); }
        | BREAK ';' { $$ = createTree(0, NULL, NULL, NODE_BREAK, NULL, NULL, NULL, NULL); }
        | CONTINUE ';' { $$ = createTree(0, NULL, NULL, NODE_CONTINUE, NULL, NULL, NULL, NULL); }
        ;

E       : E '+' E { $$ = createTree(0, TLookup("int"), NULL, NODE_OP, "+", $1, NULL, $3); }
        | E '-' E { $$ = createTree(0, TLookup("int"), NULL, NODE_OP, "-", $1, NULL, $3); }
        | E '*' E { $$ = createTree(0, TLookup("int"), NULL, NODE_OP, "*", $1, NULL, $3); }
        | E '/' E { $$ = createTree(0, TLookup("int"), NULL, NODE_OP, "/", $1, NULL, $3); }
        | E '%' E { $$ = createTree(0, TLookup("int"), NULL, NODE_OP, "%", $1, NULL, $3); }
        
        | E LT E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_LT, "<", $1, NULL, $3); }
        | E GT E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_GT, ">", $1, NULL, $3); }
        | E LE E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_LE, "<=", $1, NULL, $3); }
        | E GE E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_GE, ">=", $1, NULL, $3); }
        | E EQ E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_EQ, "==", $1, NULL, $3); }
        | E NE E  { $$ = createTree(0, TLookup("bool"), NULL, NODE_NE, "!=", $1, NULL, $3); }
        
        | '&' Variable { 
            $$ = createTree(0, $2->type, NULL, NODE_AMPER, NULL, $2, NULL, NULL); 
        }
        | '(' E ')' { $$ = $2; }
        | NUM       { $$ = $1; $$->type = TLookup("int"); }
        | STRING    { $$ = $1; $$->type = TLookup("str"); }
        | Variable  { $$ = $1; }
        
        | ID '(' ArgList ')' {
            struct Gsymbol* g = GLookUp($1->varname);
            if(g == NULL) { printf("Error: Undefined function '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, g->type, $1->varname, NODE_FUNCALL, NULL, NULL, NULL, NULL);
            $$->Gentry = g;
            $$->arglist = $3;
        }
        
        | INITIALIZE '(' ')' { $$ = createTree(0, TLookup("int"), NULL, NODE_INIT, NULL, NULL, NULL, NULL); }
        | ALLOC '(' ')'      { $$ = createTree(0, TLookup("void"), NULL, NODE_ALLOC, NULL, NULL, NULL, NULL); }
        | FREE '(' Variable ')' { $$ = createTree(0, TLookup("int"), NULL, NODE_FREE, NULL, $3, NULL, NULL); }
        | NUL                { $$ = createTree(0, TLookup("void"), NULL, NODE_NULL, NULL, NULL, NULL, NULL); }
        ;

ArgList : ArgListInner 
        | { $$ = NULL; } 
        ;

ArgListInner : ArgListInner ',' E { 
                struct tnode* temp = $1;
                while(temp->mid != NULL) temp = temp->mid;
                temp->mid = $3;
                $$ = $1;
             }
             | E { $$ = $1; }
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
            $$ = createTree(0, $2->type, NULL, NODE_DEREF, NULL, $2, NULL, NULL);
         }
         | Variable '.' ID %prec '.' {
            struct Fieldlist *f = FLookup($1->type, $3->varname);
            if (f == NULL) { printf("Error: Field '%s' not found in type '%s'\n", $3->varname, $1->type->name); exit(1); }
            $$ = createTree(0, f->type, $3->varname, NODE_FIELD, NULL, $1, NULL, NULL);
         }
         ;

%%

void yyerror(char const *s) {
    printf("yyerror: %s\n", s);
}

void addParam(char* name, struct Typetable* type) {
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

void addField(char* name, struct Typetable* type) {
    struct Fieldlist* field = (struct Fieldlist*)malloc(sizeof(struct Fieldlist));
    field->name = strdup(name);
    field->fieldIndex = field_index++;
    field->type = type;
    field->next = NULL;

    if (current_field_list == NULL) {
        current_field_list = field;
    } else {
        struct Fieldlist* temp = current_field_list;
        while(temp->next != NULL) temp = temp->next;
        temp->next = field;
    }
}

int main(int argc, char* argv[]) {
    TypeTableCreate(); // Initializes int, str, bool, and void (for NULL)
    
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    
    FILE *out = fopen("output.xsm", "w");
    fclose(out);

    yyparse();
    return 0;
}