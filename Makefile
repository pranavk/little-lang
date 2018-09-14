parser: lex.yy.c parser.cpp
	g++ -g lex.yy.c parser.cpp -o parser

lexer: lex.yy.c
	gcc lex.yy.c -lfl -o lexer

lex.yy.c: lexer.l
	flex lexer.l
.PHONY:
clean:
	rm -rf lex.yy.c parser lexer

GOOD_FILES:= $(wildcard ../passing-tests/*.lil)
BAD_FILES:= $(wildcard ../failing-tests/*.lil)

test-correct: parser
	for file in ${GOOD_FILES}; do \
		./parser $${file}; \
	done

test-error: parser
	for file in ${BAD_FILES}; do \
		./parser $${file} && (echo "ERROR: Fail expected but it passed."); \
	done
