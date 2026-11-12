#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "typetable.h"

struct Paramstruct {
    char* name;
    struct Typetable *type; // Changed to Typetable ptr
    struct Paramstruct* next;
};

struct Gsymbol {
    char* name;
    struct Typetable *type; // Changed to Typetable ptr
    int size;        
    int col_size;    
    int binding;     
    struct Paramstruct *paramlist;
    int flabel;
    struct Gsymbol* next;
};

struct Lsymbol {
    char* name;
    struct Typetable *type; // Changed to Typetable ptr
    int binding;
    struct Lsymbol* next;
};


struct Gsymbol* GLookUp(char *name);
void GInstall(char* name, struct Typetable *type, int size, int col_size, struct Paramstruct* paramlist);

struct Lsymbol* LLookUp(char *name);
void LInstall(char* name, struct Typetable *type, int is_param);
void ClearLST();

void VerifyFunctionSignature(char* name, struct Typetable *ret_type, struct Paramstruct* def_params);
void PrintGST();
void PrintLST(const char* scopeName);

#endif