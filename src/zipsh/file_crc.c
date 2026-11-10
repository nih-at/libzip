#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

/* clang-format off */
argument_definition_t file_crc_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t file_crc_options[] = {
    FILE_REFERENCE_OPTIONS,
};
/* clang-format on */


bool file_crc_execute(invocation_t *invocation, void *context, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    zip_stat_t st;

    (void)context;

    if (zip_stat_index(archive, file_ref->index, matches->flags, &st) < 0) {
        *error_message = error_message_new_format("can't stat file %s: %s", file_ref->error_info, zip_strerror(archive));
        return false;
    }
    else {
        printf("%08x\n", st.crc);
    }

    return true;
}

bool file_crc_implementation(invocation_t *invocation, error_message_t **error_message) {
    file_reference_command_t command = {NULL, file_crc_execute, NULL, NULL};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(file_crc, "get CRC of file");
