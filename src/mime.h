#ifndef MIME_TYPES_HEADER
#define MIME_TYPES_HEADER

#include <stdlib.h>

struct mime_type_t{
    const char *ext;
    const char *mime;
};

struct mime_page_t {
    struct mime_type_t *mime_buf;
    int mime_fd;
    size_t mime_total_size;
};


struct mime_page_t load_mimes();
const char *get_mime_from_ext(const char *ext, struct mime_page_t p);
const char *get_mime_from_filename(const char *ext, struct mime_page_t p);
void deinit(struct mime_page_t p);

#endif
