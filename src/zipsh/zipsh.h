#ifndef ZIPSH_H
#define ZIPSH_H

#include <zip.h>

#include "shell.h"

extern zip_t *archive;

struct file_reference {
    const char *name;
    zip_uint64_t index;
    char *error_info;
};

typedef struct file_reference file_reference_t;

file_reference_t *file_reference_create(invocation_t *invocation, error_message_t **error);
void file_reference_free(file_reference_t *file_ref);

#endif /* ZIPSH_H */
