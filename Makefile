all: build-nob

clean:
	rm -rf ./build/* && rm -rf nob nob.old

build-nob:
	gcc -o nob nob.c

count:
	cloc --exclude-dir='external,refs,root,root-cubes' --exclude-content='nob.h' ./src

valgrind: all
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./listener