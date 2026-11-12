#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"


struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r) {

    // This is the new part here just basic of type checking we are strictly saying that 
    // int with int and boolen with boolean our if and while only supports boolean
    /***************************************************************************************************/
    if (nodetype == NODE_OP) {
        if (l->type != inttype || r->type != inttype) {
            printf("Type Error: Arithmetic operators require integer operands.\n");
            exit(1);
        }
        type = inttype; 
    }
    else if (nodetype == NODE_LT || nodetype == NODE_LE || nodetype == NODE_GT || nodetype == NODE_GE || nodetype == NODE_EQ || nodetype == NODE_NE) {
        if (l->type != inttype || r->type != inttype) {
            printf("Type Error: Relational operators require integer operands.\n");
            exit(1);
        }
        type = booltype;
    }
    else if (nodetype == NODE_IF || nodetype == NODE_WHILE) {
        if (l->type != booltype) {
            printf("Type Error: 'if' and 'while' guards must be boolean expressions.\n");
            exit(1);
        }
    }
    else if (nodetype == NODE_ASSIGN) {
        if (r->type != inttype) {
            printf("Type Error: Cannot assign a boolean to an integer variable.\n");
            exit(1);
        }
    }
    /***************************************************************************************************/

    struct tnode *temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->type = type;
    
    if (varname != NULL) {
        temp->varname = strdup(varname); 
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
    /***************************************************************************************************/
    temp->mid = m; // here we have mid pointer to support the body
    /***************************************************************************************************/      
    temp->right = r;
    return temp;
}

int memory[26];
int evaluate(struct tnode *t) {
    if (t == NULL) {
        return 0;
    }

    switch (t->nodetype) {
        case NODE_NUM:
            return t->val;

        case NODE_ID:
            return memory[t->varname[0] - 'a'];

        case NODE_READ:
            printf("Enter value for %s: ", t->left->varname);
            scanf("%d", &memory[t->left->varname[0] - 'a']);
            return 0;

        case NODE_WRITE:
            printf("Output: %d\n", evaluate(t->left));
            return 0;

        case NODE_ASSIGN:
            memory[t->left->varname[0] - 'a'] = evaluate(t->right);
            return 0;

        case NODE_CONN:
            evaluate(t->left);
            evaluate(t->right);
            return 0;

        case NODE_OP:
            if (strcmp(t->op, "+") == 0) return evaluate(t->left) + evaluate(t->right);
            if (strcmp(t->op, "-") == 0) return evaluate(t->left) - evaluate(t->right);
            if (strcmp(t->op, "*") == 0) return evaluate(t->left) * evaluate(t->right);
            if (strcmp(t->op, "/") == 0) return evaluate(t->left) / evaluate(t->right);
            return 0;

        /***************************************************************************************************/  
        case NODE_LT:
            return evaluate(t->left) < evaluate(t->right);
        case NODE_GT:
            return evaluate(t->left) > evaluate(t->right);
        case NODE_LE:
            return evaluate(t->left) <= evaluate(t->right);
        case NODE_GE:
            return evaluate(t->left) >= evaluate(t->right);
        case NODE_EQ:
            return evaluate(t->left) == evaluate(t->right);
        case NODE_NE:
            return evaluate(t->left) != evaluate(t->right);

        
        case NODE_IF:
            if (evaluate(t->left)) {
                evaluate(t->mid);
            } else if (t->right != NULL) {
                evaluate(t->right);
            }
            return 0;

        case NODE_WHILE:
            while (evaluate(t->left)) {
                evaluate(t->mid);
            }
            return 0;
        /***************************************************************************************************/
            
        default:
            return 0;
    }
}