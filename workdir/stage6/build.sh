#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
XSM="$REPO_ROOT/xsm_dev/xsm"
LIB="$REPO_ROOT/library.lib"

cd "$SCRIPT_DIR"

lex lexer.l
yacc -d parser.y
gcc y.tab.c lex.yy.c ast.c symboltable.c typetable.c -o compiler
rm y.tab.c y.tab.h lex.yy.c

./compiler test.txt
rm compiler

lex label.l
gcc lex.yy.c -o label
rm lex.yy.c

./label output.xsm > final.xsm
rm label

"$XSM" -l "$LIB" -e final.xsm