#include "req_handle.h"
#include "arena.h"
#include "external/map.h"
#include "parse_http.h"
#include "simple_lexer.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mime.h"

// #define ROOT_FOLDER "/home/dani/faf/http-server/root/"

char *ROOT_FOLDER = "/home/dani/faf/http-server/root/";

struct _node {
    char *key;
    char *val;
};

void print_node(struct _node n) {
    printf("Node || %s: %s \n", n.key, (char *)n.val);
}

void print_map(map_a fields) {

    map_t *m = fields.map;
    Da_str *keys = fields.keys;

    for (size_t i = 0; i < keys->size; i++) {
        char *key = keys->list[i];
        print_node((struct _node){.key = key, .val = (char *)map_get(m, key)});
    }
}

char *get_file_path_str(char *file_name) {

    /*Check if the root dir has '/' at the end if not put it*/
    char *result = NULL;
    Arena *arena = arena_new(KILOBYTE / 8);
    Da_str da = da_str_new(arena);

    da_str_push(&da, a_strdup(arena, ROOT_FOLDER));

    if (ROOT_FOLDER[strlen(ROOT_FOLDER) - 1] != '/') {
        da_str_push(&da, a_strdup(arena, "/"));
    }

    da_str_push(&da, a_strdup(arena, file_name));

    result = concat_list(da.list, da.size, "");

    arena_free(arena);
    return result;
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

    map_a q_params = a_map_new();
    char *url = get_file_path(uri, q_params);
    printf("URL: '%s`\n", url);
    free(url);

    print_map(req->fields);

    if (q_params.keys->size != 0)
        printf("Q_PARAMS\n");

    print_map(q_params);

    /*
     * Create Response
     */

    RESP_FIELD_APPEND(resp, "key", "val");

    // content-type: image/jpeg
    //  RESP_FIELD_APPEND(resp, "content-type", "image/jpeg");
    // content-type: text/html; charset=UTF-8

    char *file_path = get_file_path_str("index.html");

    dump_file_to_body(resp, file_path);
    free(file_path);

    if (resp->body.body_len < 0) {
        set_status_code(resp, 404, "NOT-FOUND");
    } else {
        RESP_FIELD_APPEND(resp, "content-type", "text/html; charset=UTF-8");
        set_status_code(resp, 200, "OK");
    }

    a_map_free(q_params);

    /*^ The code above is the same hopefully ^*/
    // map_ffree(q_params, key_list.list, key_list.size);
    // free_str_list(key_list.list, key_list.size); //< simple-lexer.h
    // da_str_destroy(key_list);
}

void serve_dir_handler(Req *req, Resp *resp) {

    map_a uri_fields = a_map_new();

    char *req_file_or_dir = get_file_path(req->uri, uri_fields);

    if (!strcmp(req_file_or_dir, "/")) {
        free(req_file_or_dir);
        req_file_or_dir = strdup("/index.html");
    }

    char *abs_path = strdup(ROOT_FOLDER);
    concat(&abs_path, req_file_or_dir);

    dump_file_to_body(resp, abs_path);

    switch (resp->body.body_len) {
    case -1:
        set_status_code(resp, 404, "NOT-FOUND");
        break;
    case -2:
        char *body = malloc(strlen(req_file_or_dir) + 32);
        sprintf(body, "<h1>URL: %s </h1>", req_file_or_dir);
        resp->body.body = body;
        resp->body.body_len = strlen(body);

        RESP_FIELD_APPEND(resp, "content-type", "text/html; charset=UTF-8");
        set_status_code(resp, 200, "OK");
        dump_dir_list_html(resp, req_file_or_dir, ROOT_FOLDER);
        break;
    default:
        // enum Mime_Type mime_type = get_mime_type(req_file_or_dir);
        const char *mime_type = get_mime_from_filename(req_file_or_dir, mime_info);
        RESP_FIELD_APPEND(resp, "Content-Type", (void *)mime_type);
        printf("GET %s\n", req_file_or_dir);
        set_status_code(resp, 200, "OK");
        break;

    }

    free(abs_path);
    a_map_free(uri_fields);
    free(req_file_or_dir);
}

void req_handler(Req *req, Resp *resp) { serve_dir_handler(req, resp); }
