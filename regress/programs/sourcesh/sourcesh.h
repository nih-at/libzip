#ifndef HAD_SOURCESH_H
#define HAD_SOURCESH_H

#include "shell.h"
#include "zip.h"

zip_source_t *find_opened_source(const char *name, error_message_t **error);
bool is_source_opened(const char *name);
bool add_opened_source(const char *name, zip_source_t *source, error_message_t **error);
bool free_opened_source(zip_source_t *source, error_message_t **error);
void free_all_opened_sources();

extern argument_type_t source_argument_type_instance;
#define argument_type_source &source_argument_type_instance

#define ARGUMENT_SOURCE 0

#define SOURCE_ARGUMENTS \
    { "source", "source to use", argument_type_source, ARGUMENT_REQUIRED, {.pointer = NULL } }

#define CREATE_SOURCE_ARGUMENTS \
    { "source", "source to use", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL } }

extern const argument_type_t argument_type_seek_whence_instance;
#define argument_type_seek_whence &argument_type_seek_whence_instance

#endif /* HAD_SOURCESH_H */
