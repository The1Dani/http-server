#include "parse_http.h"
#include "da.h"
#include "external/map.h"
#include "simple_lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

int is_end(const char *s) {
    if (strlen(s) != 1) return 0;
    if (s[0] == '\xd') return 1;
    return 0;
}

bool parse_field(const char *line, map_t *m, Da_str *da) {

    if (da == NULL) {
        printf("DA Pointer is NULL!\n");
        return false;
    }

    if (da->cap == 0 && da->size == 0) {
        *da = da_str_new();
    }

    char **tokens = NULL;
    int n_tokens = get_words_from_delim(line, ":\r", &tokens);
    char *val = NULL;
    if (n_tokens <= 1) {
        printf ("n_tokens: %d | expected >= 2\n", n_tokens);
        printf ("Failed on string `%s' \n", line);
        exit(1);
        return false;
    }
    if (n_tokens > 2) {
        // See here
        concat_list(&tokens[1], n_tokens - 1, &val, ":"); //TODO Free the unused tokens in function    
    } else val = tokens[1]; 
    char *key = tokens[0];
    da_str_push(da, key);

    if (map_set(m, key, val) == -1) {
        printf ("map_set did fail\n");
        return false;
    }

    return true;
}

Req *http_parse_req(char **lines, size_t line_count) {

    const char *idents = lines[0];
    map_t *fields = map_new(DEFAULT_SIZE);
    Req *req = calloc(1, sizeof(Req));

    /*Parse Idents*/
    //TODO
    char **list = NULL;
    int n_words = get_words(idents, &list);
    if (n_words != 3) {
        printf("Expected 3 tokens got %d :\n", n_words);
        for (int i = 1; i <= n_words; i++) {
            printf("Token %02d: %s\n", i, list[i-1]);
        }
        exit(1);
    }
    req->method = list[0];
    req->uri = list[1];
    req->protocol = list[2];

    if (strcmp(req->protocol, SUPPORTED_PROTOCOL)) {
        printf("Unsupported Protocol %s\n", req->protocol);
    }

    // //LOG
    // printf("The idents parsed succsess\n");
    // printf("METHOD: %s\nURI: %s\nPROTOCOL: %s\n", req->method, req->uri, req->protocol);

    /*Parse Fields*/
    Da_str *keys = calloc(1, sizeof(Da_str));

    size_t i = 1;
    for (; i < line_count; i++) {
        if (is_end(lines[i])) {
            break;
        }
        if (!parse_field(lines[i], fields, keys)) return NULL;
    }

    /*Parse Body*/
    char *body = NULL;
    concat_list(lines + i + 1, line_count - i - 1, &body, " ");

    req->body = body;
    req->fields = (struct fields) {
        .fields = fields,
        .keys = keys,
    };

    return req;
}