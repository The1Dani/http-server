all: sender listener

build_folder = ./build
obj = simple_lexer map da parse_http
obj_folder = $(foreach f,$(obj),$(build_folder)/$(f).o)

sender: sender.c
	gcc -g sender.c -o $(build_folder)/$@

listener: listener.c $(obj_folder)
	gcc -g listener.c $(obj_folder) -o $(build_folder)/listener

test: lexer-tests.c $(obj_folder)
	gcc lexer-tests.c $(obj_folder) -g -o  $(build_folder)/tests &&\
	$(build_folder)/tests

$(build_folder)/%.o: %.c %.h
	gcc -g -c $< -o $@

clean:
	rm -rf $(build_folder)/*
