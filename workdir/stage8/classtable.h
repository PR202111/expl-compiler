#ifndef CLASSTABLE_H
#define CLASSTABLE_H

#include "typetable.h"
#include "symboltable.h"

struct Classtable {
    char *name;                           
    struct Fieldlist *memberfield;        
    struct Memberfunclist *vfuncptr;      
    struct Classtable *parentptr;         
    int class_index;                      
    int fieldcount;                       
    int methodcount;                      
    struct Classtable *next;              
};

struct Memberfunclist {
    char *name;                      
    struct Typetable *type;          
    struct Paramstruct *paramlist;   
    int funcposition;                
    int flabel;                      
    struct Memberfunclist *next;
};

void ClassTableCreate();
struct Classtable* CInstall(char *name, char *parent_class_name);
struct Classtable* CLookup(char *name);
void Class_Finstall(struct Classtable *cptr, char *typename, char *name);
void Class_Minstall(struct Classtable *cptr, char *name, struct Typetable *type, struct Paramstruct *Paramlist);
struct Memberfunclist* Class_Mlookup(struct Classtable* Ctype, char* name);
struct Fieldlist* Class_Flookup(struct Classtable* Ctype, char* name);
int isDescendant(struct Classtable* parent, struct Classtable* child);

#endif