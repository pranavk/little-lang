# Building

Create a buid directory for yourself and do:

`cmake ..` (assuming you created builddir in the root directory)

`make`

It will create binary called `lilang`. You can check for various options with `lilang --help`

Files that should be parsed by this compiler are in passing-tests/ and files that should not be parsed are in failing-tests/ directory.

You can do:

`ctest` to run the tests. Tests assume that passing-tests/ and failing-tests/ directory are one level up than the root directory (this is for my own convenience)

## Compiling and executing .lil files

You have a file, `lil.lil`. You compile it to LLVM IR like follows:

`./lilang lil.lil --print-ir=a.bc`

The file `a.bc` now contains the LLVM IR in textual form. If you want to execute it, you need to link it with the runtime file as follows:

`clang runtime.c a.bc -o file.out`

Then execute the file.out and see the live action.
