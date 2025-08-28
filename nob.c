#include <stdbool.h>
#include <stdio.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <stdarg.h>

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define EXTERNAL_FOLDER "src/external/"
#define TEST_BUILD_FOLDER "tests/"


#define BUILD_OBJECTS_EVERYTIME

#ifdef BUILD_OBJECTS_EVERYTIME
#define BUILD_OBJECTS 1
#else
#define BUILD_OBJECTS 0
#endif

#define objects_from_da(cmd, da, output_folder)                                \
    da_foreach(char *, obj, da) {                                              \
        cmd_append(cmd, nob_temp_sprintf("%s%s.o", output_folder, *obj));      \
    }

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

void flags_and_cc(Nob_Cmd *cmd) {
    cmd_append(cmd, "gcc");
    nob_cc_flags(cmd);
    cmd_append(cmd, "-ggdb", "-lm");
    cmd_append(cmd, "-pedantic");
    cmd_append(cmd, "-std=c23");
}

bool build_object(bool force, Nob_Cmd *cmd, const char *name,
                  const char *from) {

    const char *source = nob_temp_sprintf("%s%s.c", from, name);
    const char *source_header = nob_temp_sprintf("%s%s.h", from, name);
    const char *output = nob_temp_sprintf(BUILD_FOLDER "%s.o", name);

    const char *inputs[] = {source, source_header};

    int rebuild_is_needed = nob_needs_rebuild(output, inputs, 2);
    if (rebuild_is_needed < 0)
        return false;

    if (rebuild_is_needed || force) {
        flags_and_cc(cmd);
        cmd_append(cmd, "-c");
        nob_cc_output(cmd, output);
        nob_cc_inputs(cmd, source);
        return cmd_run(cmd);
    }

    return true;
}

bool build_exec(bool force, Nob_Cmd *cmd, const char *name, const char *from,
                const char *to, int n_list, ...) {

    va_list args;

    const char *source = nob_temp_sprintf("%s%s.c", from, name);
    const char *output = nob_temp_sprintf(BUILD_FOLDER "%s", name);

    int rebuild_is_needed = nob_needs_rebuild1(output, source);
    if (rebuild_is_needed < 0)
        return false;

    if (rebuild_is_needed || force) {
        flags_and_cc(cmd);

        va_start(args, n_list);
        for (int i = 0; i < n_list; i++) {
            Strings *nxt_list = va_arg(args, Strings *);
            objects_from_da(cmd, nxt_list, BUILD_FOLDER);
        }
        va_end(args);

        nob_cc_output(cmd, nob_temp_sprintf("%s%s", to, name));
        nob_cc_inputs(cmd, nob_temp_sprintf("%s%s.c", from, name));
        return nob_cmd_run(cmd);
    }

    return true;
}

int main(int argc, char **argv) {

    NOB_GO_REBUILD_URSELF(argc, argv);
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;
    Nob_Cmd cmd = {0};
    Strings objects = {0};
    Strings externals = {0};
    Strings tests = {0};
    const char *object_names[] = {"simple_lexer", "parse_http",
                                  "url_escape"};
    const char *external_names[] = {"map"};
    const char *test_names[] = {"test01", "test02", "test03"};

    da_append_many(&objects, object_names, ARRAY_LEN(object_names));
    da_append_many(&externals, external_names, ARRAY_LEN(external_names));
    da_append_many(&tests, test_names, ARRAY_LEN(test_names));

    da_foreach(char *, obj, &objects) {
        if (!build_object(BUILD_OBJECTS, &cmd, *obj, SRC_FOLDER))
            return 1;
    }

    da_foreach(char *, ext, &externals) {
        if (!build_object(BUILD_OBJECTS, &cmd, *ext, EXTERNAL_FOLDER))
            return 1;
    }

    /*Build final executable*/
    if (!build_exec(true, &cmd, "listener", SRC_FOLDER, "", 2, &objects,
                    &externals))
        return 1;

    if (!nob_mkdir_if_not_exists(TEST_BUILD_FOLDER))
        return 1;

    da_foreach(char *, test, &tests) {
        if (!build_exec(true, &cmd, *test, SRC_FOLDER "tests/",
                        TEST_BUILD_FOLDER, 2, &objects, &externals))
            return 1;
    }
}