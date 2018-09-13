parser: lex.yy.c parser.cpp
	g++ -g lex.yy.c parser.cpp -o parser

lexer: lex.yy.c
	gcc lex.yy.c -lfl -o lexer

lex.yy.c: lexer.l
	flex lexer.l
.PHONY:
clean:
	rm -rf lex.yy.c parser lexer

test-correct: parser correct1.lil correct2.lil correct3.lil
	./parser correct1.lil && ./parser correct2.lil && ./parser correct3.lil

test-error: parser err1.lil err2.lil err3.lil
	-./parser err1.lil && (echo "ERROR: Fail expected but it passed.")
	-./parser err2.lil && (echo "ERROR: Fail expected but it passed.")
	-./parser err3.lil && (echo "ERROR: fail expected but it passed.")
