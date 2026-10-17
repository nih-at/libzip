#include <stdlib.h>

#include "zipsh.h"

file_reference_t *file_reference_create(invocation_t *invocation, error_message_t **error) {
    argument_t *index_option = invocation_get_option(invocation, 'i');
    file_reference_t *file_ref = (file_reference_t *)malloc(sizeof(file_reference_t));
    if (!file_ref) {
        *error = error_message_new("out of memory");
        return NULL;
    }

    file_ref->name = NULL;
    file_ref->error_info = NULL;

    if (index_option && index_option->b) {
        if (!convert_uint64(&file_ref->index, invocation->arguments[0].string, error)) {
            free(file_ref);
            return NULL;
        }
        asprintf(&file_ref->error_info, "at index %" PRIu64, file_ref->index);
    }
    else {
        zip_int64_t idx = zip_name_locate(archive, invocation->arguments[0].string, 0);
        file_ref->index = (zip_uint64_t)idx;
        if (idx < 0) {
            *error = error_message_new_format("file '%s' not found", invocation->arguments[0].string);
            free(file_ref);
            return NULL;
        }
        asprintf(&file_ref->error_info, "'%s'", invocation->arguments[0].string);
        file_ref->name = invocation->arguments[0].string;
    }

    return file_ref;
}

void file_reference_free(file_reference_t *file_ref) {
    if (file_ref == NULL) {
        return;
    }
    free(file_ref->error_info);
    free(file_ref);
}
