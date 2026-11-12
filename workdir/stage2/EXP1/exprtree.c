#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "exprtree.h"


void printTree(struct tnode* t, int level) {
    if (t == NULL) return;
    
    
    for (int i = 0; i < level; i++) {
        printf("  | ");
    }
    
    
    if (t->nodetype == NODE_NUM) printf("-- NUM (%d)\n", t->val);
    else if (t->nodetype == NODE_ID) printf("-- ID (%s)\n", t->varname);
    else if (t->nodetype == NODE_OP) printf("-- OP (%s)\n", t->op);
    else if (t->nodetype == NODE_ASSIGN) printf("-- ASSIGN (=)\n");
    else if (t->nodetype == NODE_READ) printf("-- READ\n");
    else if (t->nodetype == NODE_WRITE) printf("-- WRITE\n");
    else if (t->nodetype == NODE_CONN) printf("-- CONNECTOR\n");
    

    printTree(t->left, level + 1);
    printTree(t->right, level + 1);
}

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
    
    printf("\n[Building Subtree]:\n");
    printTree(temp, 0);

    return temp;
}