#ifndef CLASSTABLE_H
#define CLASSTABLE_H

#include "typetable.h"
#include "symboltable.h"

struct Classtable {
    char *name;                           
    struct Fieldlist *memberfield;        
    struct Memberfunclist *vfuncptr;
    struct Classtable *parentptr;         
    int class_index;                      // position of the class in the virtual function table
    int fieldcount;                       // count of fields
    int methodcount;                      // count of methods
    struct Classtable *next;              
};

struct Memberfunclist {
    char *name;                      // name of the member function in the class
    struct Typetable *type;          // return type
    struct Paramstruct *paramlist;   // pointer to the head of formal parameters
    int funcposition;                // position of the function in the class table
    int flabel;                      // pseudo-address / label for the function
    struct Memberfunclist *next;
};

void ClassTableCreate();
struct Classtable* CInstall(char *name, char *parent_class_name);
struct Classtable* CLookup(char *name);
void Class_Finstall(struct Classtable *cptr, char *typename, char *name);
void Class_Minstall(struct Classtable *cptr, char *name, struct Typetable *type, struct Paramstruct *Paramlist);
struct Memberfunclist* Class_Mlookup(struct Classtable* Ctype, char* name);
struct Fieldlist* Class_Flookup(struct Classtable* Ctype, char* name);
void PrintClassTable();

#endif