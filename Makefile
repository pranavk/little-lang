lexer: lex.yy.c
	gcc lex.yy.c -lfl -o lexer

lex.yy.c: lexer.l
	flex lexer.l

parser: lex.yy.c parser.cpp
	g++ -g lex.yy.c parser.cpp -o parser

.PHONY:
test-correct: parser correct1.lil correct2.lil correct3.lil
	./parser correct1.lil && ./parser correct2.lil && ./parser correct3.lil

test-error: parser err1.lil err2.lil err3.lil
	./parser err1.lil && ./parser err2.lil && ./parser err3.lil
