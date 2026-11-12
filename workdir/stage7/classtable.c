#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "classtable.h"

struct Classtable *Chead = NULL, *Ctail = NULL;
int class_index_counter = 0;
extern int flabel_counter;

struct Classtable* CInstall(char *name, char *parent_class_name) {
    if (CLookup(name) != NULL) {
        printf("Error: Class %s already defined.\n", name);
        exit(1);
    }

    struct Classtable *newClass = (struct Classtable*)malloc(sizeof(struct Classtable));
    newClass->name = strdup(name);
    newClass->memberfield = NULL;
    newClass->vfuncptr = NULL;
    newClass->parentptr = NULL; // Inheritance disabled in Stage 7
    newClass->class_index = class_index_counter++;
    newClass->fieldcount = 0;
    newClass->methodcount = 0;
    newClass->next = NULL;

    if (Chead == NULL) {
        Chead = Ctail = newClass;
    } else {
        Ctail->next = newClass;
        Ctail = newClass;
    }
    return newClass;
}

struct Classtable* CLookup(char *name) {
    struct Classtable *temp = Chead;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void Class_Finstall(struct Classtable *cptr, char *typename, char *name) {
    if (Class_Flookup(cptr, name) != NULL) {
        printf("Error: Field %s already declared in class %s.\n", name, cptr->name);
        exit(1);
    }
    if (cptr->fieldcount >= 8) {
        printf("Error: Class %s cannot have more than 8 fields.\n", cptr->name);
        exit(1);
    }

    struct Fieldlist *newField = (struct Fieldlist*)malloc(sizeof(struct Fieldlist));
    newField->name = strdup(name);
    newField->fieldIndex = cptr->fieldcount++;
    newField->type = TLookup(typename);
    newField->Ctype = CLookup(typename);
    newField->next = NULL;

    if (newField->type == NULL && newField->Ctype == NULL) {
        printf("Error: Unknown type %s for field %s.\n", typename, name);
        exit(1);
    }

    if (cptr->memberfield == NULL) cptr->memberfield = newField;
    else {
        struct Fieldlist *temp = cptr->memberfield;
        while (temp->next != NULL) temp = temp->next;
        temp->next = newField;
    }
}

void Class_Minstall(struct Classtable *cptr, char *name, struct Typetable *type, struct Paramstruct *Paramlist) {
    if (Class_Mlookup(cptr, name) != NULL) {
        printf("Error: Method %s already declared in class %s.\n", name, cptr->name);
        exit(1);
    }
    if (cptr->methodcount >= 8) {
        printf("Error: Class %s cannot have more than 8 methods.\n", cptr->name);
        exit(1);
    }

    struct Memberfunclist *newFunc = (struct Memberfunclist*)malloc(sizeof(struct Memberfunclist));
    newFunc->name = strdup(name);
    newFunc->type = type;
    newFunc->paramlist = Paramlist;
    newFunc->funcposition = cptr->methodcount++;
    newFunc->flabel = flabel_counter++;
    newFunc->next = NULL;

    if (cptr->vfuncptr == NULL) cptr->vfuncptr = newFunc;
    else {
        struct Memberfunclist *temp = cptr->vfuncptr;
        while (temp->next != NULL) temp = temp->next;
        temp->next = newFunc;
    }
}

struct Memberfunclist* Class_Mlookup(struct Classtable* Ctype, char* name) {
    if (Ctype == NULL) return NULL;
    struct Memberfunclist *temp = Ctype->vfuncptr;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

struct Fieldlist* Class_Flookup(struct Classtable* Ctype, char* name) {
    if (Ctype == NULL) return NULL;
    struct Fieldlist *temp = Ctype->memberfield;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}