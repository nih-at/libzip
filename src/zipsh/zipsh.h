#ifndef ZIPSH_H
#define ZIPSH_H

#include "shell.h" /* Include first, so we include compat.h before zip.h. */

#include <zip.h>

extern zip_t *archive;
extern zip_flags_t default_flags;
extern const char *archive_name;

enum long_options {
    OPTION_CASE_INSENSITIVE = 300,
    OPTION_CASE_SENSITIVE,
    OPTION_CHANGED,
    OPTION_ENCODING,
    OPTION_IGNORE_DIRECTORY,
    OPTION_ORIGINAL,
    OPTION_NO_IGNORE_DIRECTORY,
    OPTION_READ_ONLY
};

#define OPTION_CHECK_CONSISTENCY 'C'

#define ZIPSH_OPTIONS \
    {OPTION_CHECK_CONSISTENCY, "check-consistency", "Check consistency of the archive"}, \
    {OPTION_READ_ONLY, "read-only", "Open the archive in read-only mode"}

#define ENCODING_FLAGS_OPTIONS \
    {OPTION_ENCODING, "encoding", "encoding", "specify encoding for file name lookup", argument_type_encoding}

#define CHANGED_FLAGS_OPTIONS \
    {OPTION_CHANGED, "changed", NULL, "operate on changed files", NULL}, \
    {OPTION_ORIGINAL, "original", NULL, "operate on original files", NULL}

#define EXTRA_FIELD_OPTIONS \
    {'c', "central", NULL, "operate on central directory extra fields", NULL}, \
    {'l', "local", NULL, "operate on local extra fields", NULL}

#define EXTRA_FIELD_ID_OPTIONS \
    EXTRA_FIELD_OPTIONS, \
    {'I', "id", "id", "specify extra field ID", argument_type_uint16}

#define FLAGS_OPTIONS \
    ENCODING_FLAGS_OPTIONS, \
    CHANGED_FLAGS_OPTIONS, \
    {OPTION_CASE_INSENSITIVE, "case-insensitive", NULL, "ignore case in file name lookup", NULL}, \
    {OPTION_CASE_SENSITIVE, "case-sensitive", NULL, "match case in file name lookup", NULL}, \
    {OPTION_IGNORE_DIRECTORY, "ignore-directory", NULL, "ignore directories in file name lookup", NULL}, \
    {OPTION_NO_IGNORE_DIRECTORY, "no-ignore-directory", NULL, "consider directories in file name lookup", NULL}

#define FILE_REFERENCE_OPTIONS \
    FLAGS_OPTIONS, \
    {'i', "index", NULL, "file argument is index instead of name", NULL}

#define FILE_REFERENCE_ARGUMENTS \
    {"file|index", "file or index to operate on", argument_type_archive_filename, ARGUMENT_REQUIRED, {.string = NULL}}

extern const argument_type_t argument_type_compression_method_instance;
extern const argument_type_t argument_type_encryption_method_instance;
extern const argument_type_t argument_type_archive_filename_instance;
extern const argument_type_t argument_type_encoding_instance;

#define argument_type_compression_method (&argument_type_compression_method_instance)
#define argument_type_encryption_method (&argument_type_encryption_method_instance)
#define argument_type_archive_filename (&argument_type_archive_filename_instance)
#define argument_type_encoding (&argument_type_encoding_instance)

struct file_reference {
    const char *name;
    zip_uint64_t index;
    char *error_info;
};

typedef struct file_reference file_reference_t;

struct file_matches {
    zip_flags_t flags;
    file_reference_t *references;
    size_t count;
};

typedef struct file_matches file_matches_t;

struct extra_field_options {
    zip_uint16_t id;
    bool id_specified;
    zip_flags_t flags;
};

typedef struct extra_field_options extra_field_options_t;

file_matches_t *file_matches_create(invocation_t *invocation, error_message_t **error);
void file_matches_free(file_matches_t *file_matches);
size_t file_matches_get_index(const file_matches_t *file_matches, const file_reference_t *file_ref);
bool file_matches_is_multiple(const file_matches_t *file_matches);
zip_flags_t get_flags(invocation_t *invocation);
extra_field_options_t *extra_field_options_create(invocation_t *invocation, error_message_t **error_message);
void extra_field_options_free(extra_field_options_t *extra_field_options);

zip_uint8_t *decode_hex(const char *hex, error_message_t **error_message);
void print_hex(const zip_uint8_t *data, size_t len);
const char* format_mtime(time_t mtime);

struct file_reference_command {
    bool (*setup)(invocation_t *invocation, void *context, file_matches_t *file_matches, error_message_t **error_message);
    bool (*execute)(invocation_t *invocation, void *context, file_matches_t *file_matches, file_reference_t *file_ref, error_message_t **error_message);
    bool (*cleanup)(invocation_t *invocation, void *context, file_matches_t *file_matches, error_message_t **error_message);
    void *context;
};

typedef struct file_reference_command file_reference_command_t;

bool file_reference_command_execute(file_reference_command_t *command, invocation_t *invocation, error_message_t **error_message);
int get_archive_flags(invocation_t *invocation);
bool open_archive(invocation_t *invocation, error_message_t **error_message);
bool close_archive(error_message_t **error_message);

#endif /* ZIPSH_H */
