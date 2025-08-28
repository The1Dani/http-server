#include "../simple_lexer.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define ARENA_IMPLEMENTATION
#include "../arena.h"


#define ROOT "/home/dani/faf/http-server/src"

int main(void) {

    const char*file = ROOT"/index.html";

    char *f_content = NULL;
    int f_size = get_file_content(file, &f_content);
    assert(f_size > 0);
    f_content = realloc(f_content, f_size + 1);
    f_content[f_size] = '\0';

    puts(f_content);
    printf("f_size = %d\n", f_size);

    free(f_content);
    return 0;

}