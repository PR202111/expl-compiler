
echo "1. Running Lex..."
lex expr.l

echo "2. Running Yacc..."
yacc -d expr.y

echo "3. Compiling C files..."
gcc y.tab.c lex.yy.c exprtree.c codegen.c -o compiler

echo "4. Cleaning up intermediate files..."
rm lex.yy.c y.tab.c y.tab.h

echo "5. Running the compiler on test.txt..."
./compiler test.txt

echo "========================================"
echo "Build and cleanup complete!"
echo "target.xsm is ready for the XSM simulator."
echo "========================================"