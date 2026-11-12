#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "typetable.h"
#include "classtable.h"

struct Paramstruct {
    char* name;
    struct Typetable *type;
    struct Classtable *ctype;
    struct Paramstruct* next;
};

struct Gsymbol {
    char* name;
    struct Typetable *type;
    struct Classtable *ctype; 
    int size;        
    int col_size;    
    int binding;     
    struct Paramstruct *paramlist;
    int flabel;
    struct Gsymbol* next;
};

struct Lsymbol {
    char* name;
    struct Typetable *type;
    int binding;
    struct Lsymbol* next;
};

struct Gsymbol* GLookUp(char *name);
void GInstall(char* name, struct Typetable *type, struct Classtable *ctype, int size, int col_size, struct Paramstruct* paramlist);
struct Lsymbol* LLookUp(char *name);
void LInstall(char* name, struct Typetable *type, int is_param);
void ClearLST();
void VerifyFunctionSignature(char* name, struct Typetable *ret_type, struct Paramstruct* def_params);
void PrintGST();

#endif