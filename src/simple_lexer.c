#define _DEFAULT_SOURCE

#include "simple_lexer.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "arena.h"

void lstrndup(const char *from, char **to, size_t n) {
    if (*to != NULL)
        free(*to);
    *to = NULL;
    *to = strndup(from, n);
}

void lex_destroy(Lexer *lex) {
    if (lex->str != NULL)
        free(lex->str);
}

int get_line_len(const char *str) {
    char *res;

    switch (str[0]) {
    case '\n':
        return 0;
        break;
    case '\0':
        return -1;
        break;
    }

    if ((res = strchr(str, '\n')) != NULL) {
        return (int)(res - str);
    }
    if ((res = strchr(str, '\0')) == NULL) {
        return -1;
    }
    if ((res = strchr(str, '\0'))) {
        return (int)(res - str);
    }

    return 0;
}

int is_whitespace(char ch) {
    if (ch <= 13 && ch >= 9)
        return 1;
    if (ch == ' ')
        return 1;
    return 0;
}

/*It consumes all whitespace till the next word start*/
int get_word_len(Lexer *lex) {

    int start = lex->cur;
    char cur_char = lex->src[lex->cur];
    if (cur_char == '\0')
        return 0;
    if (is_whitespace(cur_char)) {
        lex->cur += 1;
        return get_word_len(lex);
    }

    int len = 0;
    for (;;) {
        cur_char = lex->src[lex->cur];
        if (is_whitespace(cur_char) || cur_char == '\0') {
            break;
        }
        len++;
        lex->cur += 1;
        cur_char = lex->src[lex->cur];
    }
    lex->cur = start;

    return len;
}

void lex_get_word(Lexer *lex) {

    int w_len = get_word_len(lex);
    if (w_len == 0) {
        lex->status = LEXER_EOF;
        return;
    }

    lstrndup(lex->src + lex->cur, &lex->str, w_len); // nnew
    lex->cur += w_len;
    lex->status = LEXER_SUCCSESS;
}

void lex_get_line(Lexer *lex) {

    if (lex->cur != 0 && lex->src[lex->cur] == '\n') {
        lex->cur += 1;
    }

    size_t start = lex->cur;
    int line_len = get_line_len(lex->src + start);

    if (line_len < -1)
        exit(1);

    switch (line_len) {
    case -1:
        // printf("LEXER_EOF, line_len: %d\n", line_len);
        lex->status = LEXER_EOF;
        break;
    case 0:
        lex->status = LEXER_EMPTY;
        break;
    default:
        lex->status = LEXER_SUCCSESS;
        break;
    }

    if (lex->status == LEXER_SUCCSESS) {
        lstrndup(lex->src + start, &lex->str, line_len);
        lex->cur += line_len;
    }
}

void get_words_from_delim(const char *str, const char *delim, Da_str *da) {

    if (str == NULL) {
        return;
    }

    char *body = strdup(str);
    char *_body = body;

    char *token = NULL;

    for (;;) {
        token = strtok(body, delim);
        if (token == NULL)
            break;
        da_str_push(da, a_strdup(da->arena, token));
        body = NULL;
    }

    free(_body);

}

void get_words(const char *str, Da_str *da) {
    get_words_from_delim(str, WHITE_SPACE, da);
}

void concat(char **dst, const char *src) {

    if (!dst || !*dst || !src)
        return;

    char *dst_ = strdup(*dst);
    dst_ = realloc(dst_, strlen(dst_) + strlen(src) + 1);
    *dst = realloc(*dst, strlen(dst_) + strlen(src) + 1);

    sprintf(*dst, "%s%s", dst_, src);

    free(dst_);
}

char *concat_list(char **list, int size, const char *sep) {

    if (size <= 0) {
        return NULL;
    }

    char *dst = ""; //= *dest;

    int i = 1;
    dst = strdup(list[0]);
    for (; i < size; i++) {
        concat(&dst, sep);
        concat(&dst, list[i]);
        // dst = realloc(dst, strlen(dst) + strlen(sep) + strlen(list[i]) + 1);
        // sprintf(dst, "%s%s%s", dst, sep, list[i]);
    }

    return dst;
}

void str_shift_right(char *str, unsigned int amount) {
    memmove(str, str + amount, strlen(str) - amount + 1);
}

char *paint_str(const char *str, const char *color) {
    if (!str || !color) {
        return NULL;
    }

    int seq_len = 5;
    int color_len = (int)strlen(color);
    char *s = calloc(1, seq_len + color_len + strlen(str) + 1);
    if (s == NULL)
        return NULL;
    sprintf(s, ESC "%s%s" ESC_CLOSE, color, str);
    return s;
}

off_t get_file_size(int fd) {
    off_t file_size;
    struct stat stbuf;

    if (fd == -1)
        return -1;

    if ((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
        if (S_ISDIR(stbuf.st_mode)) {
            return -2;
        }
        return -1;
    }
    file_size = stbuf.st_size;

    return file_size;
}

int get_file_content(const char *file, char **buf) {

    int fd = open(file, O_RDONLY);
    if (fd == -1)
        return -1;

    off_t f_size = get_file_size(fd);
    if (f_size <= 0) {
        close(fd);
        return f_size;
    };

    *buf = malloc(f_size);
    read(fd, *buf, f_size);
    close(fd);

    return f_size;
}

void free_str_list(char **li, size_t len) {
    for (size_t i = 0; i < len; i++)
        free(li[i]);
}

Dir_Components get_dir_components(char *dir) {

    Arena *component_arena = arena_new(0);
    Da_str dirs = da_str_new(component_arena);
    Da_str files = da_str_new(component_arena);

    struct dirent **namelist;

    int n = scandir(dir, &namelist, NULL, alphasort);
    assert(n == -1 && "scandir failed");
    
    while(n--) {
        if (namelist[n]->d_type != DT_DIR) {
            da_str_push(&files, strdup(namelist[n]->d_name));
            free(namelist[n]);
        } else {
            da_str_push(&dirs, strdup(namelist[n]->d_name));
            free(namelist[n]);
        }
    }
    free(namelist);

    return (Dir_Components) {
        .files = files,
        .dirs = dirs,
    };
}