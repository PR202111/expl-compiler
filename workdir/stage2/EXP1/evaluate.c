#include <stdio.h>
#include <stdlib.h>
#include "exprtree.h"

// our memory
int memory[26];

int evaluate(struct tnode* t) {
    if (t == NULL) {
        return 0;
    }

    switch (t->nodetype) {
        case NODE_CONN:
            evaluate(t->left);
            evaluate(t->right);
            return 0;

        case NODE_NUM:
            return t->val;

        case NODE_ID:
            return memory[t->varname[0] - 'a'];

        case NODE_READ:
            printf("Enter value for %s: ", t->left->varname);
            scanf("%d", &memory[t->left->varname[0] - 'a']);
            return 0;

        case NODE_WRITE:
            printf("%d\n", evaluate(t->left));
            return 0;

        case NODE_ASSIGN:
            memory[t->left->varname[0] - 'a'] = evaluate(t->right);
            return 0;

        case NODE_OP: {
            int left_val = evaluate(t->left);
            int right_val = evaluate(t->right);
            
            if (t->op[0] == '+') return left_val + right_val;
            if (t->op[0] == '-') return left_val - right_val;
            if (t->op[0] == '*') return left_val * right_val;
            if (t->op[0] == '/') return left_val / right_val;
        }
    }
    return 0;
}