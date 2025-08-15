#include "url_escape.h"

#define str_equal(str1, str2) strcmp(str1, str2) == 0

char *go_next(char *s, char *tok) {

    char *t = strchr(s, '%');
    if (t == NULL)
        return NULL;

    memcpy(tok, t, sizeof(char) * 3);
    str_shift_right(t, 2);
    return t;
}

void replace_to_ascii(char *str, char *tok) {

    if (!str || !tok)
        exit(4);

#define ELSE_IF_TOK_TO_ASCII(tok_name, var_name, tok, chr)                     \
    else if (str_equal(tok_name, tok)) {                                       \
        var_name = chr;                                                        \
    }
    char ch = '\0';
    if (str_equal(tok, PERCENT_SPACE)) {
        ch = ' ';
    }
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_EXLAMATION, '!')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_QUOTES, '"')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_POUND, '#')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_DOLLAR, '$')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_PERCENT, '%')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_AMPERSAND, '&')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_APOSTROPHE, '\'')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_BRACKET_OPEN, '(')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_BRACKET_CLOSE, ')')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_ASTERISK, '*')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_PLUS, '+')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_COMMA, ',')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_HYPHEN, '-')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_PERIOD, '.')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_SLASH, '/')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_COLON, ':')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_SEMICOLON, ';')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_LESS_THAN, '<')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_EQUAL, '=')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_GREATER_THAN, '>')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_QUESTION, '?')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_AT_SIGN, '@')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_SQ_BRACKET_OPEN, '[')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_BACK_SLASH, '\\')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_SQ_BRACKET_CLOSE, ']')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_CARET, '^')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_UNDERSCORE, '_')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_GRAVE, '`')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_CURLY_BRACE_OPEN, '{')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_PIPE, '|')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_CURLY_BRACE_CLOSE, '}')
    ELSE_IF_TOK_TO_ASCII(tok, ch, PERCENT_TILDE, '~')
    else {
        printf("THIS PERCENT CHAR IS NOT DEFINED %s\n", tok);
        ch = ' ';
    }

    str[0] = ch;
}

char *decode_url(char *_s) {

    char *s = strdup(_s);

    char *nxt = NULL;
    char *tok = malloc(sizeof(char) * 3);

    nxt = go_next(s, tok);
    if (!nxt)
        goto defer;
    replace_to_ascii(nxt, tok);

    bool done = nxt == NULL ? true : false;

    while (!done) {

        if ((nxt = go_next(nxt, tok)) == NULL) {
            break;
        }
        replace_to_ascii(nxt, tok);
    }

    defer:
        free(tok);
        return s;
}