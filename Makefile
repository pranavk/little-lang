lexer: lex.yy.c
	gcc lex.yy.c -lfl -o lexer

lex.yy.c: lexer.l
	flex lexer.l

parser: lex.yy.c parser.cpp
	g++ -g lex.yy.c parser.cpp -o parser

.PHONY:
test-sample: parser sample.lil
	./parser sample.lil

test-lil: parser lil.lil
	./parser lil.lil
