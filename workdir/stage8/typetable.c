#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typetable.h"
#include "classtable.h"

struct Typetable *Thead = NULL, *Ttail = NULL;

void TypeTableCreate() {
    TInstall("int", 1, NULL);
    TInstall("str", 1, NULL);
    TInstall("bool", 1, NULL);
    TInstall("void", 1, NULL); // Used for NULL/void returns
}

void TInstall(char *name, int size, struct Fieldlist *fields) {
    if (TLookup(name) != NULL) {
        printf("Error: Type %s already declared.\n", name);
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
    if (type == NULL) return NULL;
    struct Fieldlist *temp = type->fields;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void PrintTypeTable() {
    printf("\n=========================== TYPE TABLE ===========================\n");
    struct Typetable *temp = Thead;
    while (temp != NULL) {
        printf("TYPE: %-10s | SIZE: %d\n", temp->name, temp->size);
        temp = temp->next;
    }
    printf("==================================================================\n\n");
}