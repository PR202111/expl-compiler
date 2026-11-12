#ifndef EXPRTREE_H
#define EXPRTREE_H

typedef struct tnode {
    int val;                // Value of the number for leaf nodes
    char *op;               // Operator (+, -, *, /) for internal nodes
    struct tnode *left;     // Left child
    struct tnode *right;    // Right child
} tnode;

struct tnode* makeLeafNode(int n);
struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r);
void prefix(tnode *t);
void postfix(tnode *t);

#endif