#include "parse_http.h"
#include "arena.h"
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

bool parse_field(const char *line, map_a map) {

    Arena *arena = arena_new(0);

    if (map.keys == NULL) {
        printf("DA Pointer is NULL!\n");
        return false;
    }

    assert(!(map.keys->cap == 0 && map.keys->size == 0));

    Da_str tokens = da_str_new(arena);
    get_words_from_delim(line, ":\r", &tokens);
    int n_tokens = tokens.size;
    char *val = NULL;

    if (n_tokens <= 1) {
        printf("n_tokens: %d | expected >= 2\n", n_tokens);
        printf("Failed on string `%s' \n", line);
        exit(1);
        return false;
    }
    if (n_tokens > 2) {
        // See here
        val = str_to_arena_ptr(arena,
                               concat_list(&tokens.list[1], n_tokens - 1, ":"));
    } else
        val = tokens.list[1];
    char *key = tokens.list[0];

    str_shift_right(val, 1);

    a_map_set(map, key, val);

    arena_free(arena);

    return true;
}

Req *http_parse_req(char **lines, size_t line_count) {

    Arena *arena = arena_new(0);

    const char *idents = lines[0];
    map_a fields = a_map_new();
    Arena *req_arena = fields.keys->arena;

    Req *req = calloc(1, sizeof(Req));

    /*Parse Idents*/
    Da_str words = da_str_new(arena);
    get_words(idents, &words);

    int n_words = words.size;
    if (n_words == 0) {
        arena_free(arena);
        return NULL;
    }
    if (n_words != 3) {
        printf("Expected 3 tokens got %d :\n", n_words);
        for (int i = 1; i <= n_words; i++) {
            printf("Token %02d: %s\n", i, words.list[i - 1]);
        }
        exit(1);
    }
    req->method = a_strdup(req_arena, words.list[0]);
    req->uri = a_strdup(req_arena, words.list[1]);
    req->protocol = a_strdup(req_arena, words.list[2]);

    if (strcmp(req->protocol, SUPPORTED_PROTOCOL)) {
        printf("Unsupported Protocol %s\n", req->protocol);
    }

    // //LOG
    // printf("The idents parsed succsess\n");
    // printf("METHOD: %s\nURI: %s\nPROTOCOL: %s\n", req->method, req->uri,
    // req->protocol);

    /*Parse Fields*/

    size_t i = 1;
    for (; i < line_count; i++) {
        if (is_end(lines[i])) {
            break;
        }
        if (!parse_field(lines[i], fields))
            return NULL;
    }

    /*Parse Body*/
    char *body = NULL;
    body = concat_list(lines + i + 1, line_count - i - 1, " ");

    req->body = body;
    req->fields = fields;

    arena_free(arena);

    return req;
}

void req_free(Req *req) {
    a_map_free(req->fields);
    free(req);
}

/* Replaces '?' with '\0' */
int get_querry_params(char *uri, map_a m) {

    Arena *arena = arena_new(0);

    char *q_params = NULL;

    if (!(q_params = strchr(uri, '?'))) {
        arena_free(arena);
        return 0;
    }

    q_params[0] = '\0';
    q_params += 1; // GET TO THE PARAMETERS "?asdasd" -> "asdasd"

    Da_str param_pairs = da_str_new(arena);
    get_words_from_delim(q_params, "&", &param_pairs);
    int n_params = param_pairs.size;
    if (param_pairs.size == 0) {
        perror("PARAM_PAIRS IS EMPTY\n");
        // Add frees here
        arena_free(arena);
        return -1;
    }

    /*We dont free the vals bc map stores raw pointers*/
    /*The keys are stored as copied strings*/
    for (int i = 0; i < n_params; i++) {

        Arena *temp = arena_new(0);

        Da_str param_pair = da_str_new(temp);
        char *key = NULL;
        char *val = NULL;

        get_words_from_delim(param_pairs.list[i], "=", &param_pair);
        int n_pair = param_pair.size;

        if (n_pair <= 0) { // DONT KNOW WHAT
            continue;
        } else if (n_pair == 1) {
            continue;
        } else if (n_pair > 2) {
            key = param_pair.list[0];
            val = str_to_arena_ptr(
                temp, concat_list(param_pair.list + 1, n_pair - 1, "="));
        } else {
            key = param_pair.list[0];
            val = param_pair.list[1];
        }

        char *decoded_key = str_to_arena_ptr(temp, decode_url(key));
        char *decoded_val = str_to_arena_ptr(temp, decode_url(val));

        a_map_set(m, decoded_key, decoded_val);
    }

    arena_free(arena);
    return n_params;
}

char *get_file_path(const char *_uri, map_a m) {

    Arena *arena = arena_new(0);

    if (_uri == NULL) {
        return NULL;
    }

    char *uri = a_strdup(arena, _uri);

    get_querry_params(uri, m);

    char *url = str_to_arena_ptr(arena, decode_url(uri));
    char *s_url = path_sanitize(url);

    arena_free(arena);

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
        char *val = a_map_get(r.fields, key);

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
    char *val = a_map_get(r.fields, key);
    val = val != NULL ? val : "";

    // key: val\r\n
    return strlen(key) + strlen(val) + 4;
}

size_t construct_response(Resp *r, void *_buf) {

    r->body.body_len = r->body.body_len < 0 ? 0 : r->body.body_len;

    assert(r->protocol != NULL);
    assert(r->status_name != NULL);

    //! Add Content-Length: xx if needed
    {
        if (r->body.body != NULL && r->body.body_len > 0) {
            char *content_len_str = "Content-Length";

            size_t c_len = r->body.body_len;
            char *num = malloc((size_t)floor(log10(c_len)) + 2);
            sprintf(num, "%zu", c_len);
            a_map_set(r->fields, content_len_str, num);
            free(num);
        }
    }

    size_t off = 0;
    size_t r_size = calc_response_size(*r);
    *(char **)(_buf) = malloc(r_size); //? Some time can be moved to resp arena!
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
            char *val = a_map_get(r->fields, key);
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

void set_status_code(Resp *r, int status_code, char *code_name) {
    Arena *resp_arena = r->fields.keys->arena;
    r->status_code = status_code;
    r->status_name = a_strdup(resp_arena, code_name);
}

/*
 * Return values is the file byte lenght that was written to the buffer.
 * Special Cases:
 * -1: Non Regular File Type, in this case the buffer is not populated and can
 * contain garbage -2: Directory File, in this case the file is a directory and
 * the body is not populated
 */
void dump_file_to_body(Resp *r, const char *f_name) {
    char *buf;
    int size;
    if ((size = get_file_content(f_name, &buf)) <= 0) {

        switch (size) {
        case NON_REGULAR_FILE:
            // I think that -1 could indicate no file so i dont want to exit and
            // maybe throw a 404 printf("NON-REGULAR-FILE-TYPE `%s'\n", f_name);
            // exit(1);
            break;
        case DIRECTORY_FILE:
            char *str;
            printf("%s DIR-FILE\n", (str = paint_str("[DEBUG]", GREEN)));
            free(str);

            break;
        }
    }

    if (size != NON_REGULAR_FILE)
        r->body.body = buf;
    r->body.body_len = size;
}

void dump_dir_list_html(Resp *r, char *dir_name, char *root) {

    Arena *arena = arena_new(0);
    char *abs_path = strdup(root);
    concat(&abs_path, dir_name);
    abs_path = str_to_arena_ptr(arena, abs_path);

    assert(dir_name != NULL);

    puts(abs_path);
    char *dir_n = strrchr(dir_name, '/');
    if (dir_n == NULL) {
        dir_n = dir_name;
    }
    assert(is_dir(abs_path) &&
           "dump_dir_list_html function cannot be used on files that are "
           "not directories");

    DirComponents comps = get_dir_components(abs_path);

    Da_str html_gen = da_str_new(arena);
    Da_str *da = &html_gen;

    char *style_buf;
    int s = get_file_content(STYLESHEET_PATH, &style_buf);
    TRANSFORM_BUF_TO_C_STR(style_buf, s);

    da_str_push(da, "<!DOCTYPE html>");
    da_str_push(da, "<html><head><meta charset = \"UTF-8\">");
    char *title_tag[16 + 1 + strlen(dir_n)];
    sprintf((char *)title_tag, "<title>%s</title>", dir_n);
    da_str_push(da, (char *)title_tag);
    da_str_push(da, "<style>");
    da_str_push(da, style_buf);
    da_str_push(da, "</style>");
    da_str_push(da,
                "</head>"); // Consider to put the css in another file
    da_str_push(da, "<body>");
    da_str_push(da, "<div class=\"container\">");
    da_str_push(da, "<h1>Directory ");
    da_str_push(da, dir_n);
    da_str_push(da, "</h1>");
    da_str_push(da, "<table class=\"table\">");
    da_str_push(da, "<thead><tr><th>Path</th></tr></thead>");
    da_str_push(da, "<tbody>");
    for (size_t i = 0; i < comps.dirs.size; i++) {
        da_str_push(da, "<tr><td><a href=\"");

        char *new_uri = strdup(dir_name);
        concat(&new_uri, "/");
        concat(&new_uri, comps.dirs.list[i]);

        char *santized_path = path_sanitize(new_uri);
        
        // printf("Dir name in html %s\n", new_uri);
        free(new_uri);
        
        char *_sanitized_path = str_to_arena_ptr(arena, santized_path);
        
        da_str_push(da, _sanitized_path);
        da_str_push(da, "\">");
        da_str_push(da, _sanitized_path);
        da_str_push(da, "</a></td></tr>");

    }
    for (size_t i = 0; i < comps.files.size; i++) {
        da_str_push(da, "<tr><td><a href=\"");

        char *new_uri = strdup(dir_name);
        concat(&new_uri, "/");
        concat(&new_uri, comps.files.list[i]);
        da_str_push(da, new_uri);
        da_str_push(da, "\">");
        da_str_push(da, new_uri);
        da_str_push(da, "</a></td></tr>");
    }
    da_str_push(da, "</tbody>");
    da_str_push(da, "</table>");
    da_str_push(da, "</div>");
    da_str_push(da, "</body></html>");

    char *buf = concat_list(da->list, da->size, "");
    size_t len = strlen(buf);

    arena_free(arena);

    r->body.body = buf;
    r->body.body_len = len;
}

void write_redirect(Resp *r, char *uri) {
    set_status_code(r, 301, "Moved Permanently");
    fields_append(r->fields, "Location", uri);
}

enum Mime_Type get_mime_type(char *url) {

    char *f_name = strrchr(url, '/');
    char *ext = strrchr(f_name, '.');
    if (ext == NULL) {
        return APPLICATION_OCTETSTREAM;
    }
    if (*(ext + 1) == '\0') {
        return APPLICATION_OCTETSTREAM;
    }

    ext = ext + 1;

    if (!strcmp(ext, "txt")) {
        return TEXT_PLAIN;
    } else if (!strcmp(ext, "css")) {
        return TEXT_CSS;
    } else if (!strcmp(ext, "csv")) {
        return TEXT_CSV;
    } else if (!strcmp(ext, "html")) {
        return TEXT_HTML;
    } else if (!strcmp(ext, "js")) {
        return TEXT_JS;
    } else if (!strcmp(ext, "xml")) {
        return TEXT_XML;
    } else if (!strcmp(ext, "avif")) {
        return IMAGE_AVIF;
    } else if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg")) {
        return IMAGE_JPEG;
    } else if (!strcmp(ext, "png") || !strcmp(ext, "ico")) {
        return IMAGE_PNG;
    } else if (!strcmp(ext, "svg")) {
        return IMAGE_SVG;
    } else if (!strcmp(ext, "mpg") || !strcmp(ext, "mpeg")) {
        return AUDIO_MPEG;
    } else if (!strcmp(ext, "json") || !strcmp(ext, "map")) {
        return APPLICATION_JSON;
    } else if (!strcmp(ext, "pdf")) {
        return APPLICATION_PDF;
    } else {
        return APPLICATION_OCTETSTREAM;
    }
}