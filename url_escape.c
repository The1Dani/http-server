#include "url_escape.h"
#include "simple_lexer.h"
#include <string.h>

char *go_next(char *s, char *tok) {

    char *t = strchr(s, '%');
    if (t == NULL)
        return NULL;

    memcpy(tok, t, sizeof(char) * 3);
    str_shift_right(t, 2);
    return t;
}

char *decode_url(char *_s) {

    char *s = strdup(_s);

    char *nxt = NULL;
    char *tok = malloc(sizeof(char) * 3);

    nxt = go_next(s, tok);

    free(tok);
    return NULL;
}