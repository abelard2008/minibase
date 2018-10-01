GLOBAL_DB_DIR = ../..
include ../../Makefile.preamble
include Makefile.preamble

SUBDIR_PREFIX = PARSER
include ../../subdir.make


y.tab.C y.tab.h : sql.y
	$(YACC) sql.y; \
	mv y.tab.c y.tab.C 

lex.yy.C : sql.l
	$(LEX) sql.l; \
	mv lex.yy.c lex.yy.C 

OTHERGARBAGE = y.tab.* lex.yy.*
