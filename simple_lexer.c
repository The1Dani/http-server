#include "simple_lexer.h"
#include "da.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    // TODO
    // lex->str = realloc(lex->str, sizeof(char) * (w_len + 1));
    // memcpy(lex->str, lex->src + lex->cur, w_len);
    // lex->str[w_len] = '\0';
    // lex->str = strndup(lex->src + lex->cur, w_len); //new
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
        // TODO
        //  lex->str = realloc(lex->str, sizeof(char) * (line_len + 1));
        //  memcpy(lex->str, lex->src + start, line_len);
        //  lex->str[line_len] = '\0';
        //  lex->str = strndup(lex->src + start, line_len);//new
        lstrndup(lex->src + start, &lex->str, line_len);
        lex->cur += line_len;
    }
}

int get_words_from_delim(const char *str, const char *delim, char ***list) {

    if (*list != NULL)
        free(list);

    if (str == NULL) {
        return 0;
    }

    char *body = strdup(str);
    char *_body = body;

    char *token = NULL;
    char *saveptr = NULL;

    Da_str str_arr = da_str_new();

    for (;;) {
        token = strtok_r(body, delim, &saveptr);
        if (token == NULL)
            break;
        da_str_push(&str_arr, strdup(token));
        body = NULL;
    }

    *list = str_arr.list;
    free(_body);

    return str_arr.size;
}

int get_words(const char *str, char ***list) {
    return get_words_from_delim(str, WHITE_SPACE, list);
}

void concat_list(char **list, int size, char **dest, const char *sep) {

    if (size <= 0) {
        return;
    }

    if (*dest != NULL) {
        return;
    }
    char *dst = ""; //= *dest;

    int i = 1;
    dst = strdup(list[0]);
    for (; i < size; i++) {
        dst = realloc(dst, strlen(dst) + strlen(sep) + strlen(list[i]) + 1);
        sprintf(dst, "%s%s%s", dst, sep, list[i]);
    }

    *dest = dst;
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
    sprintf(s, ESC"%s%s"ESC_CLOSE, color, str);
    return s;
}

void free_str_list(char **li, size_t len) {
    for (size_t i = 0; i < len; i++) 
        free(li[i]);
}