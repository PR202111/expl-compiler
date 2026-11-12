# eXpL Compiler for XSM — Compiler Design Lab

A stage-by-stage implementation of a compiler for **eXpL** (eXperimental Programming Language), targeting the **XSM** (eXperimental String Machine) architecture. Built as part of the Compiler Design Laboratory curriculum at NITC.

The repository includes the full toolchain: the eXpL compiler (built across 9 progressive stages), an XSM simulator, an XFS disk interface, and an SPL system-language compiler.

---

## Repository Structure

```
compiler-lab-repo/
├── workdir/              # Stage-by-stage compiler source (core lab work)
│   ├── stage0/           # Lex & Yacc warm-up exercises
│   ├── stage1/           # Expression parsing and AST construction
│   ├── stage2/           # Simple interpreter with evaluation
│   ├── stage3/           # Control flow — if/while + XSM code generation
│   ├── stage4/           # Symbol table, declarations, full code generation
│   ├── stage5/           # User-defined functions and recursion
│   ├── stage6/           # User-defined types (structs) and heap memory
│   ├── stage7/           # Classes, objects, and methods (OOP stage 1)
│   └── stage8/           # Inheritance and dynamic dispatch (OOP stage 2)
├── xsm_dev/              # XSM simulator source (C)
├── spl/                  # SPL (System Programmer's Language) compiler source
├── xfs-interface/        # XFS disk filesystem interface source
├── library.lib           # eXpL standard library (XSM assembly)
└── Makefile              # Top-level build: spl + xfs-interface + xsm_dev
```

---

## Toolchain

| Component | Description |
|-----------|-------------|
| eXpL Compiler (`workdir/stageN/`) | Translates eXpL source to XSM assembly; built progressively across stages |
| XSM Simulator (`xsm_dev/`) | Simulates the XSM virtual machine to execute compiled `.xsm` programs |
| XFS Interface (`xfs-interface/`) | Manages the XSM virtual disk — format, load, and list files |
| SPL Compiler (`spl/`) | Compiles System Programmer's Language to XSM for OS-level routines |

---

## Building

### Prerequisites

```bash
# macOS
brew install flex bison gcc readline

# Ubuntu / Debian
sudo apt install flex bison gcc libreadline-dev
```

### Build all tools

```bash
make
```

### Build individually

```bash
cd xsm_dev       && make
cd spl           && make
cd xfs-interface && make
```

---

## Compiler Stages

### Stage 0 — Lex and Yacc Warm-up (`workdir/stage0/`)

Introductory exercises with Flex and Bison.

| File | Description |
|------|-------------|
| `LEX/exer1_1.l` | Count lines, words, and characters |
| `LEX/exer1_2.l` | Tokenize identifiers and numbers |
| `LEX/exer1_3.l` | Keyword vs identifier recognition |
| `LEX/exer1_4.l` | Multiple token classes |
| `LEX_WITH_YACC/EXER1/` | Infix expression to postfix |
| `LEX_WITH_YACC/EXER2/` | Arithmetic expression evaluator |
| `LEX_WITH_YACC/EXER3/` | Extended expression grammar |

---

### Stage 1 — AST Construction (`workdir/stage1/`)

Builds an Abstract Syntax Tree for arithmetic expressions.

| Sub-stage | Description |
|-----------|-------------|
| `EXP1` | Parse expressions, build and print the AST |
| `EXP2` | Emit XSM assembly from an expression AST |
| `EXP3` | Full expression to XSM with label resolution |

```bash
cd workdir/stage1/EXP3
lex exprtree.l && yacc -d exprtree.y
gcc y.tab.c lex.yy.c exprtree.c -o compiler
./compiler < sample_input.txt
```

---

### Stage 2 — Interpreter (`workdir/stage2/`)

A minimal interpreter with assignments, read/write, and arithmetic.

| Sub-stage | Description |
|-----------|-------------|
| `EXP1` | Direct AST evaluator (interprets without codegen) |
| `EXP2` | Code generator that emits XSM assembly |

---

### Stage 3 — Control Flow (`workdir/stage3/`)

Adds `if/then/else/endif` and `while/do/endwhile` with XSM code generation and label resolution via a secondary lexer (`label.l`).

| Sub-stage | Description |
|-----------|-------------|
| `EXP1` | AST nodes for control flow |
| `EXP2-3` | Code generation with label lexer |
| `EXP4` | Pointer and address-of operations |

---

### Stage 4 — Symbol Table and Declarations (`workdir/stage4/`)

Introduces global variable declarations, a symbol table, and complete XSM code generation.

```
decl
    int n, i;
enddecl

begin
    read(n);
    while (i < n) do
        write(i);
        i = i + 1;
    endwhile;
end
```

Test programs in `TESTS/`: bubble sort, Fibonacci, prime sieve, matrix operations, factorial with summation.

---

### Stage 5 — Functions (`workdir/stage5/`)

Adds user-defined functions, local symbol tables, and recursive calls using the XSM activation record and stack frame convention (`CALL`/`RET` instructions).

```bash
cd workdir/stage5 && bash build.sh
```

Test programs in `TESTS/`: recursive factorial, Fibonacci, GCD, sorting.

---

### Stage 6 — User-Defined Types (`workdir/stage6/`)

Adds struct-like user-defined types with field access (`.`), heap allocation (`alloc`, `free`, `initialize`), and a type table.

```
type
    Node {
        int data;
        Node next;
    }
endtype
```

Test programs: linked list, binary search tree, chaining hash table.

---

### Stage 7 — Classes and Objects (`workdir/stage7/`)

First OOP stage. Introduces classes with fields and methods, the `self` keyword, `new`/`delete`, and a class table.

```
class
    Counter {
        decl
            int count;
            int increment();
        enddecl
        int increment() {
            decl enddecl
            begin
                self.count = self.count + 1;
                return self.count;
            end
        }
    }
endclass
```

---

### Stage 8 — Inheritance and Dynamic Dispatch (`workdir/stage8/`)

Final stage. Adds single inheritance (`extends`), method overriding, and dynamic dispatch via a virtual method table (vtable).

```
class
    A { decl int f(); enddecl  int f() { ... } }
    B extends A { decl int f(); enddecl  int f() { ... } }
endclass
```

```bash
cd workdir/stage8 && bash build.sh
```

Test programs in `TESTS/`: polymorphic dispatch, Rectangle/Square hierarchy, linked list with classes, complex inheritance chains.

---

## XSM Simulator (`xsm_dev/`)

Simulates the eXperimental String Machine. Executes compiled `.xsm` programs.

```bash
cd xsm_dev && make
./xsm -l library.lib -e output.xsm
./xsm -l library.lib -e output.xsm --debug    # interactive debugger
```

The machine provides 16 general-purpose registers (`R0`–`R15`), stack pointer (`SP`), base pointer (`BP`), instruction pointer (`IP`), a heap region, and an interactive breakpoint debugger (`BRKP` instruction).

---

## XFS Interface (`xfs-interface/`)

External interface for managing the XSM virtual disk (`disk.xfs`).

```bash
cd xfs-interface && make
./xfs-interface
```

Common commands:

```
fdisk                       # format the virtual disk
load --library library.lib  # load the standard library
load --exec output.xsm      # load a compiled program
ls                          # list files on disk
```

---

## SPL Compiler (`spl/`)

Compiles System Programmer's Language to XSM assembly. Used for OS-level routines such as the bootstrap, exception handler, and interrupt handlers.

```bash
cd spl && make
./spl progs/os_startup.spl
```

Pre-compiled programs in `spl/progs/`: `os_startup`, `exhandler`, `halt`, `read`, `write`.

---

## End-to-End Example (Stage 8)

```bash
# Step 1: Compile an eXpL program
cd workdir/stage8
bash build.sh               # emits output.xsm, then resolves labels to final.xsm

# Step 2: Load onto the virtual disk
cd ../../xfs-interface
./xfs-interface
# > fdisk
# > load --library ../library.lib
# > load --exec ../workdir/stage8/final.xsm

# Step 3: Run in the simulator
cd ../xsm_dev
./xsm -l ../library.lib -e ../workdir/stage8/final.xsm
```

---

## Language Feature Progression

| Stage | Features Added |
|-------|---------------|
| 0 | Lex/Yacc fundamentals |
| 1 | Expressions, AST |
| 2 | Variables, assignments, interpretation |
| 3 | Control flow (`if`, `while`), XSM code generation |
| 4 | Declarations, symbol table, global variables |
| 5 | Functions, local variables, recursion |
| 6 | User-defined types, heap allocation, pointers |
| 7 | Classes, objects, methods, `self` |
| 8 | Inheritance, method overriding, dynamic dispatch |

---

## References

- [eXpL Language Specification](https://silcnitc.github.io/expl.html)
- [Compiler Design Lab — NITC/SILCNITC](https://silcnitc.github.io/)

---

Built as part of the Compiler Design Laboratory course.
Language: C, Flex/Lex, Bison/Yacc — Target: XSM Virtual Machine
