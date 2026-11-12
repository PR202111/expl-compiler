%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int address = 2056;     // XSM starting memory address
int labels[100];        // Array to store memory addresses for L0 to L99
int pass = 1;           // Tracks if we are on Pass 1 or Pass 2
%}

%%

L[0-9]+:\n? {
    // PASS 1: We found a label declaration like "L2:". Save its address!
    if (pass == 1) {
        int num = atoi(yytext + 1); // Extract the number after 'L'
        labels[num] = address;
    }
    // PASS 2: Do nothing. This deletes the "L2:" line from the final output.
}

L[0-9]+ {
    // PASS 2: We found a label being used, like in "JMP L2". 
    // Look up the address in our array and print it instead.
    if (pass == 2) {
        int num = atoi(yytext + 1);
        printf("%d", labels[num]);
    }
}

(MOV|ADD|SUB|MUL|DIV|LT|GT|LE|GE|EQ|NE|JZ|JNZ|JMP|PUSH|POP|CALL|RET|BRKP|INT|MOD) {
    // These are the actual XSM instructions. Every instruction takes 2 memory words.
    if (pass == 1) {
        address += 2;
    }
    if (pass == 2) {
        printf("%s", yytext); // Just print the instruction normally
    }
}

.|\n {
    // For everything else (numbers, registers, spaces, the 8-line header)
    if (pass == 2) {
        printf("%s", yytext);
    }
}

%%

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./label <input_file.xsm>\n");
        return 1;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Error opening file\n");
        return 1;
    }
    
    yyin = fp;
    
    // --- PASS 1 ---
    yylex(); 
    
    // --- PASS 2 ---
    pass = 2;
    rewind(fp); // Go back to the start of the file
    yylex(); 
    
    fclose(fp);
    return 0;
}

int yywrap() {
    return 1;
}