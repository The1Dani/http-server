#include <stdio.h>
#include <string.h>
#include "../parse_http.h"
#include "../da.h"

int main() {

    char *buf;

    Da_str keys = da_str_new();
    map_t *m = map_new(DEFAULT_SIZE);

    da_str_push(&keys, "Content-Type");
    map_set(m, "Content-Type", strdup("text/html; charset=UTF-8"));

    Resp r = {
        .protocol = SUPPORTED_PROTOCOL,
        .status_code = 200,
        .status_name = "OK",
        .body = {
            .body = "Body",
            .body_len = 4,
        },
        .fields = {
            .fields = m,
            .keys = &keys,
        },
    };

    size_t len = construct_response(&r, (void *)(&buf));

    buf = realloc(buf, len + 1);
    buf[len] = '\0';

    puts(buf);

    return 0;
}