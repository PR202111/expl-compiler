#include <stdio.h>
#include <stdlib.h>
#include "exprtree.h"

int free_reg = 0;


int getReg() {
    if (free_reg > 19) {
        printf("Error: Out of registers!\n");
        exit(1);
    }
    return free_reg++;
}


void freeReg() {
    if (free_reg > 0) {
        free_reg--;
    }
}


int codeGen(struct tnode* t, FILE* fp) {
    if (t == NULL) return -1;

    switch (t->nodetype) {
        case NODE_CONN:
            codeGen(t->left, fp);
            codeGen(t->right, fp);
            return -1;

        case NODE_NUM: {
            int reg = getReg();
            fprintf(fp, "MOV R%d, %d\n", reg, t->val);
            return reg;
        }

        case NODE_ID: {
            int reg = getReg();
            int addr = 4096 + (t->varname[0] - 'a');
            fprintf(fp, "MOV R%d, [%d]\n", reg, addr);
            return reg;
        }

        case NODE_ASSIGN: {
            int right_reg = codeGen(t->right, fp);
            int addr = 4096 + (t->left->varname[0] - 'a');
            
            fprintf(fp, "MOV [%d], R%d\n", addr, right_reg);
            freeReg();
            return -1;
        }

        case NODE_READ: {
            int addr = 4096 + (t->left->varname[0] - 'a');
            int reg = getReg();
            
            fprintf(fp, "MOV R%d, \"Read\"\n", reg);
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 1: Function name
            fprintf(fp, "MOV R%d, -1\n", reg);      // -1 represents terminal read
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 2: File Descriptor
            fprintf(fp, "MOV R%d, %d\n", reg, addr); 
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 3: Memory Address
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 4: Empty
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 5: Empty
            fprintf(fp, "CALL 0\n");
            
            fprintf(fp, "POP R%d\n", reg);          // Return value
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            
            freeReg();
            return -1;
        }

        case NODE_WRITE: {

            int result_reg = codeGen(t->left, fp);
            int reg = getReg();
            
            fprintf(fp, "MOV R%d, \"Write\"\n", reg);
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 1: Function name
            fprintf(fp, "MOV R%d, -2\n", reg);      // -2 represents terminal write
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 2: File Descriptor
            fprintf(fp, "PUSH R%d\n", result_reg);  // Arg 3: Value to write
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 4: Empty
            fprintf(fp, "PUSH R%d\n", reg);         // Arg 5: Empty
            fprintf(fp, "CALL 0\n");
            
            fprintf(fp, "POP R%d\n", reg);          
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            fprintf(fp, "POP R%d\n", reg);
            
            freeReg();
            freeReg(); 
            return -1;
        }

        case NODE_OP: {
            int left_reg = codeGen(t->left, fp);
            int right_reg = codeGen(t->right, fp);
            
            if (t->op[0] == '+') fprintf(fp, "ADD R%d, R%d\n", left_reg, right_reg);
            if (t->op[0] == '-') fprintf(fp, "SUB R%d, R%d\n", left_reg, right_reg);
            if (t->op[0] == '*') fprintf(fp, "MUL R%d, R%d\n", left_reg, right_reg);
            if (t->op[0] == '/') fprintf(fp, "DIV R%d, R%d\n", left_reg, right_reg);
            
            freeReg();
            return left_reg;
        }
    }
    return -1;
}


void generateCode(struct tnode* root, FILE* fp) {

    fprintf(fp, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    
    fprintf(fp, "MOV SP, 4121\n");
    
    codeGen(root, fp);
    

    int reg = getReg();
    fprintf(fp, "MOV R%d, \"Exit\"\n", reg);
    fprintf(fp, "PUSH R%d\n", reg);
    fprintf(fp, "PUSH R%d\n", reg);
    fprintf(fp, "PUSH R%d\n", reg);
    fprintf(fp, "PUSH R%d\n", reg);
    fprintf(fp, "PUSH R%d\n", reg);
    fprintf(fp, "CALL 0\n");
}