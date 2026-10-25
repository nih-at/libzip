#include "zipsh.h"

bool list_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(archive, i, 0);
        if (name == NULL) {
            continue;
        }
        printf("%s\n", name);
    }
    return true;
}

COMMAND_NO_ARGUMENTS_OR_OPTIONS(list, "list the contents of the archive");
