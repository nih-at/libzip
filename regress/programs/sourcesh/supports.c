#include "sourcesh.h"
#include "zipint.h"

#include <stdio.h>

struct command_flag {
    const char *name;
    zip_int64_t flag;
};

typedef struct command_flag command_flag_t;

/* clang-format off */
argument_definition_t supports_arguments[] = {
    SOURCE_ARGUMENTS
};

const command_flag_t command_names[] = {
    {"accept_empty", ZIP_SOURCE_ACCEPT_EMPTY},
    {"at_eof", ZIP_SOURCE_AT_EOF},
    {"begin_write_cloning", ZIP_SOURCE_BEGIN_WRITE_CLONING},
    {"begin_write", ZIP_SOURCE_BEGIN_WRITE},
    {"close", ZIP_SOURCE_CLOSE},
    {"commit_write", ZIP_SOURCE_COMMIT_WRITE},
    {"error", ZIP_SOURCE_ERROR},
    {"free", ZIP_SOURCE_FREE},
    {"get_dos_time", ZIP_SOURCE_GET_DOS_TIME},
    {"get_file_attributes", ZIP_SOURCE_GET_FILE_ATTRIBUTES},
    {"open", ZIP_SOURCE_OPEN},
    {"read", ZIP_SOURCE_READ},
    {"remove", ZIP_SOURCE_REMOVE},
    {"rollback_write", ZIP_SOURCE_ROLLBACK_WRITE},
    {"seek_write", ZIP_SOURCE_SEEK_WRITE},
    {"seek", ZIP_SOURCE_SEEK},
    {"stat", ZIP_SOURCE_STAT},
    {"supports_reopen", ZIP_SOURCE_SUPPORTS_REOPEN},
    {"supports", ZIP_SOURCE_SUPPORTS},
    {"tell_write", ZIP_SOURCE_TELL_WRITE},
    {"tell", ZIP_SOURCE_TELL},
    {"write", ZIP_SOURCE_WRITE},
};
/* clang-format on */

bool supports_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source = invocation->arguments[ARGUMENT_SOURCE].pointer;
    bool first = true;

    zip_int64_t ret = zip_source_supports(source);
    if (ret < 0) {
        *error_message = error_message_new_format("can't tell position: %s", zip_error_strerror(zip_source_error(source)));
        return false;
    }

    for (size_t i = 0; i < sizeof(command_names) / sizeof(command_names[0]); i++) {
        if (ZIP_SOURCE_CHECK_SUPPORTED(ret, command_names[i].flag)) {
            if (!first) {
                printf(", ");
            }
            printf("%s", command_names[i].name);
            first = false;
        }
    }

    if (!first) {
        printf("\n");
    }

    return true;
}

COMMAND_NO_OPTIONS(supports, "Check what operations are supported by a source");
