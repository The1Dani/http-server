#ifndef PARSE_HTML
#define PARSE_HTML

#include "external/map.h"
#include <assert.h>
#include <fcntl.h>
#include "arena.h"

#define SUPPORTED_PROTOCOL "HTTP/1.1"
#define HTTP_STATUS_OK "200"

typedef struct {
    map_t *fields;
    Da_str *keys;
} Fields;

/*When fields destroyed the whole structure is deallocated*/
typedef struct {

    const char *method;
    const char *uri;
    const char *protocol;
    /*Req_arena lives in fields*/
    map_a fields;

    const char *body;

} Req;

typedef struct {

    const char *protocol;
    unsigned int status_code;
    char *status_name;
    map_a fields;
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

char *get_file_path(const char *uri, map_a m);

// TODO make so that you dont use math lib
size_t construct_response(Resp *r, void *buf);

// TODO!
void set_status_code(Resp *r, int status_code, char *code_name);

// TODO!
Fields fields_new();
#define SET_FIELDS_NEW(r) r.fields = a_map_new()

// TODO!
void fields_append(Fields *fs, char *key, char *val);
#define RESP_FIELD_APPEND(r, key, val)                                         \
    a_map_set(r->fields, key, val)

// TODO!
void fields_destroy(Fields fields);

// TODO!
void fields_destroy_vals(Fields fields);

// TODO!
void dump_file_to_body(Resp *r, const char *f_name);


char *get_file_name(char *url);

enum Mime_Type get_mime_type(char *url);

#endif
