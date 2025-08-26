#ifndef PARSE_HTML
#define PARSE_HTML

#include "da.h"
#include "external/map.h"
#include <assert.h>
#include <fcntl.h>

#define SUPPORTED_PROTOCOL "HTTP/1.1"
#define HTTP_STATUS_OK "200"
#define RESP_OK                                                                \
    SUPPORTED_PROTOCOL                                                         \
    " " HTTP_STATUS_OK " OK\r\n"                                               \
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"                           \
    "<h1>elifusu çk sevyrm :33</h1>"

typedef struct {
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

enum Mime_Type {
    TEXT_PLAIN,              //"txt"
    TEXT_CSS,                // "css"
    TEXT_CSV,                // "csv"
    TEXT_HTML,               // "html"
    TEXT_JS,                 // "js"
    TEXT_XML,                // "xml"
    IMAGE_AVIF,              // "avif"
    IMAGE_JPEG,              // "jpeg", "jpg"
    IMAGE_PNG,               // "png", "ico"
    IMAGE_SVG,               // "svg"
    AUDIO_MPEG,              // "mpg", "mpeg"
    APPLICATION_JSON,        // "json"
    APPLICATION_PDF,         // "pdf"
    APPLICATION_OCTETSTREAM, // ""
    TEXT_MARKDOWN,           // "md"
};


Req *http_parse_req(char **lines, size_t line_count);

void req_free(Req *req);

char *get_file_path(const char *uri, map_t *m, Da_str *key_list);

// TODO make so that you dont use math lib
size_t construct_response(Resp *r, void *buf);

// TODO!
void set_status_code(Resp *r, int status_code, char *code_name);

// TODO!
Fields fields_new();
#define SET_FIELDS_NEW(r) r.fields = fields_new()

// TODO!
void fields_append(Fields *fs, char *key, char *val);
#define RESP_FIELD_APPEND(r, key, val)                                         \
    assert(&r->fields != NULL);                                                \
    fields_append(&r->fields, key, val)

// TODO!
void fields_destroy(Fields fields);

// TODO!
void fields_destroy_vals(Fields fields);

// TODO!
void dump_file_to_body(Resp *r, const char *f_name);

Resp resp_new();

char *get_file_name(char *url);

enum Mime_Type get_mime_type(char *url);

#endif
