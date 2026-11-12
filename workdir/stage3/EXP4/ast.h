#ifndef EXPRTREE_H
#define EXPRTREE_H

// for the datatype
#define inttype 1
#define booltype 0

// for the operation type
#define NODE_NUM 2
#define NODE_ID 3
#define NODE_READ 4
#define NODE_WRITE 5
#define NODE_ASSIGN 6
#define NODE_CONN 7
#define NODE_OP 8
#define NODE_IF 9
#define NODE_WHILE 10
#define NODE_LT 11
#define NODE_LE 12
#define NODE_GT 13
#define NODE_GE 14
#define NODE_EQ 15
#define NODE_NE 16
#define NODE_BREAK 17
#define NODE_CONTINUE 18
#define NODE_DOWHILE 19


typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype;
    char* op;
    struct tnode *left, *mid, *right;
} tnode;

struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r);
int codeGen(struct tnode *t, FILE *target_file);
#endif