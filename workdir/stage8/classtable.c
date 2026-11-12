#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "classtable.h"

struct Classtable *Chead = NULL, *Ctail = NULL;
int class_index_counter = 0;
extern int flabel_counter;

// Checks if 'child' is a descendant of 'parent' (or the exact same class)
int isDescendant(struct Classtable* parent, struct Classtable* child) {
    if (parent == NULL || child == NULL) return 0;
    struct Classtable* curr = child;
    while (curr != NULL) {
        if (strcmp(curr->name, parent->name) == 0) return 1;
        curr = curr->parentptr;
    }
    return 0;
}

struct Classtable* CInstall(char *name, char *parent_class_name) {
    if (CLookup(name) != NULL) {
        printf("Error: Class %s already defined.\n", name); exit(1);
    }

    struct Classtable *newClass = (struct Classtable*)malloc(sizeof(struct Classtable));
    newClass->name = strdup(name);
    newClass->memberfield = NULL;
    newClass->vfuncptr = NULL;
    newClass->parentptr = NULL; 
    newClass->class_index = class_index_counter++;
    newClass->fieldcount = 0;
    newClass->methodcount = 0;
    newClass->next = NULL;

    // STAGE 8: INHERITANCE (Copy fields and methods)
    if (parent_class_name != NULL) {
        struct Classtable *parent = CLookup(parent_class_name);
        if (parent == NULL) { printf("Error: Parent class %s not found.\n", parent_class_name); exit(1); }
        newClass->parentptr = parent;

        // Copy Member Fields
        struct Fieldlist *pField = parent->memberfield;
        struct Fieldlist *prevField = NULL;
        while(pField != NULL) {
            struct Fieldlist *f = (struct Fieldlist*)malloc(sizeof(struct Fieldlist));
            f->name = strdup(pField->name);
            f->fieldIndex = pField->fieldIndex;
            f->type = pField->type;
            f->Ctype = pField->Ctype;
            f->next = NULL;
            if(newClass->memberfield == NULL) newClass->memberfield = f;
            else prevField->next = f;
            prevField = f;
            newClass->fieldcount++;
            pField = pField->next;
        }

        // Copy Member Functions
        struct Memberfunclist *pMethod = parent->vfuncptr;
        struct Memberfunclist *prevMethod = NULL;
        while(pMethod != NULL) {
            struct Memberfunclist *m = (struct Memberfunclist*)malloc(sizeof(struct Memberfunclist));
            m->name = strdup(pMethod->name);
            m->type = pMethod->type;
            m->paramlist = pMethod->paramlist; 
            m->funcposition = pMethod->funcposition;
            m->flabel = pMethod->flabel; // Keep parent's flabel initially
            m->next = NULL;
            
            if(newClass->vfuncptr == NULL) newClass->vfuncptr = m;
            else prevMethod->next = m;
            prevMethod = m;
            newClass->methodcount++;
            pMethod = pMethod->next;
        }
    }

    if (Chead == NULL) Chead = Ctail = newClass;
    else { Ctail->next = newClass; Ctail = newClass; }
    
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
    // Prevent overriding parent fields
    if (Class_Flookup(cptr, name) != NULL) {
        printf("Error: Field %s already exists in class %s (cannot override).\n", name, cptr->name);
        exit(1);
    }
    if (cptr->fieldcount >= 8) {
        printf("Error: Class %s cannot have more than 8 fields.\n", cptr->name); exit(1);
    }

    struct Fieldlist *newField = (struct Fieldlist*)malloc(sizeof(struct Fieldlist));
    newField->name = strdup(name);
    newField->fieldIndex = cptr->fieldcount++;
    newField->type = TLookup(typename);
    newField->Ctype = CLookup(typename);
    newField->next = NULL;

    if (newField->type == NULL && newField->Ctype == NULL) {
        printf("Error: Unknown type %s for field %s.\n", typename, name); exit(1);
    }

    if (cptr->memberfield == NULL) cptr->memberfield = newField;
    else {
        struct Fieldlist *temp = cptr->memberfield;
        while (temp->next != NULL) temp = temp->next;
        temp->next = newField;
    }
}

void Class_Minstall(struct Classtable *cptr, char *name, struct Typetable *type, struct Paramstruct *Paramlist) {
    struct Memberfunclist* existing = Class_Mlookup(cptr, name);
    
    if (existing != NULL) {
        // STAGE 8: METHOD OVERRIDING
        if (existing->type != type) { 
            printf("Error: Return type mismatch for overridden method %s.\n", name); exit(1); 
        }
        
        // Verify signature matching
        struct Paramstruct* p1 = existing->paramlist;
        struct Paramstruct* p2 = Paramlist;
        while (p1 != NULL && p2 != NULL) {
            if (strcmp(p1->name, p2->name) != 0 || p1->type != p2->type) {
                printf("Error: Parameter mismatch for overridden method %s.\n", name); exit(1);
            }
            p1 = p1->next; p2 = p2->next;
        }
        if (p1 != NULL || p2 != NULL) {
            printf("Error: Parameter count mismatch for overridden method %s.\n", name); exit(1);
        }
        
        // Assign a new flabel for the overridden method, keep funcposition!
        existing->flabel = flabel_counter++; 
    } else {
        // NEW METHOD
        if (cptr->methodcount >= 8) {
            printf("Error: Class %s cannot have more than 8 methods.\n", cptr->name); exit(1);
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