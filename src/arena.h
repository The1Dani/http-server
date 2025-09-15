#ifndef ARENA
#define ARENA

#include "external/map.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef KILOBYTE
#define KILOBYTE 1024

#ifndef DEFAULT_DA_CAP
#define DEFAULT_DA_CAP 16
#endif

#ifndef DEFAULT_ARENA_SIZE
#define DEFAULT_ARENA_SIZE KILOBYTE
#endif

#else // KILOBYTE
#ifndef DEFAULT_ARENA_SIZE
#define DEFAULT_ARENA_SIZE 1024
#endif
#endif // KILOBYTE

#define A_BYTE
#ifdef A_BYTE
#define byte uint8_t
#endif

struct Arena {

    /*Data is originally allocated and used as uint8_t*/
    void *data;
    size_t capacity;
    size_t head;
    struct Arena *next;
};
typedef struct Arena Arena;

typedef struct {
    char **list;
    size_t cap;
    size_t size;
    Arena *arena;
} Da_str;

typedef struct {
    map_t *map;
    Da_str *keys;
} map_a;

/*Arena Functions*/
/*If size <= 0 -> size=DEFAULT_ARENA_SIZE*/
Arena *arena_new(size_t size);
void *arena_alloc(Arena *arena, size_t size);
void arena_free(Arena *arena);
void *arena_realloc(struct Arena *arena, void *old_p, size_t old_size,
                    size_t new_size);

/*Arena Utility Functions*/
char *a_strdup(Arena *arena, const char *str);
char *str_to_arena_ptr(Arena *arena, char *str);

/*Da_str Functions*/
Da_str da_str_new(Arena *arena);
void da_str_push(Da_str *da, char *str);
char *da_str_pop(Da_str *da);
char *da_str_peek(Da_str *da);
void da_str_destroy(Da_str da);

/*Map functions*/
map_a a_map_new();
void a_map_set(map_a map, char *key, void *val);
void *a_map_get(map_a map, char *key);
void a_map_free(map_a map);

#endif // ARENA


// #define ARENA_IMPLEMENTATION
#ifdef ARENA_IMPLEMENTATION

#define ASSERT_NO_MEM(ptr) assert(ptr != NULL && "OUT OF MEMORRY")

/*Arena Functions*/

struct Arena *arena_new(size_t size) {
    if (size <= 0)
        size = DEFAULT_ARENA_SIZE;

    struct Arena *n_arena = malloc(sizeof(struct Arena));
    ASSERT_NO_MEM(n_arena);

    byte *data = malloc(sizeof(byte) * size);
    ASSERT_NO_MEM(data);
    memset(data, 0, sizeof(byte) * size);

    *n_arena = (struct Arena){
        .data = (void *)data,
        .capacity = size,
        .head = 0,
        .next = NULL,
    };

    return n_arena;
}

void *arena_realloc(struct Arena *arena, void *old_p, size_t old_size,
                    size_t new_size) {

    if (new_size <= old_size)
        return old_p;

    void *new_p = arena_alloc(arena, new_size);
    memcpy(new_p, old_p, old_size);

    return new_p;
}

void *arena_alloc(struct Arena *arena, size_t size) {
    assert(size > 0);
    struct Arena *current = arena;
    while (current->head + size > current->capacity) {
        if (current->next == NULL) {
            current->next = arena_new(current->head + size);
        }
        current = current->next;
    }

    void *alloc_p = (void *)(((byte *)current->data) + current->head);

    current->head += size;
    return alloc_p;
}

void arena_free(struct Arena *arena) {

    struct Arena *cur = arena;

    while (cur != NULL) {
        Arena *tmp = cur->next;
        free(cur->data);
        free(cur);
        cur = tmp;
    }
}

/*Arena Utility Functions*/

char *a_strdup(Arena *arena, const char *str) {
    size_t str_mem_size = strlen(str) + 1;
    char *arena_str = arena_alloc(arena, str_mem_size);
    strncpy(arena_str, str, str_mem_size);
    return arena_str;
}

char *str_to_arena_ptr(Arena *arena, char *str) {
    char *arena_str_ptr = a_strdup(arena, str);
    free(str);
    return arena_str_ptr;
}

/*Da_str Functions*/

Da_str da_str_new(Arena *arena) {

    char **str_list = arena_alloc(arena, sizeof(char *) * DEFAULT_DA_CAP);

    return (Da_str){
        .list = str_list,
        .cap = DEFAULT_DA_CAP,
        .size = 0,
        .arena = arena,
    };
}

void da_realloc(Da_str *da) {
    size_t old_cap = da->cap;
    if (da->cap == 0)
        da->cap = 1;
    assert(da->size <= da->cap);
    da->cap *= 2;
    da->list = arena_realloc(da->arena, da->list, old_cap * sizeof(char *), da->cap * sizeof(char *));
}

void da_str_push(Da_str *da, char *str) {
    if (da->size >= da->cap) {
        da_realloc(da);
    }
    da->list[da->size] = str;
    da->size += 1;
}

char *da_str_pop(Da_str *da) {

    size_t size = da->size;
    if (size <= 0)
        return NULL;

    da->size -= 1;

    return da->list[size - 1];
}

char *da_str_peek(Da_str *da) {
    size_t size = da->size;
    if (size <= 0)
        return NULL;
    return da->list[size - 1];
}

void da_str_destroy(Da_str da) { arena_free(da.arena); }

/*map_a Functions*/
map_a a_map_new() {
    Arena *key_arena = arena_new(DEFAULT_ARENA_SIZE);
    map_t *m = map_new(DEFAULT_SIZE);
    Da_str *k = arena_alloc(key_arena, sizeof(Da_str));
    *k = da_str_new(key_arena);

    return (map_a){
        .map = m,
        .keys = k,
    };
}

void a_map_set(map_a map, char *key, void *val) {
    da_str_push(map.keys, a_strdup(map.keys->arena, key));
    assert(!map_set(map.map, key, a_strdup(map.keys->arena, val)));
}

void *a_map_get(map_a map, char *key) { return map_get(map.map, key); }

void a_map_free(map_a map) {
    map_ffree(map.map, map.keys->list, map.keys->size);
    da_str_destroy(*map.keys);
}

#endif // ARENA_IMPLEMENTATION

#ifdef A_BYTE
#undef A_BYTE
#endif

