#include <stdio.h>
#include <string.h>
#include "../parse_http.h"
#include "../arena.h"

#define ARENA_IMPLEMENTATION
#include "../arena.h"


int main() {

    char *buf;

    Arena *keys_arena = arena_new(0);
    Da_str keys = da_str_new(keys_arena);
    map_a m = a_map_new();

    da_str_push(&keys, "Content-Type");
    a_map_set(m, "Content-Type", "text/html; charset=UTF-8");

    Resp r = {
        .protocol = SUPPORTED_PROTOCOL,
        .status_code = 200,
        .status_name = "OK",
        .body = {
            .body = "Body",
            .body_len = 4,
        },
        .fields = m,
    };

    size_t len = construct_response(&r, (void *)(&buf));

    buf = realloc(buf, len + 1);
    buf[len] = '\0';

    puts(buf);

    a_map_free(m);

    return 0;
}