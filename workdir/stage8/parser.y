%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symboltable.h"
#include "classtable.h"
#include "typetable.h"

int yylex(void);
void yyerror(char const *s);
extern FILE* yyin;

struct Typetable* DeclType = NULL;
struct Classtable* DeclClass = NULL;
struct Paramstruct* current_param_list = NULL;

struct Fieldlist* current_field_list = NULL;
int field_index = 0;

struct Classtable* Cptr = NULL;        
struct Classtable* CurrentClass = NULL;

extern int class_index_counter;
extern int memory_binding;
extern int current_method_param_count;

void addParam(char* name, char* type_name);
void addField(char* name, char* type_name);
void PrintGST();
void PrintLST(const char* scopeName);
void PrintTypeTable();
%}

%union {
    struct tnode *no;
}

%token CLASS ENDCLASS EXTENDS NEW DELETE SELF
%token DECL ENDDECL INT STR MAIN RETURN TYPE ENDTYPE
%token START END READ WRITE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token ALLOC FREE INITIALIZE NUL
%token <no> NUM ID STRING
%token LT GT LE GE EQ NE

%type <no> E Statements Slist Stmt RetStmt Variable ArgList ArgListInner

%left LT GT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right '&' USTAR    
%left '.'

%%

Program : TypeDefBlock ClassDefBlock GDeclBlock FDefList MainBlock {
            exit(0); 
        }
        ;

/* ==================== TYPE DEFINITIONS ==================== */
TypeDefBlock  : TYPE TypeDefList ENDTYPE { PrintTypeTable(); }
              | { }
              ;

TypeDefList   : TypeDefList TypeDef | TypeDef ;

TypeDef       : ID { TInstall($1->varname, 0, NULL); } '{' TFieldDeclList '}' {
                  struct Typetable *type = TLookup($1->varname);
                  type->fields = current_field_list;
                  type->size = field_index;
                  if (field_index > 8) { printf("Error: UDT size > 8.\n"); exit(1); }
                  current_field_list = NULL; field_index = 0;
              }
              ;

TFieldDeclList: TFieldDeclList TFieldDecl | TFieldDecl ;
TFieldDecl    : INT ID ';' { addField($2->varname, "int"); } 
              | STR ID ';' { addField($2->varname, "str"); } 
              | ID ID ';'  { addField($2->varname, $1->varname); } 
              ;

/* ==================== CLASS DEFINITIONS ==================== */
ClassDefBlock : CLASS ClassDefList ENDCLASS | /* empty */ ;
ClassDefList  : ClassDefList ClassDef | ClassDef ;

ClassDef      : Cname '{' DECL CDeclList ENDDECL MethodDefList '}' ;

Cname         : ID { Cptr = CInstall($1->varname, NULL); } 
              | ID EXTENDS ID { Cptr = CInstall($1->varname, $3->varname); } // STAGE 8 ENABLED
              ;

CDeclList     : CDeclList CDecl | /* empty */ ;

CDecl         : INT ID ';' { Class_Finstall(Cptr, "int", $2->varname); } 
              | STR ID ';' { Class_Finstall(Cptr, "str", $2->varname); } 
              | ID ID ';'  { Class_Finstall(Cptr, $1->varname, $2->varname); } 
              | INT ID '(' ParamList ')' ';' { Class_Minstall(Cptr, $2->varname, TLookup("int"), current_param_list); current_param_list = NULL; } 
              | STR ID '(' ParamList ')' ';' { Class_Minstall(Cptr, $2->varname, TLookup("str"), current_param_list); current_param_list = NULL; } 
              | ID ID '(' ParamList ')' ';'  { 
                  struct Typetable* rt = TLookup($1->varname);
                  if(!rt) { printf("Error: Invalid return type %s\n", $1->varname); exit(1); }
                  Class_Minstall(Cptr, $2->varname, rt, current_param_list); 
                  current_param_list = NULL; 
              } 
              ;

MethodDefList : MethodDefList MethodDef | MethodDef ;

MethodDef     : INT ID '(' ParamList ')' '{' {
                    CurrentClass = Cptr;
                    struct Memberfunclist* m = Class_Mlookup(Cptr, $2->varname);
                    if(m == NULL) { printf("Error: Method %s not declared.\n", $2->varname); exit(1); }
                    current_method_param_count = 0;
                    struct Paramstruct* temp = m->paramlist;
                    while(temp != NULL) {
                        LInstall(temp->name, temp->type, 1);
                        current_method_param_count++;
                        temp = temp->next;
                    }
                } LDeclBlock START Statements RetStmt END '}' {
                    struct tnode* body = createTree(0, NULL, CurrentClass, NULL, NODE_CONN, NULL, $10, NULL, $11);
                    FILE* out = fopen("output.xsm", "a");
                    generateFuncCode($2->varname, body, out);
                    fclose(out);
                    ClearLST(); current_param_list = NULL; CurrentClass = NULL;
                }
              | STR ID '(' ParamList ')' '{' {
                    CurrentClass = Cptr;
                    struct Memberfunclist* m = Class_Mlookup(Cptr, $2->varname);
                    if(m == NULL) { printf("Error: Method %s not declared.\n", $2->varname); exit(1); }
                    current_method_param_count = 0;
                    struct Paramstruct* temp = m->paramlist;
                    while(temp != NULL) {
                        LInstall(temp->name, temp->type, 1);
                        current_method_param_count++;
                        temp = temp->next;
                    }
                } LDeclBlock START Statements RetStmt END '}' {
                    struct tnode* body = createTree(0, NULL, CurrentClass, NULL, NODE_CONN, NULL, $10, NULL, $11);
                    FILE* out = fopen("output.xsm", "a");
                    generateFuncCode($2->varname, body, out);
                    fclose(out);
                    ClearLST(); current_param_list = NULL; CurrentClass = NULL;
                }
              | ID ID '(' ParamList ')' '{' {
                    CurrentClass = Cptr;
                    struct Memberfunclist* m = Class_Mlookup(Cptr, $2->varname);
                    if(m == NULL) { printf("Error: Method %s not declared.\n", $2->varname); exit(1); }
                    current_method_param_count = 0;
                    struct Paramstruct* temp = m->paramlist;
                    while(temp != NULL) {
                        LInstall(temp->name, temp->type, 1);
                        current_method_param_count++;
                        temp = temp->next;
                    }
                } LDeclBlock START Statements RetStmt END '}' {
                    struct tnode* body = createTree(0, NULL, CurrentClass, NULL, NODE_CONN, NULL, $10, NULL, $11);
                    FILE* out = fopen("output.xsm", "a");
                    generateFuncCode($2->varname, body, out);
                    fclose(out);
                    ClearLST(); current_param_list = NULL; CurrentClass = NULL;
                }
              ;

/* ==================== GLOBAL DECLARATIONS ==================== */
GDeclBlock : DECL { memory_binding = 4096 + (class_index_counter * 8); } GDeclList ENDDECL 
           | DECL { memory_binding = 4096 + (class_index_counter * 8); } ENDDECL           
           |      { memory_binding = 4096 + (class_index_counter * 8); } 
           ;

GDeclList  : GDeclList GDecl | GDecl ;

GDecl      : INT { DeclType = TLookup("int"); DeclClass = NULL; } GIdList ';' 
           | STR { DeclType = TLookup("str"); DeclClass = NULL; } GIdList ';'
           | ID  { 
                 DeclType = TLookup($1->varname); 
                 DeclClass = CLookup($1->varname); 
                 if(!DeclType && !DeclClass) { printf("Error: Unknown Type/Class %s\n", $1->varname); exit(1); }
             } GIdList ';' 
           ;

GIdList    : GIdList ',' GId | GId ;

GId : ID { GInstall($1->varname, DeclType, DeclClass, 1, -1, NULL); }
    | '*' ID { GInstall($2->varname, DeclType, DeclClass, 1, -1, NULL); }
    | ID '[' NUM ']' { GInstall($1->varname, DeclType, DeclClass, $3->val, -1, NULL); }
    | ID '[' NUM ']' '[' NUM ']' { GInstall($1->varname, DeclType, DeclClass, $3->val * $6->val, $6->val, NULL); }
    | ID '(' ParamList ')' { GInstall($1->varname, DeclType, DeclClass, -1, -1, current_param_list); current_param_list = NULL; }
    ;

ParamList : ParamListInner | /* empty */ { current_param_list = NULL; } ;
ParamListInner : ParamListInner ',' Param | Param ;
Param : INT ID { addParam($2->varname, "int"); }
      | STR ID { addParam($2->varname, "str"); }
      | ID ID  { addParam($2->varname, $1->varname); }
      ;

/* ==================== LOCAL DECLARATIONS ==================== */
LDeclBlock : DECL LDecList ENDDECL | DECL ENDDECL | /* empty */ ;
LDecList   : LDecList LDecl | LDecl ;
LDecl      : INT { DeclType = TLookup("int"); } IdList ';' 
           | STR { DeclType = TLookup("str"); } IdList ';' 
           | ID  { 
               DeclType = TLookup($1->varname); 
               if(!DeclType) { printf("Error: Local variable type %s invalid (cannot be a class).\n", $1->varname); exit(1); }
             } IdList ';' 
           ;

IdList     : IdList ',' ID { LInstall($3->varname, DeclType, 0); }
           | IdList ',' '*' ID { LInstall($4->varname, DeclType, 0); }
           | ID { LInstall($1->varname, DeclType, 0); }
           | '*' ID { LInstall($2->varname, DeclType, 0); }
           ;

/* ==================== FUNCTIONS & MAIN ==================== */
FDefList : FDefList FDef | /* empty */ ;

FDef : INT ID '(' ParamList ')' '{' {
        VerifyFunctionSignature($2->varname, TLookup("int"), current_param_list);
        struct Paramstruct* temp = current_param_list;
        while(temp != NULL) { LInstall(temp->name, temp->type, 1); temp = temp->next; }
    } LDeclBlock START Statements RetStmt END '}' {
        struct tnode* body = createTree(0, NULL, NULL, NULL, NODE_CONN, NULL, $10, NULL, $11);
        FILE* out = fopen("output.xsm", "a");
        generateFuncCode($2->varname, body, out);
        fclose(out);
        ClearLST(); current_param_list = NULL;
    }
    | STR ID '(' ParamList ')' '{' {
        VerifyFunctionSignature($2->varname, TLookup("str"), current_param_list);
        struct Paramstruct* temp = current_param_list;
        while(temp != NULL) { LInstall(temp->name, temp->type, 1); temp = temp->next; }
    } LDeclBlock START Statements RetStmt END '}' {
        struct tnode* body = createTree(0, NULL, NULL, NULL, NODE_CONN, NULL, $10, NULL, $11);
        FILE* out = fopen("output.xsm", "a");
        generateFuncCode($2->varname, body, out);
        fclose(out);
        ClearLST(); current_param_list = NULL;
    }
    | ID ID '(' ParamList ')' '{' {
        struct Typetable* t = TLookup($1->varname);
        if(!t) { printf("Error: Invalid function return type %s\n", $1->varname); exit(1); }
        VerifyFunctionSignature($2->varname, t, current_param_list);
        struct Paramstruct* temp = current_param_list;
        while(temp != NULL) { LInstall(temp->name, temp->type, 1); temp = temp->next; }
    } LDeclBlock START Statements RetStmt END '}' {
        struct tnode* body = createTree(0, NULL, NULL, NULL, NODE_CONN, NULL, $10, NULL, $11);
        FILE* out = fopen("output.xsm", "a");
        generateFuncCode($2->varname, body, out);
        fclose(out);
        ClearLST(); current_param_list = NULL;
    }
    ;

MainBlock : INT MAIN '(' ')' '{' {
        GInstall("main", TLookup("int"), NULL, -1, -1, NULL); 
    } LDeclBlock START Statements RetStmt END '}' {
        struct tnode* body = createTree(0, NULL, NULL, NULL, NODE_CONN, NULL, $9, NULL, $10);
        FILE* out = fopen("output.xsm", "a"); 
        generateFuncCode("main", body, out);
        fclose(out);
        ClearLST();

        FILE* header = fopen("header.xsm", "w");
        fprintf(header, "0\n2056\n0\n0\n0\n0\n0\n0\n"); 
        
        extern int memory_binding;
        fprintf(header, "MOV SP, %d\n", memory_binding);
        fprintf(header, "MOV R0, 0\nPUSH R0\n");
        
        initializeVFT(header); 
        
        struct Gsymbol* main_entry = GLookUp("main");
        fprintf(header, "CALL F%d\n", main_entry->flabel);
        fprintf(header, "MOV R0, \"Exit\"\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nPUSH R0\nCALL 0\n");
        fclose(header);
        system("cat header.xsm output.xsm > temp.xsm && mv temp.xsm output.xsm && rm header.xsm");
    }
    ;

/* ==================== STATEMENTS & CONTROL FLOW ==================== */
Statements : Slist { $$ = $1; } 
           | /* empty */ { $$ = NULL; } 
           ;

Slist   : Slist Stmt { 
            if($1 == NULL) $$ = $2;
            else $$ = createTree(0, NULL, NULL, NULL, NODE_CONN, NULL, $1, NULL, $2); 
          } 
        | Stmt { $$ = $1; } 
        ;

RetStmt : RETURN E ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_RET, NULL, $2, NULL, NULL); } ;

Stmt    : Variable '=' E ';' { 
            // STAGE 8: POLYMORPHISM CHECK
            if ($1->ctype != NULL) {
                if ($3->nodetype != NODE_NULL && $3->nodetype != NODE_NEW) {
                    if (!isDescendant($1->ctype, $3->ctype)) {
                        printf("Error: Type mismatch. %s is not a descendant of %s.\n", $3->ctype->name, $1->ctype->name); exit(1);
                    }
                }
            }
            $$ = createTree(0, NULL, NULL, NULL, NODE_ASSIGN, "=", $1, NULL, $3); 
        }
        | Variable '=' NEW '(' ID ')' ';' { 
            struct Classtable* c = CLookup($5->varname);
            if(c == NULL) { printf("Error: Class %s not found.\n", $5->varname); exit(1); }
            
            // STAGE 8: POLYMORPHISM CHECK
            if(!isDescendant($1->ctype, c)) { 
                printf("Error: Class %s is not a descendant of %s.\n", c->name, $1->ctype->name); exit(1); 
            }
            
            struct tnode* n = createTree(0, NULL, c, NULL, NODE_NEW, NULL, NULL, NULL, NULL);
            $$ = createTree(0, NULL, NULL, NULL, NODE_ASSIGN, "=", $1, NULL, n); 
        }
        | DELETE '(' Variable ')' ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_DELETE, NULL, $3, NULL, NULL); }
        | READ '(' Variable ')' ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_READ, NULL, $3, NULL, NULL); }
        | WRITE '(' E ')' ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_WRITE, NULL, $3, NULL, NULL); }
        | IF '(' E ')' THEN Statements ELSE Statements ENDIF ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_IF, NULL, $3, $6, $8); }
        | IF '(' E ')' THEN Statements ENDIF ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_IF, NULL, $3, $6, NULL); }
        | WHILE '(' E ')' DO Statements ENDWHILE ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_WHILE, NULL, $3, $6, NULL); }
        | BREAK ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_BREAK, NULL, NULL, NULL, NULL); }
        | CONTINUE ';' { $$ = createTree(0, NULL, NULL, NULL, NODE_CONTINUE, NULL, NULL, NULL, NULL); }
        ;

/* ==================== EXPRESSIONS ==================== */
E       : E '+' E { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_OP, "+", $1, NULL, $3); }
        | E '-' E { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_OP, "-", $1, NULL, $3); }
        | E '*' E { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_OP, "*", $1, NULL, $3); }
        | E '/' E { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_OP, "/", $1, NULL, $3); }
        | E '%' E { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_OP, "%", $1, NULL, $3); }
        | E LT E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_LT, "<", $1, NULL, $3); }
        | E GT E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_GT, ">", $1, NULL, $3); }
        | E LE E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_LE, "<=", $1, NULL, $3); }
        | E GE E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_GE, ">=", $1, NULL, $3); }
        | E EQ E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_EQ, "==", $1, NULL, $3); }
        | E NE E  { $$ = createTree(0, TLookup("bool"), NULL, NULL, NODE_NE, "!=", $1, NULL, $3); }
        | '&' Variable { $$ = createTree(0, $2->type, $2->ctype, NULL, NODE_AMPER, NULL, $2, NULL, NULL); }
        | '(' E ')' { $$ = $2; }
        | NUM       { $$ = $1; $$->type = TLookup("int"); }
        | STRING    { $$ = $1; $$->type = TLookup("str"); }
        | Variable  { $$ = $1; }
        | ID '(' ArgList ')' {
            struct Gsymbol* g = GLookUp($1->varname);
            if(g == NULL) { printf("Error: Undefined function '%s'\n", $1->varname); exit(1); }
            $$ = createTree(0, g->type, NULL, $1->varname, NODE_FUNCALL, NULL, NULL, NULL, NULL);
            $$->Gentry = g; $$->arglist = $3;
        }
        | Variable '.' ID '(' ArgList ')' { 
            if($1->ctype == NULL) { printf("Error: Method call on non-object\n"); exit(1); }
            struct Memberfunclist* m = Class_Mlookup($1->ctype, $3->varname);
            if(m == NULL) { printf("Error: Method %s not found\n", $3->varname); exit(1); }
            $$ = createTree(0, m->type, NULL, $3->varname, NODE_METHODCALL, NULL, $1, NULL, NULL);
            $$->arglist = $5;
        }
        | INITIALIZE '(' ')' { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_INIT, NULL, NULL, NULL, NULL); }
        | ALLOC '(' ')'      { $$ = createTree(0, TLookup("void"), NULL, NULL, NODE_ALLOC, NULL, NULL, NULL, NULL); }
        | FREE '(' Variable ')' { $$ = createTree(0, TLookup("int"), NULL, NULL, NODE_FREE, NULL, $3, NULL, NULL); }
        | NUL                { $$ = createTree(0, TLookup("void"), NULL, NULL, NODE_NULL, NULL, NULL, NULL, NULL); }
        ;

ArgList : ArgListInner | { $$ = NULL; } ;
ArgListInner : ArgListInner ',' E { struct tnode* t = $1; while(t->mid) t = t->mid; t->mid = $3; $$ = $1; } | E { $$ = $1; } ;

/* ==================== VARIABLES ==================== */
Variable : ID {
            struct Lsymbol* local = LLookUp($1->varname);
            if (local) {
                $$ = createTree(0, local->type, NULL, $1->varname, NODE_ID, NULL, NULL, NULL, NULL);
                $$->Lentry = local;
            } else {
                struct Gsymbol* global = GLookUp($1->varname);
                if (!global) { printf("Error: Undeclared Variable '%s'\n", $1->varname); exit(1); }
                $$ = createTree(0, global->type, global->ctype, $1->varname, NODE_ID, NULL, NULL, NULL, NULL);
                $$->Gentry = global;
            }
         }
         | SELF { 
            if(CurrentClass == NULL) { printf("Error: 'self' used outside of a method.\n"); exit(1); }
            $$ = createTree(0, NULL, CurrentClass, "self", NODE_SELF, NULL, NULL, NULL, NULL); 
         }
         | ID '[' E ']' { 
            struct Gsymbol *entry = GLookUp($1->varname);
            $$ = createTree(0, entry->type, NULL, $1->varname, NODE_ARRAY, NULL, $3, NULL, NULL);
            $$->Gentry = entry;
         }
         | ID '[' E ']' '[' E ']' { 
            struct Gsymbol *entry = GLookUp($1->varname);
            $$ = createTree(0, entry->type, NULL, $1->varname, NODE_ARRAY2D, NULL, $3, $6, NULL);
            $$->Gentry = entry;
         }
         | '*' E %prec USTAR {
            $$ = createTree(0, $2->type, NULL, NULL, NODE_DEREF, NULL, $2, NULL, NULL);
         }
         | Variable '.' ID {
            if ($1->ctype != NULL) { 
                struct Fieldlist *f = Class_Flookup($1->ctype, $3->varname);
                if (f == NULL) { printf("Error: Field '%s' not found in class '%s'\n", $3->varname, $1->ctype->name); exit(1); }
                $$ = createTree(0, f->type, f->Ctype, $3->varname, NODE_FIELD, NULL, $1, NULL, NULL);
            } else { 
                struct Fieldlist *f = FLookup($1->type, $3->varname);
                if (f == NULL) { printf("Error: Field '%s' not found in type '%s'\n", $3->varname, $1->type->name); exit(1); }
                $$ = createTree(0, f->type, f->Ctype, $3->varname, NODE_FIELD, NULL, $1, NULL, NULL);
            }
         }
         ;

%%
void yyerror(char const *s) { printf("yyerror: %s\n", s); }

void addParam(char* name, char* type_name) {
    struct Typetable* type = TLookup(type_name);
    if(type == NULL) { printf("Error: Param type %s invalid (cannot be a class).\n", type_name); exit(1); }
    struct Paramstruct* param = (struct Paramstruct*)malloc(sizeof(struct Paramstruct));
    param->name = strdup(name);
    param->type = type;
    param->ctype = NULL;
    param->next = NULL;
    if (current_param_list == NULL) current_param_list = param;
    else {
        struct Paramstruct* temp = current_param_list;
        while(temp->next != NULL) temp = temp->next;
        temp->next = param;
    }
}

void addField(char* name, char* type_name) {
    struct Fieldlist* field = (struct Fieldlist*)malloc(sizeof(struct Fieldlist));
    field->name = strdup(name);
    field->fieldIndex = field_index++;
    field->type = TLookup(type_name);
    if(field->type == NULL) { printf("Error: UDT field type %s invalid.\n", type_name); exit(1); }
    field->Ctype = NULL;
    field->next = NULL;
    if (current_field_list == NULL) current_field_list = field;
    else {
        struct Fieldlist* temp = current_field_list;
        while(temp->next != NULL) temp = temp->next;
        temp->next = field;
    }
}

int main(int argc, char* argv[]) {
    TypeTableCreate(); 
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    FILE *out = fopen("output.xsm", "w");
    fclose(out);
    yyparse();
    return 0;
}