parser: lex.yy.c parser.cpp printvisitor.cpp baseast.cpp baseast.hpp visitor.hpp
	g++ -std=c++14 -g lex.yy.c parser.cpp printvisitor.cpp baseast.cpp -o parser

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
	@echo -e "Testing passing-tests (Expected outcome: OK)\n";
	@for file in ${GOOD_FILES}; do \
		./parser $${file} 2> /dev/null || (echo -e "${RED}ERR: OK expected${NC} for $${file}, got NOK") \
	done || true;
	
	@echo -e "Testing failing-tests (Expected outcome: NOK)\n";
	@for file in ${BAD_FILES}; do \
		./parser $${file} 2> /dev/null && (echo -e "${RED}ERR: NOK expected${NC} for $${file}, got OK"); \
	done || true
