#ifndef ZIPSH_H
#define ZIPSH_H

#include "shell.h" /* Include first, so we include compat.h before zip.h. */

#include <zip.h>

extern zip_t *archive;
extern zip_flags_t default_flags;

enum long_options {
    OPTION_CASE_INSENSITIVE = 300,
    OPTION_CASE_SENSITIVE,
    OPTION_CHANGED,
    OPTION_ENCODING,
    OPTION_IGNORE_DIRECTORY,
    OPTION_ORIGINAL,
    OPTION_NO_IGNORE_DIRECTORY
};

#define ENCODING_FLAGS_OPTIONS \
    {OPTION_ENCODING, "encoding", "encoding", "specify encoding for file name lookup", argument_type_encoding}

#define EXTRA_FIELD_OPTIONS \
    {'c', "central", NULL, "operate on central directory extra fields", NULL}, \
    {'l', "local", NULL, "operate on local extra fields", NULL}

#define EXTRA_FIELD_ID_OPTIONS \
    EXTRA_FIELD_OPTIONS, \
    {'I', "id", "id", "specify extra field ID", argument_type_uint16}

#define FLAGS_OPTIONS \
    ENCODING_FLAGS_OPTIONS, \
    {OPTION_CASE_INSENSITIVE, "case-insensitive", NULL, "ignore case in file name lookup", NULL}, \
    {OPTION_CASE_SENSITIVE, "case-sensitive", NULL, "match case in file name lookup", NULL}, \
    {OPTION_CHANGED, "changed", NULL, "operate on changed files", NULL}, \
    {OPTION_IGNORE_DIRECTORY, "ignore-directory", NULL, "ignore directories in file name lookup", NULL}, \
    {OPTION_ORIGINAL, "original", NULL, "operate on original files", NULL}, \
    {OPTION_NO_IGNORE_DIRECTORY, "no-ignore-directory", NULL, "consider directories in file name lookup", NULL}

#define FILE_REFERENCE_OPTIONS \
    FLAGS_OPTIONS, \
    {'i', "index", NULL, "file argument is index instead of name", NULL}

#define FILE_REFERENCE_ARGUMENTS \
    {"file", "file or index to operate on", argument_type_archive_filename, true, {.string = NULL}}

extern const argument_type_t argument_type_compression_method_instance;
extern const argument_type_t argument_type_encryption_method_instance;
extern const argument_type_t argument_type_archive_filename_instance;
extern const argument_type_t argument_type_encoding_instance;

#define argument_type_compression_method (&argument_type_compression_method_instance)
#define argument_type_encryption_method (&argument_type_encryption_method_instance)
#define argument_type_archive_filename (&argument_type_archive_filename_instance)
#define argument_type_encoding (&argument_type_encoding_instance)


struct file_reference {
    zip_flags_t flags;
    const char *name;
    zip_uint64_t index;
    char *error_info;
};

typedef struct file_reference file_reference_t;

file_reference_t *file_reference_create(invocation_t *invocation, error_message_t **error);
void file_reference_free(file_reference_t *file_ref);
zip_flags_t get_flags(invocation_t *invocation);

char *decode_hex(const char *hex, error_message_t **error_message);
void print_hex(const char *data, size_t len);
const char* format_mtime(time_t mtime);

#endif /* ZIPSH_H */
