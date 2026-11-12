#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "exprtree.h"

struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *r) {
    struct tnode *temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->type = type;
    

    if (varname != NULL) {
        temp->varname = strdup(varname); // we are using strdup as the memory allocation will happen in one go
        // but in strcpy we need to preallocate the memory
    } else {
        temp->varname = NULL;
    }
    
    if (op != NULL) {
        temp->op = strdup(op);
    } else {
        temp->op = NULL;
    }
    
    temp->nodetype = nodetype;
    temp->left = l;
    temp->right = r;
    return temp;
}