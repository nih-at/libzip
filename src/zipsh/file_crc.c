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


bool file_crc_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    zip_stat_t st;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (zip_stat_index(archive, file_ref->index, file_ref->flags, &st) < 0) {
        *error_message = error_message_new_format("can't stat file %s: %s", file_ref->error_info, zip_strerror(archive));
        result = false;
    }
    else {
        printf("%08x\n", st.crc);
    }

    file_reference_free(file_ref);

    return result;
}

COMMAND(file_crc, "get CRC of file");
