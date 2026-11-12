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
    temp->mid = m;       
    temp->right = r;
    return temp;
}

int reg_counter = -1;
int label_counter = -1;
int while_start_stack[100];
int while_end_stack[100];
int while_top = -1;

int getReg() {
    if (reg_counter < 19) {
        return ++reg_counter;
    }
    printf("Error: Out of registers!\n");
    exit(1);
}

void freeReg() {
    if (reg_counter >= 0) {
        reg_counter--;
    }
}


int getLabel() {
    return ++label_counter;
}


int codeGen(struct tnode *t, FILE *target_file) {
    if (t == NULL) return -1;

    int p, q, label_1, label_2, var_address;

    switch (t->nodetype) {
        case NODE_NUM:
            p = getReg();
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
            return p;

        case NODE_ID:
            p = getReg();
            var_address = 4096 + (t->varname[0] - 'a');
            fprintf(target_file, "MOV R%d, [%d]\n", p, var_address);
            return p;

        case NODE_ASSIGN:
            p = codeGen(t->right, target_file);
            var_address = 4096 + (t->left->varname[0] - 'a');
            fprintf(target_file, "MOV [%d], R%d\n", var_address, p);
            freeReg();
            return -1;

        case NODE_READ:

            p = getReg();
            var_address = 4096 + (t->left->varname[0] - 'a');
            fprintf(target_file, "MOV R%d, \"Read\"\n", p);
            fprintf(target_file, "PUSH R%d\n", p);
            fprintf(target_file, "MOV R%d, -1\n", p);
            fprintf(target_file, "PUSH R%d\n", p);
            fprintf(target_file, "MOV R%d, %d\n", p, var_address);
            fprintf(target_file, "PUSH R%d\n", p);
            fprintf(target_file, "PUSH R%d\n", p);
            fprintf(target_file, "PUSH R%d\n", p);
            fprintf(target_file, "CALL 0\n");
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", p, p, p, p, p);
            freeReg();
            return -1;

        case NODE_WRITE:

            p = codeGen(t->left, target_file);
            q = getReg();
            fprintf(target_file, "MOV R%d, \"Write\"\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "MOV R%d, -2\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", p); // Push the value to write
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "CALL 0\n");
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
            freeReg();
            freeReg();
            return -1;

        case NODE_CONN:
            codeGen(t->left, target_file);
            codeGen(t->right, target_file);
            return -1;

        case NODE_OP:
        case NODE_LT:
        case NODE_GT:
        case NODE_LE:
        case NODE_GE:
        case NODE_EQ:
        case NODE_NE:
            p = codeGen(t->left, target_file);
            q = codeGen(t->right, target_file);
            
            if (strcmp(t->op, "+") == 0) fprintf(target_file, "ADD R%d, R%d\n", p, q);
            else if (strcmp(t->op, "-") == 0) fprintf(target_file, "SUB R%d, R%d\n", p, q);
            else if (strcmp(t->op, "*") == 0) fprintf(target_file, "MUL R%d, R%d\n", p, q);
            else if (strcmp(t->op, "/") == 0) fprintf(target_file, "DIV R%d, R%d\n", p, q);
            else if (strcmp(t->op, "<") == 0) fprintf(target_file, "LT R%d, R%d\n", p, q);
            else if (strcmp(t->op, ">") == 0) fprintf(target_file, "GT R%d, R%d\n", p, q);
            else if (strcmp(t->op, "<=") == 0) fprintf(target_file, "LE R%d, R%d\n", p, q);
            else if (strcmp(t->op, ">=") == 0) fprintf(target_file, "GE R%d, R%d\n", p, q);
            else if (strcmp(t->op, "==") == 0) fprintf(target_file, "EQ R%d, R%d\n", p, q);
            else if (strcmp(t->op, "!=") == 0) fprintf(target_file, "NE R%d, R%d\n", p, q);
            
            
            return p;

        case NODE_WHILE:
            label_1 = getLabel(); 
            label_2 = getLabel(); 
            
            while_top++;
            while_start_stack[while_top] = label_1;
            while_end_stack[while_top] = label_2;
            
            fprintf(target_file, "L%d:\n", label_1); 
            p = codeGen(t->left, target_file);       
            
            fprintf(target_file, "JZ R%d, L%d\n", p, label_2); 
            freeReg();
            
            codeGen(t->mid, target_file);
            fprintf(target_file, "JMP L%d\n", label_1);
            
            fprintf(target_file, "L%d:\n", label_2);
            
            while_top--;
            return -1;

        case NODE_BREAK:
            
            if (while_top >= 0) {
                fprintf(target_file, "JMP L%d\n", while_end_stack[while_top]);
            }
            return -1;
            
        case NODE_CONTINUE:

            if (while_top >= 0) {
                fprintf(target_file, "JMP L%d\n", while_start_stack[while_top]);
            }
            return -1;

        case NODE_IF:
            label_1 = getLabel(); 
            p = codeGen(t->left, target_file); 
            

            fprintf(target_file, "JZ R%d, L%d\n", p, label_1);
            freeReg();
            
            codeGen(t->mid, target_file);
            
            if (t->right != NULL) {

                label_2 = getLabel();
                fprintf(target_file, "JMP L%d\n", label_2);
                
                fprintf(target_file, "L%d:\n", label_1);
                codeGen(t->right, target_file);
                
                fprintf(target_file, "L%d:\n", label_2); 
            } else {
                
                fprintf(target_file, "L%d:\n", label_1);
            }
            return -1;

        case NODE_DOWHILE:
            label_1 = getLabel();
            label_2 = getLabel();
            int label_3 = getLabel(); 
            

            while_top++;
            while_start_stack[while_top] = label_2;
            while_end_stack[while_top] = label_3;
            
            fprintf(target_file, "L%d:\n", label_1);
            
            codeGen(t->mid, target_file);
            
            fprintf(target_file, "L%d:\n", label_2);
            p = codeGen(t->left, target_file);

            
            fprintf(target_file, "JNZ R%d, L%d\n", p, label_1); 
            freeReg();
            
            fprintf(target_file, "L%d:\n", label_3);
            
            while_top--;
            return -1;

        
    }
    return -1;
}