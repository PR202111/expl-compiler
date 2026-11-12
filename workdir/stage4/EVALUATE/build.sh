#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$SCRIPT_DIR"

lex lexer.l
yacc -d parser.y
gcc y.tab.c lex.yy.c ast.c symboltable.c -o compiler
rm y.tab.c y.tab.h lex.yy.c

./compiler test.txt
rm compiler