#ifndef URL_ESCAPE
#define URL_ESCAPE

#include "da.h"
#include "simple_lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*RESERVED CHARACTERS AFTER PERCENT-ENCODING*/
#define PERCENT_SPACE "%20"             // ' '
#define PERCENT_EXLAMATION "%21"        // '!'
#define PERCENT_QUOTES "%22"            // '"'
#define PERCENT_POUND "%23"             // '#'
#define PERCENT_DOLLAR "%24"            // '$'
#define PERCENT_PERCENT "%25"           // '%'
#define PERCENT_AMPERSAND "%26"         // '&'
#define PERCENT_APOSTROPHE "%27"        // '''
#define PERCENT_BRACKET_OPEN "%28"      // '('
#define PERCENT_BRACKET_CLOSE "%29"     // ')'
#define PERCENT_ASTERISK "%2A"          // '*'
#define PERCENT_PLUS "%2B"              // '+'
#define PERCENT_COMMA "%2C"             // ','
#define PERCENT_HYPHEN "%2D"            // '-'
#define PERCENT_PERIOD "%2E"            // '.'
#define PERCENT_SLASH "%2F"             // '/'
#define PERCENT_COLON "%3A"             // ':'
#define PERCENT_SEMICOLON "%3B"         // ';'
#define PERCENT_LESS_THAN "%3C"         // '<'
#define PERCENT_EQUAL "%3D"             // '='
#define PERCENT_GREATER_THAN "%3E"      // '>'
#define PERCENT_QUESTION "%3F"          // '?'
#define PERCENT_AT_SIGN "%40"           // '@'
#define PERCENT_SQ_BRACKET_OPEN "%5B"   // '['
#define PERCENT_BACK_SLASH "%5C"        // '\'
#define PERCENT_SQ_BRACKET_CLOSE "%5D"  // ']'
#define PERCENT_CARET "%5E"             // '^'
#define PERCENT_UNDERSCORE "%5F"        // '_'
#define PERCENT_GRAVE "%60"             // '`'
#define PERCENT_CURLY_BRACE_OPEN "%7B"  // '{'
#define PERCENT_PIPE "%7C"              // '|'
#define PERCENT_CURLY_BRACE_CLOSE "%7D" // '}'
#define PERCENT_TILDE "%7E"             // '~'

char *decode_url(char *_s);

#endif