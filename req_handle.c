#include "da.h"
#include "external/map.h"
#include "parse_http.h"
#include <stddef.h>

struct _node {
    char *key;
    char *val;
};

void print_node(struct _node n) {
    printf("Node || key = %s || val = %s \n", n.key, (char *)n.val);
}

void print_map(struct fields fields) {

    map_t *m = fields.fields;
    Da_str *keys = fields.keys;

    for (size_t i = 0; i < keys->size; i++) {
        char *key = keys->list[i];
        print_node((struct _node){.key = key, .val = (char *)map_get(m, key)});
    }
}

void log_http_req(Req *req) {

    const char *method = req->method;
    const char *uri = req->uri;
    const char *proto = req->protocol;
    const char *body = req->body;

    printf("METHOD: %s\n"
           "URI: %s\n"
           "PROTOCOL: %s\n"
           "BODY: %s\n\n",
           method, uri, proto, body);

    map_t *q_params = map_new(DEFAULT_SIZE);
    Da_str key_list;
    char *url = get_file_path(uri, q_params, &key_list);
    printf("URL: '%s`\n", url);

    print_map(req->fields);
    printf("Q_PARAMS\n");
    print_map((struct fields){.fields = q_params, .keys = &key_list});
}

void req_handler(Req *req) { log_http_req(req); }