#ifndef EXPRTREE_H
#define EXPRTREE_H


#define NODE_NUM 1
#define NODE_ID 2
#define NODE_READ 3
#define NODE_WRITE 4
#define NODE_ASSIGN 5
#define NODE_CONN 6
#define NODE_OP 7

typedef struct tnode {
    int val;
    int type;
    char* varname;
    int nodetype;
    char* op;
    struct tnode *left, *right;
} tnode;


struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *r);

#endif