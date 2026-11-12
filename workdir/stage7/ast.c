#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

int reg_counter = -1;
int label_counter = -1;
int while_start_stack[100];
int while_end_stack[100];
int while_top = -1;

extern int local_binding; 
extern int class_index_counter; 
extern struct Classtable *Chead;
int current_method_param_count = 0; 

struct tnode* createTree(int val, struct Typetable *type, struct Classtable *ctype, char* varname, int nodetype, char* op, struct tnode *l, struct tnode *m, struct tnode *r) {
    struct tnode *temp = (struct tnode*)malloc(sizeof(struct tnode));
    temp->val = val;
    temp->type = type;
    temp->ctype = ctype;
    temp->varname = varname != NULL ? strdup(varname) : NULL;
    temp->op = op != NULL ? strdup(op) : NULL;
    temp->nodetype = nodetype;
    temp->Gentry = NULL;
    temp->Lentry = NULL;
    temp->arglist = NULL;
    temp->left = l;
    temp->mid = m;       
    temp->right = r;
    return temp;
}

int getReg() {
    if (reg_counter < 19) return ++reg_counter;
    printf("Error: Out of registers!\n");
    exit(1);
}

void freeReg() {
    if (reg_counter >= 0) reg_counter--;
}

int getLabel() {
    return ++label_counter;
}

void initializeVFT(FILE* target_file) {
    struct Classtable *c = Chead;
    while(c != NULL) {
        int vft_start = 4096 + (c->class_index * 8);
        struct Memberfunclist *m = c->vfuncptr;
        while(m != NULL) {
            fprintf(target_file, "MOV [%d], F%d\n", vft_start + m->funcposition, m->flabel);
            m = m->next;
        }
        c = c->next;
    }
}

int getAddrCodeGen(struct tnode *t, FILE *target_file) {
    if (t->nodetype == NODE_DEREF) return codeGen(t->left, target_file); 
    
    if (t->nodetype == NODE_SELF) {
        int r = getReg();
        int offset = current_method_param_count + 4; // Mem_Ptr location
        fprintf(target_file, "MOV R%d, BP\n", r);
        int temp = getReg();
        fprintf(target_file, "MOV R%d, %d\n", temp, offset);
        fprintf(target_file, "SUB R%d, R%d\n", r, temp);
        freeReg();
        return r;
    }

    if (t->nodetype == NODE_FIELD) {
        int base_addr_reg = getAddrCodeGen(t->left, target_file);
        fprintf(target_file, "MOV R%d, [R%d]\n", base_addr_reg, base_addr_reg); 
        
        int offset_reg = getReg();
        if (t->left->ctype != NULL) {
            struct Fieldlist *f = Class_Flookup(t->left->ctype, t->varname);
            fprintf(target_file, "MOV R%d, %d\n", offset_reg, f->fieldIndex);
        } else {
            struct Fieldlist *f = FLookup(t->left->type, t->varname);
            fprintf(target_file, "MOV R%d, %d\n", offset_reg, f->fieldIndex);
        }
        fprintf(target_file, "ADD R%d, R%d\n", base_addr_reg, offset_reg);
        freeReg();
        return base_addr_reg;
    }

    int r = getReg();
    if (t->nodetype == NODE_ID) {
        if (t->Lentry != NULL) {
            int offset = t->Lentry->binding;
            fprintf(target_file, "MOV R%d, BP\n", r);
            if (offset > 0) {
                int o = getReg(); 
                fprintf(target_file, "MOV R%d, %d\nADD R%d, R%d\n", o, offset, r, o); 
                freeReg();
            } else {
                int o = getReg(); 
                fprintf(target_file, "MOV R%d, %d\nSUB R%d, R%d\n", o, -offset, r, o); 
                freeReg();
            }
        } else if (t->Gentry != NULL) {
            fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
        }
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
        int temp_reg = getReg();
        fprintf(target_file, "MOV R%d, %d\n", temp_reg, t->Gentry->col_size);
        fprintf(target_file, "MUL R%d, R%d\n", row, temp_reg);
        freeReg(); 
        fprintf(target_file, "ADD R%d, R%d\n", row, col);
        freeReg(); 
        fprintf(target_file, "MOV R%d, %d\n", r, t->Gentry->binding);
        fprintf(target_file, "ADD R%d, R%d\n", r, row);
        freeReg(); 
        return r;
    } 
    printf("Error: Attempted to get address of non-addressable node.\n");
    exit(1);
}

int codeGen(struct tnode *t, FILE *target_file) {
    if (t == NULL) return -1;
    int p, q, temp, label_1, label_2;

    switch (t->nodetype) {
        case NODE_NUM:
            p = getReg();
            fprintf(target_file, "MOV R%d, %d\n", p, t->val);
            return p;
            
        case NODE_NULL:                   
            p = getReg();
            fprintf(target_file, "MOV R%d, -1\n", p);
            return p;

        case NODE_STR:
            p = getReg();
            fprintf(target_file, "MOV R%d, %s\n", p, t->varname); 
            return p;
            
        case NODE_ID:
        case NODE_ARRAY:
        case NODE_ARRAY2D:
        case NODE_FIELD:
        case NODE_SELF:
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
            if (t->left->ctype != NULL) {
                p = getAddrCodeGen(t->left, target_file); 
                
                if (t->right->nodetype == NODE_NEW) {
                    int current_reg = reg_counter;
                    for (int i = 0; i <= current_reg; i++) fprintf(target_file, "PUSH R%d\n", i);
                    
                    temp = getReg();
                    fprintf(target_file, "MOV R%d, \"Alloc\"\n", temp);
                    fprintf(target_file, "PUSH R%d\nMOV R%d, 8\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", temp, temp, temp, temp, temp, temp); 
                    fprintf(target_file, "CALL 0\nPOP R%d\n", temp);
                    int dmy = getReg();
                    fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", dmy, dmy, dmy, dmy);
                    freeReg();
                    
                    for (int i = current_reg; i >= 0; i--) fprintf(target_file, "POP R%d\n", i);
                    
                    fprintf(target_file, "MOV [R%d], R%d\n", p, temp); 
                    
                    int vft = getReg();
                    fprintf(target_file, "MOV R%d, %d\n", vft, 4096 + (t->right->ctype->class_index * 8));
                    
                    fprintf(target_file, "MOV R%d, R%d\nADD R%d, 1\n", temp, p, temp);
                    fprintf(target_file, "MOV [R%d], R%d\n", temp, vft); 
                    
                    freeReg(); freeReg(); freeReg(); 
                } else if (t->right->nodetype == NODE_NULL) {
                    temp = getReg();
                    fprintf(target_file, "MOV R%d, -1\n", temp);
                    fprintf(target_file, "MOV [R%d], R%d\n", p, temp);
                    
                    int p_plus_1 = getReg();
                    fprintf(target_file, "MOV R%d, R%d\nADD R%d, 1\n", p_plus_1, p, p_plus_1);
                    fprintf(target_file, "MOV [R%d], R%d\n", p_plus_1, temp);
                    
                    freeReg(); freeReg(); freeReg(); 
                } else {
                    q = getAddrCodeGen(t->right, target_file);
                    temp = getReg();
                    
                    fprintf(target_file, "MOV R%d, [R%d]\n", temp, q);
                    fprintf(target_file, "MOV [R%d], R%d\n", p, temp); 
                    
                    fprintf(target_file, "MOV R%d, R%d\nADD R%d, 1\n", temp, q, temp);
                    fprintf(target_file, "MOV R%d, [R%d]\n", temp, temp);
                    
                    int p_plus_1 = getReg();
                    fprintf(target_file, "MOV R%d, R%d\nADD R%d, 1\n", p_plus_1, p, p_plus_1);
                    fprintf(target_file, "MOV [R%d], R%d\n", p_plus_1, temp); 
                    
                    freeReg(); freeReg(); freeReg(); freeReg(); 
                }
            } else {
                p = getAddrCodeGen(t->left, target_file); 
                q = codeGen(t->right, target_file);       
                fprintf(target_file, "MOV [R%d], R%d\n", p, q);
                freeReg(); freeReg(); 
            }
            return -1;
            
        case NODE_METHODCALL: {
            int active_regs = reg_counter;
            for (int i = 0; i <= active_regs; i++) fprintf(target_file, "PUSH R%d\n", i);

            int obj_addr = getAddrCodeGen(t->left, target_file);
            int mem_ptr = getReg();
            int vft_ptr = getReg();
            
            fprintf(target_file, "MOV R%d, [R%d]\n", mem_ptr, obj_addr);
            fprintf(target_file, "MOV R%d, R%d\n", vft_ptr, obj_addr);
            fprintf(target_file, "ADD R%d, 1\n", vft_ptr);
            fprintf(target_file, "MOV R%d, [R%d]\n", vft_ptr, vft_ptr);

            // DO NOT free obj_addr, mem_ptr, vft_ptr yet! This prevents LIFO stack corruption.

            fprintf(target_file, "PUSH R%d\n", mem_ptr);
            fprintf(target_file, "PUSH R%d\n", vft_ptr);

            struct tnode* arg = t->arglist;
            int arg_count = 0;
            int arg_regs[20];
            while(arg != NULL) {
                arg_regs[arg_count++] = codeGen(arg, target_file);
                arg = arg->mid;
            }
            for (int i = arg_count - 1; i >= 0; i--) {
                fprintf(target_file, "PUSH R%d\n", arg_regs[i]);
                freeReg();
            }

            fprintf(target_file, "PUSH R0\n"); // Return value space

            struct Memberfunclist* m = Class_Mlookup(t->left->ctype, t->varname);
            
            // Reuse obj_addr as a temporary register to calculate offset
            fprintf(target_file, "MOV R%d, %d\n", obj_addr, m->funcposition);
            fprintf(target_file, "ADD R%d, R%d\n", vft_ptr, obj_addr);
            fprintf(target_file, "MOV R%d, [R%d]\n", vft_ptr, vft_ptr);

            fprintf(target_file, "CALL R%d\n", vft_ptr);
            
            reg_counter = active_regs; // Automatically frees obj_addr, mem_ptr, vft_ptr correctly
            int res_reg = getReg(); // allocates active_regs + 1
            fprintf(target_file, "POP R%d\n", res_reg);
            
            temp = getReg();
            for (int i = 0; i < arg_count; i++) fprintf(target_file, "POP R%d\n", temp); 
            fprintf(target_file, "POP R%d\n", temp); // VFT
            fprintf(target_file, "POP R%d\n", temp); // Mem
            freeReg(); 
            
            for (int i = active_regs; i >= 0; i--) fprintf(target_file, "POP R%d\n", i);
            
            return res_reg;
        }

        case NODE_FUNCALL: {
            int active_regs = reg_counter;
            for (int i = 0; i <= active_regs; i++) fprintf(target_file, "PUSH R%d\n", i);
            
            struct tnode* arg = t->arglist;
            int arg_count = 0;
            int arg_regs[20];
            while(arg != NULL) {
                arg_regs[arg_count++] = codeGen(arg, target_file);
                arg = arg->mid;
            }
            for (int i = arg_count - 1; i >= 0; i--) {
                fprintf(target_file, "PUSH R%d\n", arg_regs[i]);
                freeReg();
            }
            
            fprintf(target_file, "PUSH R0\n"); 
            fprintf(target_file, "CALL F%d\n", t->Gentry->flabel);
            
            reg_counter = active_regs; 
            int res_reg = getReg();
            fprintf(target_file, "POP R%d\n", res_reg);
            
            temp = getReg();
            for (int i = 0; i < arg_count; i++) fprintf(target_file, "POP R%d\n", temp);
            freeReg();
            
            for (int i = active_regs; i >= 0; i--) fprintf(target_file, "POP R%d\n", i);
            return res_reg;
        }

        case NODE_READ:
            p = getAddrCodeGen(t->left, target_file); 
            q = getReg();
            fprintf(target_file, "MOV R%d, \"Read\"\n", q);
            fprintf(target_file, "PUSH R%d\nMOV R%d, -1\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", q, q, q, p, q, q);
            fprintf(target_file, "CALL 0\n");
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
            freeReg(); freeReg(); 
            return -1;
            
        case NODE_WRITE:
            p = codeGen(t->left, target_file);
            q = getReg();
            fprintf(target_file, "MOV R%d, \"Write\"\n", q);
            fprintf(target_file, "PUSH R%d\nMOV R%d, -2\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", q, q, q, p, q, q);
            fprintf(target_file, "CALL 0\n");
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", q, q, q, q, q);
            freeReg(); freeReg();
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
            
        case NODE_BREAK:
            if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_end_stack[while_top]);
            return -1;
            
        case NODE_CONTINUE:
            if (while_top >= 0) fprintf(target_file, "JMP L%d\n", while_start_stack[while_top]);
            return -1;

        case NODE_RET: {
            p = codeGen(t->left, target_file);
            
            int addr = getReg();
            fprintf(target_file, "MOV R%d, BP\n", addr);
            int two = getReg();
            fprintf(target_file, "MOV R%d, 2\n", two);
            fprintf(target_file, "SUB R%d, R%d\n", addr, two);
            fprintf(target_file, "MOV [R%d], R%d\n", addr, p);
            freeReg(); freeReg(); freeReg(); // Frees p correctly
            
            int local_count = local_binding - 1;
            temp = getReg();
            for(int i = 0; i < local_count; i++){
                fprintf(target_file, "POP R%d\n", temp);
            }
            freeReg();
            
            fprintf(target_file, "POP BP\n");
            fprintf(target_file, "RET\n");
            return -1;
        }
        
        case NODE_ALLOC:
        case NODE_INIT: {
            int current_reg = reg_counter;
            for (int i = 0; i <= current_reg; i++) fprintf(target_file, "PUSH R%d\n", i);
            
            int q = getReg();
            if (t->nodetype == NODE_ALLOC) {
                fprintf(target_file, "MOV R%d, \"Alloc\"\n", q);
                fprintf(target_file, "PUSH R%d\nMOV R%d, 8\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", q, q, q, q, q, q); 
            } else {
                fprintf(target_file, "MOV R%d, \"Heapset\"\n", q);
                fprintf(target_file, "PUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", q, q, q, q, q); 
            }
            fprintf(target_file, "CALL 0\n");
            
            fprintf(target_file, "POP R%d\n", q);
            int dummy = getReg();
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", dummy, dummy, dummy, dummy);
            freeReg(); 
            
            for (int i = current_reg; i >= 0; i--) fprintf(target_file, "POP R%d\n", i);
            reg_counter = current_reg + 1;
            return current_reg + 1; 
        }
        
        case NODE_DELETE:
        case NODE_FREE: {
            int arg_reg = -1;
            if (t->nodetype == NODE_DELETE) {
                int addr = getAddrCodeGen(t->left, target_file);
                fprintf(target_file, "MOV R%d, [R%d]\n", addr, addr); 
                arg_reg = addr;
            } else {
                arg_reg = codeGen(t->left, target_file);
            }

            int current_reg = reg_counter;
            for (int i = 0; i <= current_reg; i++) fprintf(target_file, "PUSH R%d\n", i);
            
            int q = getReg();
            fprintf(target_file, "MOV R%d, \"Free\"\n", q);
            fprintf(target_file, "PUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\nPUSH R%d\n", q, arg_reg, q, q, q); 
            fprintf(target_file, "CALL 0\n");
            
            fprintf(target_file, "POP R%d\n", q);
            int dummy = getReg();
            fprintf(target_file, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", dummy, dummy, dummy, dummy);
            freeReg(); 
            
            for (int i = current_reg; i >= 0; i--) fprintf(target_file, "POP R%d\n", i);
            
            fprintf(target_file, "MOV R%d, R%d\n", arg_reg, q);
            reg_counter = arg_reg; 
            return arg_reg;
        }
    }
    return -1;
}

void generateFuncCode(char* name, struct tnode* body, FILE* target_file) {
    int flabel = -1;
    if (body->ctype != NULL) {
        struct Memberfunclist* m = Class_Mlookup(body->ctype, name);
        flabel = m->flabel;
    } else {
        struct Gsymbol* g = GLookUp(name);
        flabel = g->flabel;
    }
    
    fprintf(target_file, "F%d:\n", flabel);
    fprintf(target_file, "PUSH BP\nMOV BP, SP\n");
    
    int local_count = local_binding - 1;
    int r = getReg();
    for (int i = 0; i < local_count; i++) {
        fprintf(target_file, "PUSH R%d\n", r);
    }
    freeReg();
    
    codeGen(body, target_file);
}