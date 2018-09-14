# Building

Just do:

`make`

Files that should be parsed by this compiler are in passing-tests/ and files that should not be parsed are in failing-tests/ directory.

You can do

`./parser filenametoparse`

to run the parser on any file and check its result.

There is a `make check` rule too that will parse all the files in ../passing-tests/ and ../failing-tests/ directory. So make sure that these directories exist.

Refer to this page for language specifications:
https://github.com/regehr/tiny-language
