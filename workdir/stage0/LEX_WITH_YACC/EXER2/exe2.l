%{
#include <string.h>
#include "y.tab.h"
%}

%%
[a-zA-Z0-9]+ { yylval.str = strdup(yytext); return ID; }
[-+*/()]     { return yytext[0]; }
[\n]         { return NEWLINE; }
[ \t]        { /* Ignore whitespace */ }
.            { printf("Invalid character\n"); exit(1); }
%%

int yywrap() { return 1; }