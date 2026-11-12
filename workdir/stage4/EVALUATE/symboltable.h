#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

struct Gsymbol {
    char* name;
    int type;        // 1=int, 2=str, 3=int*, 4=str*
    int size;        // total size
    int col_size;    // column size for 2D arrays (or -1 if 1D/variable)
    int binding;     // address allocated to the variable
    struct Gsymbol* next;
};

struct Gsymbol* LookUp(char *name);
void Install(char* name, int type, int size, int col_size); // Updated signature
void PrintSymbolTable();

#endif