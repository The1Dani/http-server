#include "simple_lexer.h"
#include "da.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void lstrndup(const char *from, char **to, size_t n) {
    if (*to != NULL) free(*to);
    *to = NULL;
    *to = strndup(from, n);
}

void lex_destroy(Lexer *lex) {
    if (lex->str != NULL) 
        free(lex->str);
}

int get_line_len(const char *str) {
    char* res;
    
    switch (str[0]) {
    case '\n': return 0;
    break;
    case '\0': return -1;
    break;
    }

    if ((res = strchr(str, '\n')) != NULL) {
        return (int) (res - str);
    }
    if ((res = strchr(str, '\0')) == NULL) {
        return -1;
    }
    if ((res = strchr(str, '\0'))){
        return (int) (res - str);
    }    

    return 0;
}

int is_whitespace(char ch) {
    if (ch <= 13 && ch >= 9) return 1;
    if (ch == ' ') return 1;
    return 0;
}


/*It consumes all whitespace till the next word start*/
int get_word_len(Lexer *lex) {

    int start = lex->cur;
    char cur_char = lex->src[lex->cur];
    if (cur_char == '\0') return 0;
    if (is_whitespace(cur_char)) {
        lex->cur += 1;
        return get_word_len(lex);
    }
    
    int len = 0;
    for (;;) {
        cur_char = lex->src[lex->cur];
        if (is_whitespace(cur_char) || cur_char == '\0'){
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
    
    size_t start = lex->cur;

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
    lstrndup(lex->src + lex->cur, &lex->str, w_len); //nnew
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
        //TODO
        // lex->str = realloc(lex->str, sizeof(char) * (line_len + 1));
        // memcpy(lex->str, lex->src + start, line_len);
        // lex->str[line_len] = '\0';
        // lex->str = strndup(lex->src + start, line_len);//new
        lstrndup(lex->src + start, &lex->str, line_len);
        lex->cur += line_len;
    }
}

int get_words_from_delim(const char *str, const char *delim, char ***list) {
    
    if (*list != NULL) 
        free(list);
    
    char *body = strdup(str);

    char *token = NULL;
    char *saveptr = NULL;
    
    Da_str str_arr = da_str_new();

    for (;;) {
        token = strtok_r(body, delim, &saveptr);
        if (token == NULL) 
            break;
        da_str_push(&str_arr, token);
        body = NULL;
    }

    *list = str_arr.list;
    
    return str_arr.size;
}


int get_words(const char *str, char ***list) {
    return get_words_from_delim(str, WHITE_SPACE, list);
}

void concat_list(char**list, int size, char** dest) {
    
    if (*dest != NULL) {
        return;
    }
    char *dst = *dest;

    dst = strdup(list[0]);
    dst = realloc(dst, strlen(dst) + 2);
    strncat(dst, ":", 2);

    for (int i = 1; i < size; i++) {
        dst = realloc(dst, sizeof(char *) * (strlen(dst) + strlen(list[i]) + 2));
        strncat(dst, ":", 2);
        strncat(dst, list[i], (strlen(dst) + strlen(list[i]) + 1));
    }

}
