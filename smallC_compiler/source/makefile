LEX=lex
YACC=yacc
CC=g++ -std=c++11

scc:lex.yy.o y.tab.o
	$(CC) y.tab.c -g -ly -ll -o scc 2> error.err
lex.yy.o:lex.yy.c y.tab.h
	$(CC) -c lex.yy.c 2> error.err

y.tab.o:y.tab.c node.h tree.h semantics.h intermediate.h optimize.h codegen.h
	$(CC) -c y.tab.c 2> error.err

y.tab.c y.tab.h:smallc.y def.h node.h tree.h semantics.h intermediate.h optimize.h codegen.h
	$(YACC) smallc.y -v -d

lex.yy.c:smallc.l def.h node.h tree.h semantics.h intermediate.h optimize.h codegen.h
	$(LEX) smallc.l

clean:
	@rm -f *.o *.c *.tab.* *.err *.output
