all: build-nob

clean:
	rm -rf ./build/* && rm -rf nob nob.old

build-nob:
	gcc -o nob nob.c

count:
	cloc --exclude-dir='external,refs' --exclude-content='nob.h' .

valgrind: all
	valgrind -s --leak-check=full ./build/listener