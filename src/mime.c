#define _GNU_SOURCE
#include <stdio.h>
#include "mime.h"
#include "mime_types.h"
#include <assert.h>
#include <search.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
/*Note that returning constant chars maybe not what you need in the future just
 * return the dups*/

struct mime_page_t mime_info;


static int comp_mime(const void *m1, const void *m2) {

    struct mime_type_t *mime1 = (struct mime_type_t *)m1;
    struct mime_type_t *mime2 = (struct mime_type_t *)m2;

    return strcmp(mime1->ext, mime2->ext);
}

const char *get_mime_from_ext(const char *ext, struct mime_page_t p) {
    assert(p.mime_buf != NULL);

    struct mime_type_t key = {
        .ext = ext,
    };

    struct mime_type_t *res = bsearch(&key, p.mime_buf, MIME_TYPE_COUNT,
                                      sizeof(struct mime_type_t), comp_mime);
    return res->mime;
}

const char *get_mime_from_filename(const char *filename, struct mime_page_t p) {
    char *ext = strrchr(filename, '.');
    if (ext == NULL) {
        return (const char *)"application/octet-stream";
    }
    if (*(ext + 1) == '\0') {
        return (const char *)"application/octet-stream";
    }

    ext = ext + 1;

    return get_mime_from_ext(ext, p);
}

struct mime_page_t load_mimes() {
    size_t n = MIME_TYPE_COUNT;
    size_t pool_size = 0;

    for (size_t i = 0; i < n; i++) {
        pool_size += strlen(MIME_TYPES[i].mime) + 1;
        pool_size += strlen(MIME_TYPES[i].ext) + 1;
    }

    size_t total_size = pool_size + (n * sizeof(struct mime_type_t));
    int fd = memfd_create("mime_type_table", 0);
    ftruncate(fd, total_size);

    char *mem =
        mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct mime_type_t *arr = (struct mime_type_t *)mem;

    char *pool = (char *)(arr + n); // Start of char data

    size_t offset = 0;
    size_t byte_len = 0;

    for (size_t i = 0; i < n; i++) {
        arr[i].ext = offset + pool;
        byte_len = strlen(MIME_TYPES[i].ext) + 1;
        memcpy(offset + pool, MIME_TYPES[i].ext, byte_len);
        offset += byte_len;

        arr[i].mime = offset + pool;
        byte_len = strlen(MIME_TYPES[i].mime) + 1;
        memcpy(offset + pool, MIME_TYPES[i].mime, byte_len);
        offset += byte_len;
    }

    assert(strcmp(arr[0].ext, "123") == 0);
    assert(strcmp(arr[602].ext, "png") == 0);

    return (struct mime_page_t){
        .mime_buf = arr,
        .mime_fd = fd,
        .mime_total_size = total_size,
    };
}

void deinit(struct mime_page_t p) {
    munmap(p.mime_buf, p.mime_total_size);
    close(p.mime_fd);
}
