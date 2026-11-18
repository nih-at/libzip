#include "sourcesh.h"

/* clang-format off */
argument_definition_t stat_arguments[] = {
    SOURCE_ARGUMENTS
};
/* clang-format on */

bool stat_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    zip_stat_t st;

    if (zip_source_stat(source, &st) < 0) {
        *error_message = error_message_new_format("can't stat source: %s", zip_error_strerror(zip_source_error(source)));
        zip_source_close(source);
        return false;
    }

    /* TODO: Share code to print stat information with zipsh. */
    if (st.valid & ZIP_STAT_NAME) {
        printf("Name: %s\n", st.name);
    }
    if (st.valid & ZIP_STAT_INDEX) {
        printf("Index: %" PRIu64 "\n", st.index);
    }
    if (st.valid & ZIP_STAT_SIZE) {
        printf("Size: %" PRIu64 "\n", st.size);
    }
    return true;
}

COMMAND_NO_OPTIONS(stat, "Get the stat of a source");
