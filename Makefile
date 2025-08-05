all: sender listener

build-folder = ./build
obj = $(build-folder)/simple-lexer.o

sender: sender.c
	gcc -g sender.c -o $(build-folder)/sender

listener: listener.c simple-lexer.o
	gcc -g listener.c $(obj) -o $(build-folder)/listener

simple-lexer.o: simple-lexer.c simple-lexer.h
	gcc -g -c simple-lexer.c -o $(build-folder)/simple-lexer.o

test: simple-lexer lexer-tests.c
	gcc lexer-tests.c $(obj) -g -o  $(build-folder)/tests &&\
	$(build-folder)/tests

clean:
	rm -rf $(build-folder)/*