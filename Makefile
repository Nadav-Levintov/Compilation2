all: clean
	flex -o lex.yy.c lexer.lex
	g++ -g -o hw2 *.cpp *.c

clean:
	rm -f lex.yy.c
	rm -f hw2


