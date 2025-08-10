all: __build_folder sender listener

build_folder = ./build
obj = simple_lexer da parse_http
externals_folder = ./external
externals = map
obj_folder = $(foreach f,$(obj),$(build_folder)/$(f).o)
external_obj_folder = $(foreach f,$(externals),$(build_folder)/$(f)_ext.o)
all_obj = $(external_obj_folder) $(obj_folder)

__build_folder:
	@mkdir -p './build' 

sender: sender.c
	gcc -g sender.c -o $(build_folder)/$@

listener: listener.c $(all_obj)
	gcc -g listener.c $(all_obj) -o $(build_folder)/listener

test: lexer-tests.c $(obj_folder)
	gcc lexer-tests.c $(obj_folder) -g -o  $(build_folder)/tests &&\
	$(build_folder)/tests


$(build_folder)/%_ext.o: $(externals_folder)/%.c $(externals_folder)/%.h
	gcc -g -c $< -o $@

$(build_folder)/%.o: %.c %.h
	gcc -g -c $< -o $@

clean:
	rm -rf $(build_folder)/*

count:
	cloc --exclude-dir='external' .

valgrind: all
	valgrind -s --leak-check=full ./build/listener