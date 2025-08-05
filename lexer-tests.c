#include <stdio.h>
#include <string.h>
#include "simple-lexer.h"

#define TEST_STR "This is a string which has more than one line\nThis is the second line\nThe third"
#define EXPECTED "This is a string which has more than one line"

#define TEST_STR_2 "Hello From some client!\n\n"
#define EXPECTED_2 "Hello From some client!"

#define TEST_WORD "Multi word string"
#define WORD_EXPECTED "Multi"
#define WORD_EXPECTED_1 "word"
#define WORD_EXPECTED_2 "string"

int test_get_line(const char *t_str, const char *t_str_exp) {
    int ret = 0;
    Lexer lexer = {0};
    Lexer *lex = &lexer;

    const char *str = TEST_STR;
    lex->src = str;
    lex_get_line(lex);


    if (lex->status != LEXER_SUCCSESS){
        switch (lex->status) {
        case LEXER_EMPTY:
            printf("Got LEXER_EMPTY\n");
            break;
        case LEXER_EOF:
            printf("Got LEXER_EOF\n");
            break;
        default:
            printf("Got Unkown Err\n");
            break;
        }
        ret = 1;
    }
    int diff;
    if ((diff = strcmp(t_str, lex->str))){
        printf("Lexer got\n%s\nExpected\n%s\nStrLen1 - %lu StrLen2 - %lu\nDiff - %d\n", lex->str, t_str, strlen(lex->str), strlen(t_str_exp), diff);
        ret = 1;
    }

    lex_destroy(lex);
    return ret;
}

int test_get_lines() {
    int ret = 0;
    Lexer lexer = {0};
    Lexer *lex = &lexer;

    const char *str = TEST_STR_2;

    lex->src = str;

    char *line;
    
    for(;;) {
        lex_get_line(lex);
        if(lex->status != LEXER_SUCCSESS) break; 
        line = lex->str;
        printf("%s\n", line);
    } 

    lex_destroy(lex);
    return ret;
}

int test_get_word(const char *str, const char *expected) {

    int ret = 0;
    Lexer lexer = {0};
    Lexer *lex = &lexer;
    lex->src = str;

    lex_get_word(lex);
    if (lex->status != LEXER_SUCCSESS) {
        lex_destroy(lex);
        return 1;
    }

    if (strcmp(expected, lex->str)) {
        return 1;
    }
    
    return 0;
}

int test_get_words(const char *str, const char *expected[]) {
    int ret = 0;
    Lexer lexer = {0};
    Lexer *lex = &lexer;
    lex->src = str;

    int idx = 0;
    for (;;) {
        lex_get_word(lex);
        if (lex->status == LEXER_EOF) break;
        if (strcmp(lex->str, expected[idx])) {
            printf("Expected `%s' got `%s' \n", expected[idx], lex->str);
            return idx;    
        }
        idx++;
    }

    return ret;
}

int main() {

    const char *get_word_arr[] = 
        {WORD_EXPECTED, WORD_EXPECTED_1, WORD_EXPECTED_2};

    // if(test_get_line(TEST_STR, EXPECTED)){ 
    //     printf("The Lexer had problem getting the line\tPASSED 0/2\n");
    //     return 1;
    // }
    
    // if(test_get_line(TEST_STR_2, EXPECTED_2)){ 
    //     printf("The Lexer had problem getting the line\tPASSED 0/2\n");
    //     return 1;
    // }

    // if(test_get_lines()){
    //     printf("The Lexer had problem getting the lines\tPASSED 1/2\n");
    //     return 1;
    // }

    if(test_get_word(TEST_WORD, WORD_EXPECTED)) {
        printf("Did not parse word correctly\n");
        return 1;
    }
    int idx; 
    if(( idx = test_get_words(TEST_WORD, get_word_arr))) {
        printf("Did not parse word correctly on word `%s'\n", get_word_arr[idx]);
        return 1;
    }


    printf("PASSED 2/2\n");

    return 0;
}