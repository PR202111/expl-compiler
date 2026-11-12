
echo "1. Running Lex..."
lex lexer.l

echo "2. Running Yacc..."
yacc -d parser.y

echo "3. Compiling C files..."
gcc y.tab.c lex.yy.c ast.c -o compiler

echo "4. Cleaning up intermediate files..."
rm lex.yy.c y.tab.c y.tab.h

echo "5. Running the compiler on test.txt..."
./compiler test.txt
