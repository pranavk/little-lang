lexer: lex.yy.c
	gcc lex.yy.c -lfl -o lexer

lex.yy.c: lexer.l
	flex lexer.l

.PHONY:
lex-test: lexer sample.lil
	./lexer < sample.lil | grep -E -e '' -e ERR
