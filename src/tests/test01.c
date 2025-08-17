#include "../../nob.h"
#include "../url_escape.h"
#include <stddef.h>
#include <stdio.h>

int main() {

    // '/' -> empty stack
    // '/hello' -> hello
    // '/hey/asd/' -> asd <- hey
    // '/./../../../asd/../hey/' -> hey <- '..' <- asd <- '..' <-  '..' <-  '..'
    // <- '.'
    //                              --^
    // another stack                         push >  pop > push

    const char *case1 = "/";
    const char *case2 = "/hello";
    const char *case3 = "/hey/asd";
    const char *case4 = "/./";
    const char *case5 = "/../hey/..";
    const char *case6 = "/../hey/../asd/asd2";
    const char *case7 = "/../hey/";
    const char *case8 = "//hey/.../11";

    const char *cases[] = {case1, case2, case3, case4,
                           case5, case6, case7, case8};

    for (size_t i = 0; i < NOB_ARRAY_LEN(cases); i++) {
        printf("Case %zu: %s -> %s\n", i + 1, cases[i],
               path_sanitize(cases[i]));
    }
}