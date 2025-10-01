#ifndef REQ_HANDLE
#define REQ_HANDLE

#include "mime.h"
#include "parse_http.h"

extern char *ROOT_FOLDER;
extern struct mime_page_t mime_info;

void req_handler(Req *req, Resp *resp);

#endif
