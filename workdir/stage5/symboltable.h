#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

struct Paramstruct {
    char* name;
    int type;
    struct Paramstruct* next;
};

struct Gsymbol {
    char* name;
    int type;        
    int size;        
    int col_size;    
    int binding;     
    struct Paramstruct *paramlist;
    int flabel;
    struct Gsymbol* next;
};

struct Lsymbol {
    char* name;
    int type;
    int binding;
    struct Lsymbol* next;
};


struct Gsymbol* GLookUp(char *name);
void GInstall(char* name, int type, int size, int col_size, struct Paramstruct* paramlist);

struct Lsymbol* LLookUp(char *name);
void LInstall(char* name, int type, int is_param);
void ClearLST();

void VerifyFunctionSignature(char* name, int ret_type, struct Paramstruct* def_params);
void PrintGST();
void PrintLST(const char* scopeName);

#endif