#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

#define ARGUMENT_COMMENT 0

/* clang-format off */
argument_definition_t archive_comment_arguments[] = {
    {"comment", "comment for the file in the archive", argument_type_string, ARGUMENT_OPTIONAL, {.string = NULL}},
};

option_definition_t archive_comment_options[] = {
    CHANGED_FLAGS_OPTIONS, \
    {'h', "hex", NULL, "use hexadecimal format for comment", NULL}
};
/* clang-format on */

bool archive_comment_implementation(invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    bool hex = invocation_get_option(invocation, 'h');
    const char *comment = invocation->arguments[ARGUMENT_COMMENT].string;

    if (comment == NULL) {
        int comment_len;
        const char *existing_comment = zip_get_archive_comment(archive, &comment_len, get_flags(invocation));
        if (existing_comment == NULL) {
            *error_message = error_message_new_format("can't get archive comment: %s", zip_strerror(archive));
            return false;
        }

        if (comment_len > 0) {
            if (hex) {
                print_hex((const zip_uint8_t *)existing_comment, comment_len);
            }
            else {
                printf("%.*s\n", comment_len, existing_comment);
            }
        }
        return true;
    }
    else {
        char *decoded_comment = NULL;
        size_t comment_length = strlen(comment);
        if (hex) {
            decoded_comment = (char *)decode_hex(comment, error_message);
            if (!decoded_comment) {
                *error_message = error_message_new_format("invalid hex comment");
                return false;
            }
            comment = decoded_comment;
            comment_length /= 2;
        }
        if (comment_length > ZIP_UINT16_MAX) {
            *error_message = error_message_new_format("comment too long");
            return false;
        }
        if (zip_set_archive_comment(archive, comment, (zip_uint16_t)comment_length) < 0) {
            *error_message = error_message_new_format("can't set archive comment: %s", zip_strerror(archive));
            result = false;
        }
        if (decoded_comment) {
            free(decoded_comment);
        }
    }

    return result;
}

COMMAND(archive_comment, "get or set comment for file in archive");
