#include "zipsh.h"

bool count_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    if (num_entries < 0) {
        *error_message = error_message_new_format("can't get number of entries in archive: %s", zip_strerror(archive));
        return false;
    }

    printf("%" PRId64 "\n", num_entries);
    return true;
}

COMMAND_NO_ARGUMENTS_OR_OPTIONS(count, "count the number of entries in the archive");
