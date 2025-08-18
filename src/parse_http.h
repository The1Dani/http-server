#ifndef PARSE_HTML
#define PARSE_HTML

#include "da.h"
#include "external/map.h"
#include <fcntl.h>
#include <setjmp.h>
#include <stddef.h>

#define SUPPORTED_PROTOCOL "HTTP/1.1"
#define HTTP_STATUS_OK "200"
#define RESP_OK                                                                \
    SUPPORTED_PROTOCOL                                                         \
    " " HTTP_STATUS_OK " OK\r\n"                                               \
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"                           \
    "<h1>elifusu çk sevyrm :33</h1>"

typedef struct  {
    map_t *fields;
    Da_str *keys;
} Fields;

typedef struct {

    const char *method;
    const char *uri;
    const char *protocol;
    Fields fields;

    const char *body;

} Req;

typedef struct {

    const char *protocol;
    unsigned int status_code;
    char *status_name;
    Fields fields;
    struct {
        char *body;
        off_t body_len;
    } body;

} Resp;

Req *http_parse_req(char **lines, size_t line_count);

void req_free(Req *req);

char *get_file_path(const char *uri, map_t *m, Da_str *key_list);

size_t construct_response(Resp *r, void *buf);

//TODO
void set_status_code(Resp* r, int status_code, char *code_name);

//TODO
Fields fields_new(Resp *r);

//TODO
void fields_append(Fields fs, char *key, char *val);

//TODO
void fields_destroy(Fields fields);

//TODO
void fields_destroy_vals(Fields fields);

//TODO
void dump_file_to_body(Resp *r, const char *f_name);

#endif
