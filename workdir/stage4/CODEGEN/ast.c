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


int getAddrCodeGen(struct tnode *t, FILE *target_file) {
    // For dereferences (e.g., *p), the address we want to act on IS the value stored in the pointer.
    if (t->nodetype == NODE_DEREF) {
        return codeGen(t->left, target_file); 
    }

    int r = getReg();
    
    if (t->nodetype == NODE_ID) {
        fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
        return r;
    } 
    else if (t->nodetype == NODE_ARRAY) {
        int offset = codeGen(t->left, target_file);
        fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
        fprintf(target_file, "ADD R%d, R%d\n", r, offset);
        freeReg(); 
        return r;
    } 
    else if (t->nodetype == NODE_ARRAY2D) {
        int row = codeGen(t->left, target_file);
        int col = codeGen(t->mid, target_file);
        int temp = getReg();
        
        fprintf(target_file, "MOV R%d, %d\n", temp, t->Gentry->col_size);
        fprintf(target_file, "MUL R%d, R%d\n", row, temp);
        freeReg(); 
        
        fprintf(target_file, "ADD R%d, R%d\n", row, col);
        freeReg(); 
        
        fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
        fprintf(target_file, "ADD R%d, R%d\n", r, row);
        freeReg(); 
        return r;
    } 

    printf("Error: Attempted to get address of non-addressable node type %d.\n", t->nodetype);
    exit(1);
}

int codeGen(struct tnode *t, FILE *target_file) {
    if (t == NULL) return -1;
    int p, q, label_1, label_2, label_3;

    switch (t->nodetype) {
        case NODE_NUM:
            p = getReg();
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
            return p;

        case NODE_STR:
            p = getReg();
            // Note: Make sure the lexer keeps quotes on strings, otherwise use \"_%s_\"
            fprintf(target_file, "MOV R%d, %s\n", p, t->varname); 
            return p;

        case NODE_ID:
        case NODE_ARRAY:
        case NODE_ARRAY2D:
            p = getAddrCodeGen(t, target_file);
            fprintf(target_file, "MOV R%d, [R%d]\n", p, p); 
            return p;

        case NODE_AMPER:
            return getAddrCodeGen(t->left, target_file);

        case NODE_DEREF:
            p = codeGen(t->left, target_file); 
            fprintf(target_file, "MOV R%d, [R%d]\n", p, p);
            return p;

        case NODE_ASSIGN:
            p = getAddrCodeGen(t->left, target_file); 
            q = codeGen(t->right, target_file);       
            fprintf(target_file, "MOV [R%d], R%d\n", p, q);
            freeReg(); 
            freeReg(); 
            return -1;

        case NODE_READ:
            p = getAddrCodeGen(t->left, target_file); 
            q = getReg();
            fprintf(target_file, "MOV R%d, \"Read\"\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "MOV R%d, -1\n", q); // File Descriptor: -1 for Terminal Input
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", p);   // Memory address to store the read value
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "INT 6\n");         // Inline interrupt for Read
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
            freeReg(); 
            freeReg(); 
            return -1;

        case NODE_WRITE:
            p = codeGen(t->left, target_file);       // Get value to write
            q = getReg();
            fprintf(target_file, "MOV R%d, \"Write\"\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "MOV R%d, -2\n", q); // File Descriptor: -2 for Terminal Output
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", p);   // Value to print
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "PUSH R%d\n", q);
            fprintf(target_file, "INT 7\n");         // Inline interrupt for Write
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
            
            if (strcmp(t->op, "+") == 0)      fprintf(target_file, "ADD R%d, R%d\n", p, q);
            else if (strcmp(t->op, "-") == 0) fprintf(target_file, "SUB R%d, R%d\n", p, q);
            else if (strcmp(t->op, "*") == 0) fprintf(target_file, "MUL R%d, R%d\n", p, q);
            else if (strcmp(t->op, "/") == 0) fprintf(target_file, "DIV R%d, R%d\n", p, q);
            else if (strcmp(t->op, "%") == 0) fprintf(target_file, "MOD R%d, R%d\n", p, q);
            else if (strcmp(t->op, "<") == 0) fprintf(target_file, "LT R%d, R%d\n", p, q);
            else if (strcmp(t->op, ">") == 0) fprintf(target_file, "GT R%d, R%d\n", p, q);
            else if (strcmp(t->op, "<=") == 0) fprintf(target_file, "LE R%d, R%d\n", p, q);
            else if (strcmp(t->op, ">=") == 0) fprintf(target_file, "GE R%d, R%d\n", p, q);
            else if (strcmp(t->op, "==") == 0) fprintf(target_file, "EQ R%d, R%d\n", p, q);
            else if (strcmp(t->op, "!=") == 0) fprintf(target_file, "NE R%d, R%d\n", p, q);
            
            freeReg(); 
            return p;

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

        case NODE_DOWHILE:
            label_1 = getLabel();
            label_2 = getLabel();
            label_3 = getLabel(); 
            
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

        case NODE_BREAK:
            if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_end_stack[while_top]);
            return -1;
            
        case NODE_CONTINUE:
            if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_start_stack[while_top]);
            return -1;
    }
    return -1;
}























// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include "ast.h"

// struct tnode* createTree(int val, int type, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r) {
//     if (nodetype == NODE_OP) {
//         if (l->type != inttype || r->type != inttype) {
//             printf("Type Error: Arithmetic operators require integer operands.\n");
//             exit(1);
//         }
//         type = inttype; 
//     }
//     else if (nodetype == NODE_LT || nodetype == NODE_LE || nodetype == NODE_GT || nodetype == NODE_GE || nodetype == NODE_EQ || nodetype == NODE_NE) {
//         if (l->type != r->type) {
//             printf("Type Error: Relational operators require matching types.\n");
//             exit(1);
//         }
//         type = booltype;
//     }
//     else if (nodetype == NODE_IF || nodetype == NODE_WHILE || nodetype == NODE_DOWHILE) {
//         if (l->type != booltype) {
//             printf("Type Error: Guard must be a boolean expression.\n");
//             exit(1);
//         }
//     }
//     else if (nodetype == NODE_ASSIGN) {
//         if (l->type != r->type) {
//             if(l->nodetype != NODE_DEREF && r->nodetype != NODE_AMPER) {
//                 printf("Type Error: Type mismatch in assignment.\n");
//                 exit(1);
//             }
//         }
//     }

//     struct tnode *temp = (struct tnode*)malloc(sizeof(struct tnode));
//     temp->val = val;
//     temp->type = type;
//     temp->varname = varname != NULL ? strdup(varname) : NULL;
//     temp->op = op != NULL ? strdup(op) : NULL;
//     temp->nodetype = nodetype;
//     temp->Gentry = NULL;
//     temp->left = l;
//     temp->mid = m;       
//     temp->right = r;
//     return temp;
// }


// int reg_counter = -1;
// int label_counter = -1;
// int while_start_stack[100];
// int while_end_stack[100];
// int while_top = -1;

// int getReg() {
//     if (reg_counter < 19) {
//         return ++reg_counter;
//     }
//     printf("Error: Out of registers!\n");
//     exit(1);
// }

// void freeReg() {
//     if (reg_counter >= 0) {
//         reg_counter--;
//     }
// }


// int getLabel() {
//     return ++label_counter;
// }


// int getAddrCodeGen(struct tnode *t, FILE *target_file) {
//     // For dereferences (e.g., *p), the address we want to act on IS the value stored in the pointer.
//     if (t->nodetype == NODE_DEREF) {
//         return codeGen(t->left, target_file); 
//     }

//     int r = getReg();
    
//     if (t->nodetype == NODE_ID) {
//         fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
//         return r;
//     } 
//     else if (t->nodetype == NODE_ARRAY) {
//         int offset = codeGen(t->left, target_file);
//         fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
//         fprintf(target_file, "ADD R%d, R%d\n", r, offset);
//         freeReg(); 
//         return r;
//     } 
//     else if (t->nodetype == NODE_ARRAY2D) {
//         int row = codeGen(t->left, target_file);
//         int col = codeGen(t->mid, target_file);
//         int temp = getReg();
        
//         fprintf(target_file, "MOV R%d, %d\n", temp, t->Gentry->col_size);
//         fprintf(target_file, "MUL R%d, R%d\n", row, temp);
//         freeReg(); 
        
//         fprintf(target_file, "ADD R%d, R%d\n", row, col);
//         freeReg(); 
        
//         fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
//         fprintf(target_file, "ADD R%d, R%d\n", r, row);
//         freeReg(); 
//         return r;
//     } 

//     printf("Error: Attempted to get address of non-addressable node type %d.\n", t->nodetype);
//     exit(1);
// }

// int codeGen(struct tnode *t, FILE *target_file) {
//     if (t == NULL) return -1;
//     int p, q, label_1, label_2, label_3;

//     switch (t->nodetype) {
//         case NODE_NUM:
//             p = getReg();
//             fprintf(target_file, "MOV R%d, %d\n", p, t->val);
//             return p;

//         case NODE_STR:
//             p = getReg();
//             // Note: Make sure the lexer keeps quotes on strings, otherwise use \"_%s_\"
//             fprintf(target_file, "MOV R%d, %s\n", p, t->varname); 
//             return p;

//         case NODE_ID:
//         case NODE_ARRAY:
//         case NODE_ARRAY2D:
//             p = getAddrCodeGen(t, target_file);
//             fprintf(target_file, "MOV R%d, [R%d]\n", p, p); 
//             return p;

//         case NODE_AMPER:
//             return getAddrCodeGen(t->left, target_file);

//         case NODE_DEREF:
//             p = codeGen(t->left, target_file); 
//             fprintf(target_file, "MOV R%d, [R%d]\n", p, p);
//             return p;

//         case NODE_ASSIGN:
//             p = getAddrCodeGen(t->left, target_file); 
//             q = codeGen(t->right, target_file);       
//             fprintf(target_file, "MOV [R%d], R%d\n", p, q);
//             freeReg(); 
//             freeReg(); 
//             return -1;

//         case NODE_READ:
//             p = getAddrCodeGen(t->left, target_file); 
//             q = getReg();
//             fprintf(target_file, "MOV R%d, \"Read\"\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "MOV R%d, -1\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "PUSH R%d\n", p); 
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "CALL 0\n");
//             fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
//             freeReg(); 
//             freeReg(); 
//             return -1;

//         case NODE_WRITE:
//             p = codeGen(t->left, target_file); // Get value to write
//             q = getReg();
//             fprintf(target_file, "MOV R%d, \"Write\"\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "MOV R%d, -2\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "PUSH R%d\n", p); 
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "PUSH R%d\n", q);
//             fprintf(target_file, "CALL 0\n");
//             fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
//             freeReg();
//             freeReg();
//             return -1;

//         case NODE_CONN:
//             codeGen(t->left, target_file);
//             codeGen(t->right, target_file);
//             return -1;

//         case NODE_OP:
//         case NODE_LT:
//         case NODE_GT:
//         case NODE_LE:
//         case NODE_GE:
//         case NODE_EQ:
//         case NODE_NE:
//             p = codeGen(t->left, target_file);
//             q = codeGen(t->right, target_file);
            
//             if (strcmp(t->op, "+") == 0) fprintf(target_file, "ADD R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "-") == 0) fprintf(target_file, "SUB R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "*") == 0) fprintf(target_file, "MUL R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "/") == 0) fprintf(target_file, "DIV R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "%") == 0) fprintf(target_file, "MOD R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "<") == 0) fprintf(target_file, "LT R%d, R%d\n", p, q);
//             else if (strcmp(t->op, ">") == 0) fprintf(target_file, "GT R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "<=") == 0) fprintf(target_file, "LE R%d, R%d\n", p, q);
//             else if (strcmp(t->op, ">=") == 0) fprintf(target_file, "GE R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "==") == 0) fprintf(target_file, "EQ R%d, R%d\n", p, q);
//             else if (strcmp(t->op, "!=") == 0) fprintf(target_file, "NE R%d, R%d\n", p, q);
            
//             freeReg(); 
//             return p;

//         case NODE_IF:
//             label_1 = getLabel();
//             p = codeGen(t->left, target_file);
            
//             fprintf(target_file, "JZ R%d, L%d\n", p, label_1);
//             freeReg();
            
//             codeGen(t->mid, target_file);
            
//             if (t->right != NULL) {
//                 label_2 = getLabel();
//                 fprintf(target_file, "JMP L%d\n", label_2);
//                 fprintf(target_file, "L%d:\n", label_1);
//                 codeGen(t->right, target_file);
//                 fprintf(target_file, "L%d:\n", label_2);
//             } else {
//                 fprintf(target_file, "L%d:\n", label_1);
//             }
//             return -1;

//         case NODE_WHILE:
//             label_1 = getLabel(); 
//             label_2 = getLabel(); 
            
//             while_top++;
//             while_start_stack[while_top] = label_1;
//             while_end_stack[while_top] = label_2;
            
//             fprintf(target_file, "L%d:\n", label_1); 
//             p = codeGen(t->left, target_file);       
            
//             fprintf(target_file, "JZ R%d, L%d\n", p, label_2); 
//             freeReg();
            
//             codeGen(t->mid, target_file);
//             fprintf(target_file, "JMP L%d\n", label_1);
//             fprintf(target_file, "L%d:\n", label_2);
            
//             while_top--;
//             return -1;

//         case NODE_DOWHILE:
//             label_1 = getLabel();
//             label_2 = getLabel();
//             label_3 = getLabel(); 
            
//             while_top++;
//             while_start_stack[while_top] = label_2;
//             while_end_stack[while_top] = label_3;
            
//             fprintf(target_file, "L%d:\n", label_1);
//             codeGen(t->mid, target_file);
            
//             fprintf(target_file, "L%d:\n", label_2);
//             p = codeGen(t->left, target_file);
            
//             fprintf(target_file, "JNZ R%d, L%d\n", p, label_1); 
//             freeReg();
            
//             fprintf(target_file, "L%d:\n", label_3);
//             while_top--;
//             return -1;

//         case NODE_BREAK:
//             if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_end_stack[while_top]);
//             return -1;
            
//         case NODE_CONTINUE:
//             if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_start_stack[while_top]);
//             return -1;
//     }
//     return -1;
// }