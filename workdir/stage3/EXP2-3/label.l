%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int address = 2056;     
int labels[100];
int pass = 1;
%}

%%

L[0-9]+:\n? {
    if (pass == 1) {
        int num = atoi(yytext + 1); // Extract the number after 'L'
        labels[num] = address;
    }

}

L[0-9]+ {
    if (pass == 2) {
        int num = atoi(yytext + 1);
        printf("%d", labels[num]);
    }
}

(MOV|ADD|SUB|MUL|DIV|LT|GT|LE|GE|EQ|NE|JZ|JNZ|JMP|PUSH|POP|CALL|RET|BRKP|INT) {

    if (pass == 1) {
        address += 2;
    }
    if (pass == 2) {
        printf("%s", yytext);
    }
}

.|\n {
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
    
    yylex(); 
    
    pass = 2;
    rewind(fp);
    yylex(); 
    
    fclose(fp);
    return 0;
}

int yywrap() {
    return 1;
}