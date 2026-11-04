#ifndef ZIPSH_H
#define ZIPSH_H

#include <zip.h>

#include "shell.h"

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

#define FLAGS_OPTIONS \
    {OPTION_CASE_INSENSITIVE, "case-insensitive", NULL, "ignore case in file name lookup", NULL}, \
    {OPTION_CASE_SENSITIVE, "case-sensitive", NULL, "match case in file name lookup", NULL}, \
    {OPTION_CHANGED, "changed", NULL, "operate on changed files", NULL}, \
    {OPTION_ENCODING, "encoding", "encoding", "specify encoding for file name lookup", argument_type_encoding}, \
    {OPTION_IGNORE_DIRECTORY, "ignore-directory", NULL, "ignore directories in file name lookup", NULL}, \
    {OPTION_ORIGINAL, "original", NULL, "operate on original files", NULL}, \
    {OPTION_NO_IGNORE_DIRECTORY, "no-ignore-directory", NULL, "consider directories in file name lookup", NULL}

#define FILE_REFERENCE_OPTIONS \
    FLAGS_OPTIONS, \
    {'i', "index", NULL, "file argument is index instead of name", NULL}

extern const argument_type_t argument_type_compression_method;
extern const argument_type_t argument_type_encryption_method;
extern const argument_type_t argument_type_filename_instance;
extern const argument_type_t argument_type_encoding_instance;

#define argument_type_compression_method (&argument_type_compression_method)
#define argument_type_encryption_method (&argument_type_encryption_method)
#define argument_type_filename (&argument_type_filename_instance)
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

#endif /* ZIPSH_H */
