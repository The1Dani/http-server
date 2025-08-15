#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define EXTERNAL_FOLDER "src/external/"

#define objects_from_da(cmd, da, output_folder)                                \
    da_foreach(char *, obj, da) {                                              \
        cmd_append(cmd, nob_temp_sprintf("%s%s.o", output_folder, *obj));      \
    }

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

bool build_object(bool force, Nob_Cmd *cmd, const char *name,
                  const char *from) {

    const char *source = nob_temp_sprintf("%s%s.c", from, name);
    const char *output = nob_temp_sprintf(BUILD_FOLDER "%s.o", name);
    ;

    int rebuild_is_needed = nob_needs_rebuild1(output, source);
    if (rebuild_is_needed < 0)
        return false;

    if (rebuild_is_needed || force) {
        cmd_append(cmd, "gcc");
        nob_cc_flags(cmd);
        cmd_append(cmd, "-ggdb");
        cmd_append(cmd, "-c");
        nob_cc_output(cmd, output);
        nob_cc_inputs(cmd, source);
        return cmd_run(cmd);
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
    const char *object_names[] = {"simple_lexer", "da", "parse_http",
                                  "url_escape"};
    const char *external_names[] = {"map"};

    da_append_many(&objects, object_names, ARRAY_LEN(object_names));
    da_append_many(&externals, external_names, ARRAY_LEN(external_names));

    da_foreach(char *, obj, &objects) {
        if (!build_object(true, &cmd, *obj, SRC_FOLDER))
            return 1;
    }

    da_foreach(char *, ext, &externals) {
        if (!build_object(true, &cmd, *ext, EXTERNAL_FOLDER))
            return 1;
    }

    /*Build final executable*/
    cmd_append(&cmd, "gcc");
    nob_cc_flags(&cmd);
    objects_from_da(&cmd, &objects, BUILD_FOLDER);
    objects_from_da(&cmd, &externals, BUILD_FOLDER);
    nob_cc_output(&cmd, "listener");
    nob_cc_inputs(&cmd, SRC_FOLDER "listener.c");
    if (!nob_cmd_run(&cmd))
        return 1;
}