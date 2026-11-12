#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "symboltable.h"

struct Gsymbol* head = NULL;
struct Gsymbol* tail = NULL;
int memory_binding = 4096;

struct Gsymbol* LookUp(char *name){
    struct Gsymbol* temp = head;
    while(temp != NULL){
        if(strcmp(temp->name,name) == 0){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void Install(char* name, int type, int size, int col_size){
    if(LookUp(name) != NULL){
        printf("Semantic Error: Variable '%s' declared multiple times\n",name);
        exit(1);
    }

    struct Gsymbol* new_node = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->size = size;
    new_node->col_size = col_size;
    new_node->binding = memory_binding;
    
    memory_binding += size;
    new_node->next = NULL;

    if(head == NULL){
        head = new_node;
        tail = new_node;
    } else {
        tail->next = new_node;
        tail = new_node;
    }
}

void PrintSymbolTable(){
    printf("\n--- Global Symbol Table ---\n");
    printf("NAME    | TYPE | SIZE | COLS | BINDING\n");
    struct Gsymbol* temp = head;
    while(temp != NULL){
        printf("%-7s | %-4d | %-4d | %-4d | %-7d\n", temp->name, temp->type, temp->size, temp->col_size, temp->binding);
        temp = temp->next;
    }
    printf("--------------------------------------\n\n");
}