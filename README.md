# Syntax

```
program :=
     <function \n>+ (NOTE: angular braces are used for grouping)

function :=
     type ident() stmtblock
     type ident(type ident <, type ident>*) stmtblock

type :=
  bool
  int
  array
  void

stmtblock :=
     {\n <stmt \n>* }

stmt :=
     stmtblock
     type ident <, ident>*
     array ident\[expr\] <, ident\[expr\]>*
     print(<string|expr> <, <string|expr> >*
     if (expr) stmtblock [else stmtblock]
     while (expr) stmtblock
     for (ident : expr) stmtblock
     ident := expr
     ident\[expr\] := expr
     expr
     ;[^\n]*
     return [expr]
     abort(<string|expr> <, <string|expr> >*

expr :=
     true
     false
     -?[0-9]+
     ident
     (expr ? expr : expr)
     sizeof(ident)
     input()
     ident\[expr\]
     ident()
     ident(expr <, expr>*)
     (expr binop expr)
     (unaryop expr)

binop :=
     + - * ^ / % & | == != > >= < <=
    (caret is exponentiation, we have no xor operator)

unaryop :=
    - !
    (bang is Boolean not)

string :=
    " ('\\' ["nt\\] | [^\\\n"])*  "

ident :=
     [a-zA-Z_][a-zA-Z0-9_]*

```

# Building

Create a buid directory for yourself and do:

`cmake ..` (assuming you created builddir in the root directory)

`make`

It will create binary called `lilang`. You can check for various options with `lilang --help`

Files that should be parsed by this compiler are in passing-tests/ and files that should not be parsed are in failing-tests/ directory.

You can do:

`ctest` to run the tests. 

## Compiling and executing .lil files

Say you have a file, `lil.lil`. You compile it to LLVM IR like follows:

`./lilang lil.lil --print-ir=a.bc`

The file `a.bc` now contains the LLVM IR in textual form. If you want to execute it, you need to link it with the runtime file as follows:

`clang runtime.c a.bc -o file.out`

Then execute the file.out and see the live action.
