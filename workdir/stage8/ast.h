#ifndef EXPRTREE_H
#define EXPRTREE_H

#include "symboltable.h"
#include "classtable.h"

// Define standard and new Stage 7 nodes
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
#define NODE_ARRAY2D 30
#define NODE_FIELD 31
#define NODE_ALLOC 32
#define NODE_FREE 33
#define NODE_INIT 34
#define NODE_NULL 35

// Stage 7 specific
#define NODE_METHODCALL 36
#define NODE_NEW 37
#define NODE_DELETE 38
#define NODE_SELF 39

typedef struct tnode {
    int val;
    struct Typetable *type; 
    struct Classtable *ctype; // Holds class context for object variables/methods
    char* varname;
    int nodetype;
    char* op;
    struct Gsymbol *Gentry;
    struct Lsymbol *Lentry; 
    struct tnode *left, *mid, *right;
    struct tnode *arglist;  
} tnode;

struct tnode* createTree(int val, struct Typetable *type, struct Classtable *ctype, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r);

int getReg();
void freeReg();
int getLabel();
int codeGen(struct tnode *t, FILE *target_file);
int getAddrCodeGen(struct tnode *t, FILE *target_file);
void generateFuncCode(char* name, struct tnode* body, FILE* target_file);
void initializeVFT(FILE* target_file); // Stage 7 setup

#endif