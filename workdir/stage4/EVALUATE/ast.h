#ifndef EXPRTREE_H
#define EXPRTREE_H

#include "symboltable.h"

// Datatypes
#define booltype 0
#define inttype 1
#define strtype 2
#define intptrtype 3
#define strptrtype 4

// Node Types
#define NODE_NUM 5
#define NODE_ID 6
#define NODE_READ 7
#define NODE_WRITE 8
#define NODE_ASSIGN 9
#define NODE_CONN 10
#define NODE_OP 11
#define NODE_IF 12
#define NODE_WHILE 13
#define NODE_LT 14
#define NODE_LE 15
#define NODE_GT 16
#define NODE_GE 17
#define NODE_EQ 18
#define NODE_NE 19
#define NODE_BREAK 20
#define NODE_CONTINUE 21
#define NODE_DOWHILE 22
#define NODE_ARRAY 23
#define NODE_ARRAY2D 24
#define NODE_AMPER 25   // For &x
#define NODE_DEREF 26   // For *p
#define NODE_STR 27     // For "string"

typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype;
    char* op;
    struct Gsymbol *Gentry;
    struct tnode *left, *mid, *right;
} tnode;

struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r);
int evaluate(struct tnode *t);
#endif