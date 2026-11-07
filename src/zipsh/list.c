#include "zipsh.h"

#include "compat/src_compat.h"

#define ARGUMENT_PATTERN 0

/* clang-format off */
argument_definition_t list_arguments[] = {
    { "pattern", "pattern of files to list", argument_type_string, ARGUMENT_OPTIONAL, {.string = NULL}}
};

option_definition_t list_options[] = {
    CHANGED_FLAGS_OPTIONS
};

bool list_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    const char *pattern = invocation->arguments[ARGUMENT_PATTERN].string;

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(archive, i, 0);
        if (name == NULL) {
            continue;
        }
        if (pattern == NULL || fnmatch(pattern, name, 0) == 0) {
            printf("%s\n", name);
        }
    }
    return true;
}

COMMAND_NO_OPTIONS(list, "list the contents of the archive");
