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
#define NODE_RET 20
#define NODE_FUNCALL 21
#define NODE_ARRAY 22
#define NODE_DEREF 23
#define NODE_AMPER 24
#define NODE_STR 25
#define NODE_MAIN 26
#define NODE_BREAK 27
#define NODE_CONTINUE 28
#define NODE_DOWHILE 29
#define NODE_ARRAY2D 30

typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype;
    char* op;
    struct Gsymbol *Gentry;
    struct Lsymbol *Lentry; // Local symbol table entry
    struct tnode *left, *mid, *right;
    struct tnode *arglist;  // For function arguments
} tnode;

struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r);

int getReg();
void freeReg();
int getLabel();
int codeGen(struct tnode *t, FILE *target_file);
int getAddrCodeGen(struct tnode *t, FILE *target_file);
void generateFuncCode(char* name, struct tnode* body, FILE* target_file);

#endif