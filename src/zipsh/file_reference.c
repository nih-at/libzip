#include <stdlib.h>
#include <string.h>

#include "compat/src_compat.h"
#include "zip.h"
#include "zipsh.h"

char **complete_filename(const argument_type_t *type, const char *str) {
    size_t str_length = strlen(str);
    zip_uint64_t count = 0;
    zip_int64_t i, n;
    char **matches;

    if ((n = zip_get_num_entries(archive, 0)) < 0) {
        return no_matches();
    }

    for (i = 0; i < n; i++) {
        const char *name = zip_get_name(archive, i, 0);
        if (name && strncmp(name, str, str_length) == 0) {
            count++;
        }
    }

    matches = malloc((count + 1) * sizeof(char *));
    if (matches == NULL) {
        return NULL;
    }
    count = 0;
    for (i = 0; i < n; i++) {
        const char *name = zip_get_name(archive, i, 0);
        if (name && strncmp(name, str, str_length) == 0) {
            matches[count++] = strdup(name);
        }
    }
    printf("Found %" PRIu64 " matches for '%s'\n", count, str);
    matches[count] = NULL;
    return matches;
}

const argument_type_t argument_type_filename_instance = {"filename", NULL, complete_filename, NULL, {.pointer = NULL}};

file_reference_t *file_reference_create(invocation_t *invocation, error_message_t **error) {
    argument_t *index_option = invocation_get_option(invocation, 'i');
    file_reference_t *file_ref = (file_reference_t *)malloc(sizeof(file_reference_t));
    if (!file_ref) {
        *error = error_message_new("out of memory");
        return NULL;
    }

    file_ref->name = NULL;
    file_ref->error_info = NULL;

    if (index_option && index_option->boolean) {
        argument_t argument;
        if (!convert_argument(argument_type_uint64, &argument, invocation->arguments[0].string, error)) {
            free(file_ref);
            return NULL;
        }
        file_ref->index = argument.u64;
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
