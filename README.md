# Building

Create a buid directory for yourself and do:

`cmake ..` (assuming you created builddir in the root directory)

`make`

It will create binary called `lilang`. You can check for various options with `lilang --help`

Files that should be parsed by this compiler are in passing-tests/ and files that should not be parsed are in failing-tests/ directory.

You can do:

`ctest` to run the tests. Tests assume that passing-tests/ and failing-tests/ directory are one level up than the root directory (this is for my own convenience)



