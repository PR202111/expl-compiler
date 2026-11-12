#include <stdio.h>
#include <stdlib.h>
#include "exprtree.h"

struct tnode* makeLeafNode(int n) {
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = n;
    temp->op = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r) {
    struct tnode *temp;
    temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->op = (char*)malloc(sizeof(char));
    *(temp->op) = op;
    temp->left = l;
    temp->right = r;
    return temp;
}

void prefix(tnode *t) {
    if (t == NULL)
        return;
    if (t->op != NULL)
        printf("%c ", *(t->op));
    else
        printf("%d ", t->val);
    prefix(t->left);
    prefix(t->right);
}

void postfix(tnode *t) {
    if (t == NULL)
        return;
    postfix(t->left);
    postfix(t->right);
    if (t->op != NULL)
        printf("%c ", *(t->op));
    else
        printf("%d ", t->val);
}