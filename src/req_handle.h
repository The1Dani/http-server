#ifndef REQ_HANDLE
#define REQ_HANDLE

#include "parse_http.h"


extern char *ROOT_FOLDER;

void req_handler(Req *req, Resp *resp);

#endif