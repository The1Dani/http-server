#ifndef SIMP_LEX
#define SIMP_LEX

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ESC "\x1b"
#define RED "[31m"
#define GREEN "[32m"
#define ESC_CLOSE "\x1b[0m"

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
    
    //The resulting str
    char *str;    

    //The status of the last func
    int status;
} Lexer;

void lex_destroy(Lexer *lex);

int get_line_len(const char *str);

void lex_get_line(Lexer *lex);

void lex_get_word(Lexer *len);

int is_whitespace(char ch);

/*It consumes all whitespace till the next word start*/
int get_word_len(Lexer *len);

#endif