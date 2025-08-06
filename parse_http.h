#ifndef PARSE_HTML
#define PARSE_HTML

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/map.h"
#include "simple_lexer.h"
#include <stdbool.h>

#define SUPPORTED_PROTOCOL "HTTP/1.1"
#define HTTP_STATUS_OK "200"
#define RESP_OK SUPPORTED_PROTOCOL" " HTTP_STATUS_OK " OK\r\n" 

typedef struct {

    const char* method;
    const char* uri;
    const char* protocol;
    map_t *fileds;
    const char* body;

} Req;


Req *http_parse_req(char **lines, size_t line_count);

#endif