#ifndef SHELL_UTIL_H
#define SHELL_UTIL_H

/* This library provides utility functions shared between zipsh and sourcesh. */

#include "shell.h"

#include "zip.h"

extern const argument_type_t argument_type_compression_method_instance;
extern const argument_type_t argument_type_encryption_method_instance;
extern const argument_type_t argument_type_encoding_instance;

#define argument_type_compression_method (&argument_type_compression_method_instance)
#define argument_type_encryption_method (&argument_type_encryption_method_instance)
#define argument_type_encoding (&argument_type_encoding_instance)

zip_uint8_t *decode_hex(const char *hex, error_message_t **error_message);
void print_hex(const zip_uint8_t *data, size_t len);

void print_stat(const zip_stat_t *stat);
const char* format_mtime(time_t mtime);

#endif /* SHELL_UTIL_H */
