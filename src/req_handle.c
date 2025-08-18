#include "da.h"
#include "external/map.h"
#include "parse_http.h"
#include "simple_lexer.h"
#include <stddef.h>
#include <string.h>

struct _node {
    char *key;
    char *val;
};

void print_node(struct _node n) {
    printf("Node || %s: %s \n", n.key, (char *)n.val);
}

void print_map(Fields fields) {

    map_t *m = fields.fields;
    Da_str *keys = fields.keys;

    for (size_t i = 0; i < keys->size; i++) {
        char *key = keys->list[i];
        print_node((struct _node){.key = key, .val = (char *)map_get(m, key)});
    }
}

void log_http_req(Req *req, Resp *resp) {

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

    if (key_list.size != 0)
        printf("Q_PARAMS\n");

    print_map((Fields){.fields = q_params, .keys = &key_list});

    free(url);

    for (size_t i = 0; i < key_list.size; i++) {
        char *key = key_list.list[i];
        free(map_get(q_params, key));
    }

    Da_str *resp_keys = malloc(sizeof(Da_str));
    *resp_keys = da_str_new();
    map_t *m = map_new(DEFAULT_SIZE);

    da_str_push(resp_keys, strdup("key"));
    map_set(m, "key", strdup("val"));

    //content-type: image/jpeg
    da_str_push(resp_keys, strdup("content-type"));
    map_set(m, "content-type", strdup("image/jpeg"));

    char *buf;
    size_t size = get_file_content("/home/dani/faf/http-server/root/img.jpg", &buf);

    resp->fields.keys = resp_keys;
    resp->fields.fields = m;

    resp->status_code = 200;
    resp->status_name = "OK";

    resp->body.body = buf;
    resp->body.body_len = size;

    map_ffree(q_params, key_list.list, key_list.size);
    free_str_list(key_list.list, key_list.size);
    da_str_destroy(key_list);
}

void req_handler(Req *req, Resp *resp) { log_http_req(req, resp); }