%{
#include "y.tab.h"
%}

%%
[a-zA-Z]    { yylval.c = yytext[0]; return ID; }
[-+*/()]    { return yytext[0]; }
[\n]        { return NEWLINE; }
[ \t]       { }
.           { printf("Invalid character\n"); exit(1); }
%%

int yywrap() { return 1; }