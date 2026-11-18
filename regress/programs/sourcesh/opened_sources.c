#include "sourcesh.h"

#include <stdlib.h>
#include <string.h>

struct opened_source_t {
    char *name;
    zip_source_t *source;
    struct opened_source_t *next;
};

typedef struct opened_source_t opened_source_t;

opened_source_t opened_sources = {NULL, NULL, NULL};

zip_source_t *find_opened_source(const char *name, error_message_t **error) {
    for (opened_source_t *p = opened_sources.next; p != NULL; p = p->next) {
        if (p->name && strcmp(p->name, name) == 0) {
            if (p->source == NULL) {
                *error = error_message_new_format("source named '%s' has been freed", name);
            }
            return p->source;
        }
    }

    *error = error_message_new_format("no source named '%s'", name);
    return NULL;
}

bool add_opened_source(const char *name, zip_source_t *source, error_message_t **error) {
    opened_source_t *new_source;

    for (opened_source_t *p = opened_sources.next; p != NULL; p = p->next) {
        if (p->name && strcmp(p->name, name) == 0) {
            if (p->source == NULL) {
                p->source = source;
                return true;
            }
            *error = error_message_new_format("source named '%s' already exists", name);
            return false;
        }
    }

    if ((new_source = malloc(sizeof(opened_source_t))) == NULL) {
        *error = error_message_new_format("out of memory");
        return false;
    }
    new_source->name = strdup(name);
    if (!new_source->name) {
        free(new_source);
        *error = error_message_new_format("out of memory");
        return false;
    }
    new_source->source = source;
    new_source->next = opened_sources.next;
    opened_sources.next = new_source;
    return true;
}

bool free_opened_source(zip_source_t *source, error_message_t **error) {
    for (opened_source_t *p = opened_sources.next; p != NULL; p = p->next) {
        if (p->source == source) {
            zip_source_free(p->source);
            p->source = NULL;
            return true;
        }
    }
    *error = error_message_new_format("no such source");
    return false;
}

void free_all_opened_sources() {
    opened_source_t *p = opened_sources.next;
    while (p != NULL) {
        opened_source_t *next = p->next;
        if (p->source) {
            zip_source_free(p->source);
        }
        free(p->name);
        free(p);
        p = next;
    }
    opened_sources.next = NULL;
}

bool is_source_opened(const char *name) {
    for (opened_source_t *p = opened_sources.next; p != NULL; p = p->next) {
        if (p->name && strcmp(p->name, name) == 0) {
            return p->source != NULL;
        }
    }
    return false;
}

bool convert_source(const argument_type_t *type, argument_t *argument, const char *value, error_message_t **error) {
    zip_source_t *source = find_opened_source(value, error);
    if (!source) {
        return false;
    }
    argument->pointer = source;
    return true;
}

void complete_source(completions_t *completions, const argument_type_t *type, const char *str) {
    size_t str_len = strlen(str);
    for (opened_source_t *p = opened_sources.next; p != NULL; p = p->next) {
        if (p->name && (str == NULL || strncmp(p->name, str, str_len) == 0)) {
            completions_add(completions, p->name);
        }
    }
}

/* clang-format off */
argument_type_t source_argument_type_instance = {
    "source", convert_source, complete_source, NULL, {.pointer = NULL}
};
/* clang-format on */
