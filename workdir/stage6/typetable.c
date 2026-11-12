#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typetable.h"

struct Typetable *Thead = NULL, *Ttail = NULL;

void TypeTableCreate() {
    TInstall("int", 1, NULL);
    TInstall("str", 1, NULL);
    TInstall("bool", 1, NULL);
    TInstall("void", 1, NULL); // Used for NULL
}

void TInstall(char *name, int size, struct Fieldlist *fields) {
    if (TLookup(name) != NULL) {
        printf("Error: Type %s already declared.\n", name);
        exit(1);
    }
    if (size > 8) {
        printf("Error: User defined type %s has more than 8 fields.\n", name);
        exit(1);
    }

    struct Typetable *newType = (struct Typetable*)malloc(sizeof(struct Typetable));
    newType->name = strdup(name);
    newType->size = size;
    newType->fields = fields;
    newType->next = NULL;

    if (Thead == NULL) {
        Thead = Ttail = newType;
    } else {
        Ttail->next = newType;
        Ttail = newType;
    }
}

struct Typetable* TLookup(char *name) {
    struct Typetable *temp = Thead;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

struct Fieldlist* FLookup(struct Typetable *type, char *name) {
    struct Fieldlist *temp = type->fields;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void PrintTypeTable() {
    printf("\n=========================== TYPE TABLE ===========================\n");
    printf("%-15s | %-4s | %s\n", "TYPE NAME", "SIZE", "FIELDS (Name : Type : Index)");
    printf("------------------------------------------------------------------\n");

    struct Typetable *temp = Thead;
    while (temp != NULL) {
        printf("%-15s | %-4d | ", temp->name, temp->size);
        
        struct Fieldlist *f = temp->fields;
        if (f == NULL) {
            printf("-"); // Primitive types have no fields
        } else {
            while (f != NULL) {
                // Safely get the type name of the field
                char* fieldTypeName = (f->type != NULL) ? f->type->name : "unknown";
                
                printf("%s : %s : %d", f->name, fieldTypeName, f->fieldIndex);
                if (f->next != NULL) {
                    printf(",  ");
                }
                f = f->next;
            }
        }
        printf("\n");
        temp = temp->next;
    }
    printf("==================================================================\n\n");
}