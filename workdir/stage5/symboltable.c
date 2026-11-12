#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "symboltable.h"

struct Gsymbol* Ghead = NULL;
struct Gsymbol* Gtail = NULL;

struct Lsymbol* Lhead = NULL;
struct Lsymbol* Ltail = NULL;

int memory_binding = 4096;
int flabel_counter = 0;
int local_binding = 1;  
int param_binding = -3; 

struct Gsymbol* GLookUp(char *name){
    struct Gsymbol* temp = Ghead;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void GInstall(char* name, int type, int size, int col_size, struct Paramstruct* paramlist){
    if(GLookUp(name) != NULL){
        printf("Semantic Error: Global '%s' declared multiple times.\n", name);
        exit(1);
    }
    struct Gsymbol* new_node = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->size = size;
    new_node->col_size = col_size;
    new_node->paramlist = paramlist;
    
    if (size >= 0) {
        new_node->binding = memory_binding;
        memory_binding += size;
        new_node->flabel = -1;
    } else {
        new_node->binding = -1;
        new_node->flabel = flabel_counter++;
    }
    new_node->next = NULL;

    if(Ghead == NULL) Ghead = Gtail = new_node;
    else { Gtail->next = new_node; Gtail = new_node; }
}

struct Lsymbol* LLookUp(char *name){
    struct Lsymbol* temp = Lhead;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void LInstall(char* name, int type, int is_param){
    if(LLookUp(name) != NULL){
        printf("Semantic Error: Local '%s' declared multiple times.\n", name);
        exit(1);
    }
    struct Lsymbol* new_node = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    new_node->name = strdup(name);
    new_node->type = type;
    

    if (is_param) {
        new_node->binding = param_binding--; // Params go backwards (-3, -4...)
    } else {
        new_node->binding = local_binding++; // Locals go forwards (1, 2, 3...)
    }
    
    new_node->next = NULL;

    if(Lhead == NULL) Lhead = Ltail = new_node;
    else { Ltail->next = new_node; Ltail = new_node; }
}

void ClearLST() {
    struct Lsymbol* temp = Lhead;
    while(temp != NULL) {
        struct Lsymbol* next = temp->next;
        free(temp->name);
        free(temp);
        temp = next;
    }
    Lhead = Ltail = NULL;
    local_binding = 1;
    param_binding = -3;
}

void VerifyFunctionSignature(char* name, int ret_type, struct Paramstruct* def_params) {
    struct Gsymbol* g = GLookUp(name);
    if(g == NULL) { printf("Error: Function %s not declared.\n", name); exit(1); }
    if(g->type != ret_type) { printf("Error: Return type mismatch for %s.\n", name); exit(1); }
    
    struct Paramstruct* dec = g->paramlist;
    struct Paramstruct* def = def_params;
    while(dec != NULL && def != NULL) {
        if(strcmp(dec->name, def->name) != 0 || dec->type != def->type) {
            printf("Error: Parameter mismatch for function %s.\n", name); exit(1);
        }
        dec = dec->next; 
        def = def->next;
    }
    if(dec != NULL || def != NULL) {
        printf("Error: Parameter count mismatch for function %s.\n", name); exit(1);
    }
}

// Helper to convert type codes to readable strings
const char* getTypeName(int type) {
    if (type == 1) return "int";
    if (type == 2) return "str";
    if (type == 3) return "int*";
    if (type == 4) return "str*";
    return "unknown";
}

void PrintGST() {
    printf("\n======================== GLOBAL SYMBOL TABLE ========================\n");
    printf("%-15s | %-6s | %-4s | %-7s | %-6s | %s\n", "NAME", "TYPE", "SIZE", "BINDING", "FLABEL", "PARAMETERS");
    printf("---------------------------------------------------------------------\n");
    struct Gsymbol* temp = Ghead;
    while(temp != NULL) {
        printf("%-15s | %-6s | %-4d | %-7d | ", temp->name, getTypeName(temp->type), temp->size, temp->binding);
        
        if (temp->flabel != -1) printf("F%-5d | ", temp->flabel);
        else printf("%-6s | ", "-");
        
        struct Paramstruct* p = temp->paramlist;
        if (p == NULL) {
            printf("-");
        } else {
            while (p != NULL) {
                printf("%s %s", getTypeName(p->type), p->name);
                if (p->next != NULL) printf(", ");
                p = p->next;
            }
        }
        printf("\n");
        temp = temp->next;
    }
    printf("=====================================================================\n\n");
}

void PrintLST(const char* scopeName) {
    printf("\n--- Local Symbol Table (%s) ---\n", scopeName);
    printf("%-10s | %-6s | %-7s\n", "NAME", "TYPE", "BINDING");
    printf("----------------------------------\n");
    struct Lsymbol* temp = Lhead;
    while(temp != NULL) {
        printf("%-10s | %-6s | %-7d\n", temp->name, getTypeName(temp->type), temp->binding);
        temp = temp->next;
    }
    printf("----------------------------------\n\n");
}