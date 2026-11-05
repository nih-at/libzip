#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

bool file_comment_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    bool hex = invocation_get_option(invocation, 'h');
    const char *comment = invocation->arguments[1].string;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }

    if (comment == NULL) {
        zip_uint32_t comment_len;
        const char *existing_comment = zip_file_get_comment(archive, file_ref->index, &comment_len, get_flags(invocation));
        if (existing_comment) {
            if (hex) {
                print_hex(existing_comment, comment_len);
            }
            else {
                printf("%.*s\n", comment_len, existing_comment);
            }
        }
        return true;
    }
    else {
        char *decoded_comment = NULL;
        if (hex) {
            decoded_comment = decode_hex(comment, error_message);
            if (!decoded_comment) {
                *error_message = error_message_new_format("invalid hex comment");
                return false;
            }
            comment = decoded_comment;
        }
        if (zip_file_set_comment(archive, file_ref->index, comment, strlen(comment), get_flags(invocation)) < 0) {
            *error_message = error_message_new_format("can't set comment for %s: %s", file_ref->error_info, zip_strerror(archive));
            result = false;
        }
        if (decoded_comment) {
            free(decoded_comment);
        }
    }

    file_reference_free(file_ref);

    return result;
}

/* clang-format off */
argument_definition_t file_comment_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"comment", "comment for the file in the archive", argument_type_string, false, {.string = NULL}},
};

option_definition_t file_comment_options[] = {
    FILE_REFERENCE_OPTIONS,
    {'h', "hex", "use hexadecimal format for comment"}
};
/* clang-format on */

COMMAND(file_comment, "get or set comment for file in archive");
