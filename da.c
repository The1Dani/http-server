#include "da.h"
#include <stdlib.h>


//init empty string dynamic array
Da_str da_str_new() {

    char **str_list = malloc(sizeof(char*) * DEFAULT_DA_CAP);

    return (Da_str) {
        .list = str_list,
        .cap = DEFAULT_DA_CAP,
        .size = 0,
    };

}

void da_realloc(Da_str* da) {
    if (da->cap == 0) da->cap = 1;
    if (da->size > da->cap) {
        printf("Problem with da->size expected <= da->cap = %lu got da->size = %lu\n", da->cap, da->size);
        exit(1);
    }
    da->cap *= 2;
    da->list = realloc(da->list, sizeof(char*) * da->cap);
}

//Push a new string to array
void da_str_push(Da_str *da, char *str) {
    if (da->size >= da->cap) {
        da_realloc(da);
    }
    da->list[da->size] = str;
    da->size += 1;
}

//Free da_str
void da_str_destroy(Da_str da) {
    free(da.list);
}