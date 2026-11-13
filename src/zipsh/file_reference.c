#include <stdlib.h>
#include <string.h>

#include "compat/src_compat.h"
#include "zip.h"
#include "zipsh.h"

static bool file_matches_grow(file_matches_t *file_matches, size_t new_count, error_message_t **error) {
    size_t old_count = file_matches->count;
    file_reference_t *new_references = realloc(file_matches->references, new_count * sizeof(file_reference_t));
    if (!new_references) {
        *error = error_message_new("out of memory");
        return false;
    }
    file_matches->references = new_references;
    for (size_t i = old_count; i < new_count; i++) {
        file_matches->references[i].name = NULL;
        file_matches->references[i].index = 0;
        file_matches->references[i].error_info = NULL;
    }
    file_matches->count = new_count;
    return true;
}

void file_reference_init(file_reference_t *file_ref, const char *name, zip_uint64_t index) {
    file_ref->name = name;
    file_ref->index = index;

    if (name) {
        asprintf(&file_ref->error_info, "'%s'", name);
    }
    else {
        asprintf(&file_ref->error_info, "at index %" PRIu64, file_ref->index);
    }
}

void complete_archive_filename(completions_t *completions, const argument_type_t *type, const char *str) {
    size_t str_length = strlen(str);
    zip_int64_t i, n;

    if ((n = zip_get_num_entries(archive, 0)) < 0) {
        return;
    }

    for (i = 0; i < n; i++) {
        const char *name = zip_get_name(archive, i, 0);
        if (name && strncmp(name, str, str_length) == 0) {
            completions_add(completions, name);
        }
    }
    return;
}

const argument_type_t argument_type_archive_filename_instance = {"filename", NULL, complete_archive_filename, NULL, {.pointer = NULL}};

file_matches_t *file_matches_create(invocation_t *invocation, error_message_t **error) {
    argument_t *index_option = invocation_get_option(invocation, 'i');
    file_matches_t *file_matches = (file_matches_t *)malloc(sizeof(file_matches_t));
    if (!file_matches) {
        *error = error_message_new("out of memory");
        return NULL;
    }

    file_matches->flags = get_flags(invocation);
    file_matches->references = NULL;
    file_matches->count = 0;

    if (index_option && index_option->boolean) {
        argument_t argument;
        if (!convert_argument(argument_type_uint64, &argument, invocation->arguments[0].string, error)) {
            file_matches_free(file_matches);
            return NULL;
        }
        if (!file_matches_grow(file_matches, file_matches->count + 1, error)) {
            file_matches_free(file_matches);
            return NULL;
        }
        file_reference_init(&file_matches->references[0], NULL, argument.u64);
    }
    else if (is_pattern(invocation->arguments[0].string)) {
        zip_int64_t count = 0;
        zip_int64_t n = zip_get_num_entries(archive, 0);

        if (n < 0) {
            *error = error_message_new("failed to get number of entries");
            file_matches_free(file_matches);
            return NULL;
        }

        for (zip_int64_t i = 0; i < n; i++) {
            const char *name = zip_get_name(archive, i, 0);
            if (name && fnmatch(invocation->arguments[0].string, name, 0) == 0) {
                count += 1;
            }
        }

        if (count == 0) {
            *error = error_message_new_format("no file matching '%s' found", invocation->arguments[0].string);
            file_matches_free(file_matches);
            return NULL;
        }

        if (!file_matches_grow(file_matches, count, error)) {
            file_matches_free(file_matches);
            return NULL;
        }

        count = 0;
        for (zip_int64_t i = 0; i < n; i++) {
            const char *name = zip_get_name(archive, i, 0);
            if (name && fnmatch(invocation->arguments[0].string, name, 0) == 0) {
                file_reference_init(&file_matches->references[count], name, i);
                count += 1;
            }
        }
    }
    else {
        zip_int64_t idx = zip_name_locate(archive, invocation->arguments[0].string, file_matches->flags);

        if (idx < 0) {
            *error = error_message_new_format("file '%s' not found", invocation->arguments[0].string);
            file_matches_free(file_matches);
            return NULL;
        }

        if (!file_matches_grow(file_matches, 1, error)) {
            file_matches_free(file_matches);
            return NULL;
        }
        file_reference_init(&file_matches->references[0], invocation->arguments[0].string, idx);
    }

    return file_matches;
}


void file_matches_free(file_matches_t *file_matches) {
    if (file_matches == NULL) {
        return;
    }
    for (size_t i = 0; i < file_matches->count; i++) {
        free(file_matches->references[i].error_info);
    }
    free(file_matches->references);
    free(file_matches);
}

bool file_reference_command_execute(file_reference_command_t *command, invocation_t *invocation, error_message_t **error_message) {
    bool result = true;
    file_matches_t *matches = file_matches_create(invocation, error_message);
    if (!matches) {
        return false;
    }

    if (command->setup && !command->setup(invocation, command->context, matches, error_message)) {
        file_matches_free(matches);
        result = false;
    }
    else {
        for (size_t i = 0; i < matches->count; i++) {
            if (!command->execute(invocation, command->context, matches, &matches->references[i], error_message)) {
                /* TODO: add file info to error message */
                result = false;
                break;
            }
        }
    }
    if (command->cleanup && !command->cleanup(invocation, command->context, matches, error_message)) {
        result = false;
    }

    file_matches_free(matches);
    return result;
}

size_t file_matches_get_index(const file_matches_t *file_matches, const file_reference_t *file_ref) {
    return file_ref - file_matches->references;
}

bool file_matches_is_multiple(const file_matches_t *file_matches) {
    return file_matches->count > 1;
}
