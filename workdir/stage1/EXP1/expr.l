%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exprtree.h"
    #include "y.tab.h"
%}

%%

[0-9]+ {
    yylval.no = makeLeafNode(atoi(yytext));
    return NUM;
}

"+" { return '+'; }
"-" { return '-'; }
"*" { return '*'; }
"/" { return '/'; }

[()] { return *yytext; }

[ \t] {}  // Skip whitespace

[\n] { return '\n'; }

. {
    printf("Unknown character: %s\n", yytext);
    exit(1);
}

%%

int yywrap(void) {
    return 1;
}