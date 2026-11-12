#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r) {
    
    if (nodetype == NODE_OP) {
        if (l->type != inttype || r->type != inttype) {
            printf("Type Error: Arithmetic operators require integer operands.\n");
            exit(1);
        }
        type = inttype; 
    }
    else if (nodetype == NODE_LT || nodetype == NODE_LE || nodetype == NODE_GT || nodetype == NODE_GE || nodetype == NODE_EQ || nodetype == NODE_NE) {
        if (l->type != r->type) {
            printf("Type Error: Relational operators require matching types.\n");
            exit(1);
        }
        type = booltype;
    }
    else if (nodetype == NODE_IF || nodetype == NODE_WHILE || nodetype == NODE_DOWHILE) {
        if (l->type != booltype) {
            printf("Type Error: Guard must be a boolean expression.\n");
            exit(1);
        }
    }
    else if (nodetype == NODE_ASSIGN) {
        if (l->type != r->type) {
            if(l->nodetype != NODE_DEREF && r->nodetype != NODE_AMPER) {
                printf("Type Error: Type mismatch in assignment.\n");
                exit(1);
            }
        }
    }

    struct tnode *temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->type = type;
    temp->varname = varname != NULL ? strdup(varname) : NULL;
    temp->op = op != NULL ? strdup(op) : NULL;
    temp->nodetype = nodetype;
    temp->Gentry = NULL;
    temp->left = l;
    temp->mid = m;       
    temp->right = r;
    return temp;
}


int sim_memory[10000]; 
char* sim_str_memory[10000]; 
int loop_control = 0; 

int getMemoryAddress(struct tnode *t) {
    if (t->nodetype == NODE_ID) return t->Gentry->binding;
    if (t->nodetype == NODE_ARRAY) {
        int offset = evaluate(t->left); 
        return t->Gentry->binding + offset;
    }
    if (t->nodetype == NODE_ARRAY2D) {
        int row = evaluate(t->left);
        int col = evaluate(t->mid);
        return t->Gentry->binding + (row * t->Gentry->col_size) + col;
    }
    if (t->nodetype == NODE_DEREF) return evaluate(t->left);
    return -1;
}


char* evaluateString(struct tnode *t) {
    if (t->nodetype == NODE_STR) return t->varname;
    if (t->nodetype == NODE_ID || t->nodetype == NODE_ARRAY || t->nodetype == NODE_ARRAY2D) {
        return sim_str_memory[getMemoryAddress(t)];
    }
    return "";
}

int evaluate(struct tnode *t) {
    if (t == NULL) return 0;
    
    switch (t->nodetype) {
        case NODE_NUM: return t->val;
        case NODE_STR: return 0; 
            
        case NODE_ID:
        case NODE_ARRAY:
        case NODE_ARRAY2D:
            return sim_memory[getMemoryAddress(t)];
            
        case NODE_AMPER: return getMemoryAddress(t->left);
        case NODE_DEREF: return sim_memory[evaluate(t->left)];
            
        case NODE_ASSIGN:
            
            if (t->left->type == strtype) {
                sim_str_memory[getMemoryAddress(t->left)] = evaluateString(t->right);
            } else {
                sim_memory[getMemoryAddress(t->left)] = evaluate(t->right);
            }
            return 0;
            
        case NODE_READ: {
            if (t->left->type == strtype) {
                char buffer[256];
                scanf("%s", buffer);
                char* quoted = malloc(strlen(buffer) + 3);
                sprintf(quoted, "\"%s\"", buffer); // Add quotes to match internal format
                sim_str_memory[getMemoryAddress(t->left)] = quoted;
            } else {
                int input;
                scanf("%d", &input);
                sim_memory[getMemoryAddress(t->left)] = input;
            }
            return 0;
        }
            
        case NODE_WRITE: {
            if (t->left->type == strtype) {
                char* str = evaluateString(t->left);
                if (str != NULL) {

                    for(int i = 1; str[i] != '\0' && str[i+1] != '\0'; i++) {
                        printf("%c", str[i]);
                    }
                    printf("\n");
                }
            } else {
                printf("%d\n", evaluate(t->left));
            }
            return 0;
        }
            
        case NODE_CONN:
            evaluate(t->left);
            if (loop_control != 0) return 0; 
            evaluate(t->right);
            return 0;
            
        case NODE_OP: {
            int l = evaluate(t->left);
            int r = evaluate(t->right);
            if (strcmp(t->op, "+") == 0) return l + r;
            if (strcmp(t->op, "-") == 0) return l - r;
            if (strcmp(t->op, "*") == 0) return l * r;
            if (strcmp(t->op, "/") == 0) return l / r;
            if (strcmp(t->op, "%") == 0) return l % r;
        }
        
        case NODE_LT: return evaluate(t->left) < evaluate(t->right);
        case NODE_GT: return evaluate(t->left) > evaluate(t->right);
        case NODE_LE: return evaluate(t->left) <= evaluate(t->right);
        case NODE_GE: return evaluate(t->left) >= evaluate(t->right);
        
        case NODE_EQ: 
            if (t->left->type == strtype) return strcmp(evaluateString(t->left), evaluateString(t->right)) == 0;
            return evaluate(t->left) == evaluate(t->right);
            
        case NODE_NE: 
            if (t->left->type == strtype) return strcmp(evaluateString(t->left), evaluateString(t->right)) != 0;
            return evaluate(t->left) != evaluate(t->right);
            
        case NODE_IF:
            if (evaluate(t->left)) evaluate(t->mid);
            else if (t->right != NULL) evaluate(t->right);
            return 0;
            
        case NODE_WHILE:
            while (evaluate(t->left)) {
                evaluate(t->mid);
                if (loop_control == 1) { loop_control = 0; break; }
                if (loop_control == 2) { loop_control = 0; continue; }
            }
            return 0;

        case NODE_DOWHILE:
            do {
                evaluate(t->mid);
                if (loop_control == 1) { loop_control = 0; break; }
                if (loop_control == 2) { loop_control = 0; continue; }
            } while (evaluate(t->left));
            return 0;

        case NODE_BREAK:
            loop_control = 1;
            return 0;
            
        case NODE_CONTINUE:
            loop_control = 2;
            return 0;
    }
    return 0;
}