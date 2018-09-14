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
RED=\033[0;31m
NC=\033[0m

check: parser
	for file in ${GOOD_FILES}; do \
		./parser $${file} 2> /dev/null || (echo -e "${RED}ERR: OK expected${NC} for $${file}, got NOK") \
	done || true;

	for file in ${BAD_FILES}; do \
		./parser $${file} 2> /dev/null && (echo -e "${RED}ERR: NOK expected${NC} for $${file}, got OK"); \
	done || true
