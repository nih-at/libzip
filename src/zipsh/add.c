#include <errno.h>
#include <string.h>

#include "zip.h"
#include "zipsh.h"

bool add_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_source_t *source;
    bool is_data = invocation_get_option(invocation, 'd');
    zip_uint64_t start = invocation->arguments[2].u64;
    zip_int64_t len = invocation->arguments[3].i64;


    if (is_data) {
        const char *data = invocation->arguments[1].string;
        size_t data_length = strlen(data);
        if (start > data_length) {
            if (error_message) {
                *error_message = error_message_new_format("start offset out of range: '%" PRIu64 "'", start);
            }
            return false;
        }
        if (len < 0) {
            len = data_length - start;
        }
        else if (start + len > data_length) {
            if (error_message) {
                *error_message = error_message_new_format("length out of range: '%" PRId64 "'", len);
            }
            return false;
        }
        source = zip_source_buffer(archive, data + start, len, 0);
    }
    else {
        source = zip_source_file(archive, invocation->arguments[1].string, start, len);
    }
    if (source == NULL) {
        if (error_message) {
            *error_message = error_message_new_format("failed to create zip source: '%s'", zip_strerror(archive));
        }
        return false;
    }
    if (zip_file_add(archive, invocation->arguments[1].string, source, get_flags(invocation)) < 0) {
        if (error_message) {
            *error_message = error_message_new_format("failed to add file to archive: '%s'", zip_strerror(archive));
        }
        zip_source_free(source);
        return false;
    }

    return true;
}

/* clang-format off */
argument_definition_t add_arguments[] = {
    {"name", "name of new file in archive", argument_type_string, true, {.string = NULL}},
    {"file", "file or data to add", argument_type_filename, true, {.string = NULL}},
    {"start", "starting offset in the file", argument_type_uint64, false, {.u64 = 0}},
    {"length", "number of bytes to read from the file", argument_type_int64, false, {.i64 = -1}},
};

option_definition_t add_options[] = {
    ENCODING_FLAGS_OPTIONS,
    {'d', "data", NULL, ""}
};
/* clang-format on */

COMMAND(add, "add file to archive");
