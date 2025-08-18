#include "parse_http.h"
#include "da.h"
#include "external/map.h"
#include "simple_lexer.h"
#include "url_escape.h"
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_end(const char *s) {
    if (strlen(s) != 1)
        return 0;
    if (s[0] == '\xd')
        return 1;
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

    bool is_alloced_sep = false;

    if (n_tokens <= 1) {
        printf("n_tokens: %d | expected >= 2\n", n_tokens);
        printf("Failed on string `%s' \n", line);
        exit(1);
        return false;
    }
    if (n_tokens > 2) {
        // See here
        val = concat_list(&tokens[1], n_tokens - 1, ":");
        is_alloced_sep = true;
    } else
        val = tokens[1];
    char *key = tokens[0];
    da_str_push(da, strdup(key));

    str_shift_right(val, 1);

    if (map_set(m, key, strdup(val)) == -1) {
        printf("map_set did fail\n");
        return false;
    }

    if (is_alloced_sep)
        free(val);

    free_str_list(tokens, n_tokens);
    free(tokens);

    return true;
}

Req *http_parse_req(char **lines, size_t line_count) {

    const char *idents = lines[0];
    map_t *fields = map_new(DEFAULT_SIZE);
    Req *req = calloc(1, sizeof(Req));

    /*Parse Idents*/
    char **list = NULL;
    int n_words = get_words(idents, &list);
    if (n_words == 0)
        return NULL;
    if (n_words != 3) {
        printf("Expected 3 tokens got %d :\n", n_words);
        for (int i = 1; i <= n_words; i++) {
            printf("Token %02d: %s\n", i, list[i - 1]);
        }
        exit(1);
    }
    req->method = strdup(list[0]);
    req->uri = strdup(list[1]);
    req->protocol = strdup(list[2]);

    free_str_list(list, n_words);
    free(list);

    if (strcmp(req->protocol, SUPPORTED_PROTOCOL)) {
        printf("Unsupported Protocol %s\n", req->protocol);
    }

    // //LOG
    // printf("The idents parsed succsess\n");
    // printf("METHOD: %s\nURI: %s\nPROTOCOL: %s\n", req->method, req->uri,
    // req->protocol);

    /*Parse Fields*/
    Da_str *keys = calloc(1, sizeof(Da_str));

    size_t i = 1;
    for (; i < line_count; i++) {
        if (is_end(lines[i])) {
            break;
        }
        if (!parse_field(lines[i], fields, keys))
            return NULL;
    }

    /*Parse Body*/
    char *body = NULL;
    body = concat_list(lines + i + 1, line_count - i - 1, " ");

    req->body = body;
    req->fields = (Fields){
        .fields = fields,
        .keys = keys,
    };

    return req;
}

void req_free(Req *req) {

    free((void *)req->method);
    free((void *)req->uri);
    free((void *)req->protocol);
    free((void *)req->body);

    for (size_t i = 0; i < req->fields.keys->size; i++) {
        char *key = req->fields.keys->list[i];
        void *val = map_get(req->fields.fields, key);
        free(val);
    }

    map_ffree(req->fields.fields, req->fields.keys->list,
              req->fields.keys->size);
    free_str_list(req->fields.keys->list, req->fields.keys->size);
    da_str_destroy(*req->fields.keys);
    free(req->fields.keys);
    free(req);
}

/* Replaces '?' with '\0' */
int get_querry_params(char *uri, map_t *m, Da_str *key_list) {

    char *q_params = NULL;

    if (!(q_params = strchr(uri, '?'))) {
        return 0;
    }

    q_params[0] = '\0';
    q_params += 1; // GET TO THE PARAMETERS "?asdasd" -> "asdasd"

    char **param_pairs = NULL;
    int n_params = get_words_from_delim(q_params, "&", &param_pairs);

    if (param_pairs == NULL) {
        perror("PARAM_PAIRS IS NULL");
        // Add frees here
        return -1;
    }

    /*We dont free the vals bc map stores raw pointers*/
    /*The keys are stored as copied strings*/
    for (int i = 0; i < n_params; i++) {
        char **param_pair = NULL;
        char *key = NULL;
        char *val = NULL;

        int n_pair = get_words_from_delim(param_pairs[i], "=", &param_pair);

        if (n_pair <= 0) { // DONT KNOW WHAT
            continue;
        } else if (n_pair == 1) {
            continue;
        } else if (n_pair > 2) {
            key = param_pair[0];
            val = concat_list(param_pair + 1, n_pair - 1, "=");
            free_str_list(param_pair + 1, n_pair - 1);
        } else {
            key = param_pair[0];
            val = param_pair[1];
        }

        char *decoded_key = decode_url(key);
        char *decoded_val = decode_url(val);
        free(key);
        free(val);
        free(param_pair);

        map_set(m, decoded_key, decoded_val);
        da_str_push(key_list, decoded_key);
    }

    free_str_list(param_pairs, n_params);
    free(param_pairs);

    return n_params;
}

char *get_file_path(const char *_uri, map_t *m, Da_str *_key_list) {

    if (_uri == NULL) {
        return NULL;
    }

    char *uri = strdup(_uri);
    Da_str key_list = da_str_new();

    get_querry_params(uri, m, &key_list);

    char *url = decode_url(uri);
    char *s_url = path_sanitize(url);
    free(url);

    free(uri);
    *_key_list = key_list;
    return s_url;
}

size_t calc_response_size(Resp r) {

    int n_proto, n_scode, n_sname;
    int n_keys = 0, n_vals = 0;
    int n_extra = 0;
    off_t n_body;

    n_proto = strlen(r.protocol);
    n_scode = (int)floor(log10(r.status_code)) + 1;
    n_sname = strlen(r.status_name);

    n_extra = 4;

    for (size_t i = 0; i < r.fields.keys->size; i++) {
        char *key = r.fields.keys->list[i];
        char *val = map_get(r.fields.fields, key);

        assert(val != NULL);
        assert(key != NULL);

        n_keys += strlen(key);
        n_vals +=
            strlen(val); // Somehow maybe it can be null but i cannot be sure;
        n_extra += 4;
    }

    n_extra += 2; // Empty_Field;
    n_body = r.body.body_len;

    size_t total = n_proto + n_scode + n_sname + n_keys + n_vals + n_extra +
                   (size_t)n_body;

    return total;
}

size_t get_ident_line_len(Resp r) {
    // proto st_code st_name\r\n
    return strlen(r.protocol) + (size_t)floor(log10(r.status_code)) + 1 +
           strlen(r.status_name) + 4;
}

size_t get_field_line_len(Resp r, char *key) {

    assert(key != NULL);
    char *val = map_get(r.fields.fields, key);
    val = val != NULL ? val : "";

    // key: val\r\n
    return strlen(key) + strlen(val) + 4;
}

size_t construct_response(Resp *r, void *_buf) {

    //! Add Content-Length: xx if needed
    {
        if (r->body.body != NULL && r->body.body_len > 0) {
            char *content_len = "Content-Length";
            da_str_push(r->fields.keys, content_len);

            size_t c_len = r->body.body_len;
            char *num = malloc((size_t)floor(log10(c_len)) + 2);
            sprintf(num, "%zu", c_len);
            map_set(r->fields.fields, content_len, num);
        }
    }

    size_t off = 0;
    size_t r_size = calc_response_size(*r);
    *(char **)(_buf) = malloc(r_size);
    char **buf = (char **)(_buf);

    assert(*buf != NULL);

    {
        size_t n_ident_line = get_ident_line_len(*r);
        char ident_line[n_ident_line + 1];
        sprintf(ident_line, "%s %d %s\r\n", r->protocol, r->status_code,
                r->status_name);
        memcpy(*buf + off, ident_line, n_ident_line);
        off += n_ident_line;
    }

    {
        for (size_t i = 0; i < r->fields.keys->size; i++) {
            char *key = r->fields.keys->list[i];
            char *val = map_get(r->fields.fields, key);
            size_t n_field_line = get_field_line_len(*r, key);
            char field_line[n_field_line + 1];

            sprintf(field_line, "%s: %s\r\n", key, val);
            memcpy(*buf + off, field_line, n_field_line);
            off += n_field_line;
        }
    }

    /*Empty field line*/
    memcpy(*buf + off, "\r\n", 2);
    off += 2;

    {
        if (r->body.body != NULL && r->body.body_len > 0) {
            memcpy(*buf + off, r->body.body, r->body.body_len);
            off += r->body.body_len;
        }
    }
    assert(off == r_size);

    return r_size;
}