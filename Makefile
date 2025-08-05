all: sender listener

obj = simple-lexer.o


sender: sender.c
	gcc -g sender.c -o sender

listener: listener.c
	gcc -g listener.c $(obj) -o listener

simple-lexer: simple-lexer.c simple-lexer.h
	gcc -g -c simple-lexer.c -o simple-lexer.o

test: simple-lexer lexer-tests.c
	gcc lexer-tests.c $(obj) -g -o  tests &&\
	./tests
