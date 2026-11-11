#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

#define ARGUMENT_COMMENT 1

/* clang-format off */
argument_definition_t file_comment_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
    {"comment", "comment for the file in the archive", argument_type_string, ARGUMENT_OPTIONAL, {.string = NULL}},
};

option_definition_t file_comment_options[] = {
    FILE_REFERENCE_OPTIONS,
    {'h', "hex", NULL, "use hexadecimal format for comment", NULL}
};

struct context {
    const char *comment;
    zip_uint32_t comment_len;
    bool hex;
    bool free_comment;
};

typedef struct context context_t;
/* clang-format on */

bool file_comment_setup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;
    size_t comment_length;

    context->hex = invocation_get_option(invocation, 'h');
    context->comment = invocation->arguments[ARGUMENT_COMMENT].string;
    comment_length = context->comment ? strlen(context->comment) : 0;
    context->free_comment = false;

    if (context->hex && context->comment) {
        context->comment = (const char *)decode_hex(context->comment, error_message);
        if (!context->comment) {
            return false;
        }
        comment_length /= 2;
        context->free_comment = true;
    }
    if (comment_length > ZIP_UINT32_MAX) {
        *error_message = error_message_new_format("comment too long");
        return false;
    }
    context->comment_len = (zip_uint32_t)comment_length;
    return true;
}

bool file_comment_execute(invocation_t *invocation, void *context_v, file_matches_t *matches, file_reference_t *file_ref, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;

    if (context->comment == NULL) {
        zip_uint32_t comment_len;
        const char *existing_comment = zip_file_get_comment(archive, file_ref->index, &comment_len, matches->flags);
        if (existing_comment == NULL) {
            *error_message = error_message_new_format("can't get comment for %s: %s", file_ref->error_info, zip_strerror(archive));
            return false;
        }

        if (comment_len > 0) {
            if (context->hex) {
                print_hex((const zip_uint8_t *)existing_comment, comment_len);
            }
            else {
                printf("%.*s\n", comment_len, existing_comment);
            }
        }
        return true;
    }
    else {
        if (zip_file_set_comment(archive, file_ref->index, context->comment, context->comment_len, get_flags(invocation)) < 0) {
            *error_message = error_message_new_format("can't set comment for %s: %s", file_ref->error_info, zip_strerror(archive));
            return false;
        }
    }

    return true;
}

bool file_comment_cleanup(invocation_t *invocation, void *context_v, file_matches_t *matches, error_message_t **error_message) {
    context_t *context = (context_t *)context_v;
    if (context->free_comment) {
        free((void *)context->comment);
    }
    return true;
}

bool file_comment_implementation(invocation_t *invocation, error_message_t **error_message) {
    context_t context = {NULL, 0, false, false};
    file_reference_command_t command = {file_comment_setup, file_comment_execute, file_comment_cleanup, &context};
    return file_reference_command_execute(&command, invocation, error_message);
}

COMMAND(file_comment, "get or set comment for file in archive");
