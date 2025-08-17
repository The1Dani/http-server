#ifndef D_ARR
#define D_ARR

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef DEFAULT_DA_CAP
#define DEFAULT_DA_CAP 16
#endif

typedef struct {
    char **list;
    size_t cap;
    size_t size;
} Da_str;

// init empty string dynamic array
Da_str da_str_new();

// Push a new string to array
void da_str_push(Da_str *da, char *str);

// Free da_str
void da_str_destroy(Da_str da);

char *da_str_pop(Da_str *da);

char *da_str_peek(Da_str *da);

void da_push_list_inversed(Da_str *da, int len, char **li);

#endif