#ifndef SIMP_LEX
#define SIMP_LEX

#include "da.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESC "\x1b"
#define RED "[31m"
#define GREEN "[32m"
#define ESC_CLOSE "\x1b[0m"
#define WHITE_SPACE "\x09\x0a\x0b\x0c\x0d\x20"

enum {
    LEXER_SUCCSESS,
    LEXER_EMPTY,
    LEXER_EOF,
    LEXER_ERR,
};

typedef struct {
    // The main str
    const char *src;
    size_t cur;

    // The resulting str
    char *str;

    // The status of the last func
    int status;
} Lexer;

void lex_destroy(Lexer *lex);

int get_line_len(const char *str);

void lex_get_line(Lexer *lex);

void lex_get_word(Lexer *lex);

int get_words_from_delim(const char *str, const char *delim, char ***list);

int get_words(const char *str, char ***list);

int is_whitespace(char ch);

void concat_list(char **list, int size, char **dest, const char *sep);

void str_shift_right(char *str, unsigned int amount);

/*It consumes all whitespace till the next word start*/
int get_word_len(Lexer *len);

char *paint_str(const char *str, const char *color);

#endif