#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>

#include "compat.h"

struct error_message {
    char *message;
    bool free_message;
};

typedef struct error_message error_message_t;

#if defined(__GNUC__) && __GNUC__ >= 4
#define PRINTF_LIKE(n, m) __attribute__((__format__(__printf__, n, m)))
#else
#define PRINTF_LIKE(n, m)
#endif


enum argument_type {
    ARGUMENT_TYPE_BOOL,
    ARGUMENT_TYPE_STRING,
    ARGUMENT_TYPE_INT8,
    ARGUMENT_TYPE_INT16,
    ARGUMENT_TYPE_INT32,
    ARGUMENT_TYPE_INT64,
    ARGUMENT_TYPE_UINT8,
    ARGUMENT_TYPE_UINT16,
    ARGUMENT_TYPE_UINT32,
    ARGUMENT_TYPE_UINT64,

    ARGUMENT_TYPE_COMPRESSION_METHOD,
    ARGUMENT_TYPE_ENCRYPTION_METHOD
};

typedef enum argument_type argument_type_t;

union argument {
    bool b;
    const char *string;
    zip_int8_t i8;
    zip_int16_t i16;
    zip_int32_t i32;
    zip_int64_t i64;
    zip_uint8_t u8;
    zip_uint16_t u16;
    zip_uint32_t u32;
    zip_uint64_t u64;
};

typedef union argument argument_t;

struct parsed_option {
    int id;
    argument_t value;
};

typedef struct parsed_option option_t;

struct invocation {
    size_t n_options;
    size_t options_capacity;
    option_t *options;
    size_t n_arguments;
    size_t arguments_capacity;
    argument_t *arguments;
};
typedef struct invocation invocation_t;


struct argument_definition {
    const char *name;
    const char *help;
    argument_type_t type;
    bool required;
    argument_t default_value;
};

typedef struct argument_definition argument_definition_t;

struct option_definition {
    int id; /* short name or unique value > 255 */
    const char *long_name;
    const char *argument_name;
    const char *help;
    argument_type_t argument_type;
};

typedef struct option_definition option_definition_t;

struct command {
    const char *name;
    const char *help;
    option_definition_t *options;
    size_t n_options;
    argument_definition_t *arguments;
    size_t n_arguments;
    bool (*function)(invocation_t *invocation, error_message_t **error_message);
};

typedef struct command command_t;

struct words {
    char **words;
    size_t n_words;

    size_t words_capacity;
    char *text;
    size_t text_capacity;
    size_t next_word_start;
    size_t next_char;
};
typedef struct words words_t;


#define COMMAND(name, help) command_t command_##name = {#name, help, name##_options, sizeof(name##_options) / sizeof(name##_options[0]), name##_arguments, sizeof(name##_arguments) / sizeof(name##_arguments[0]), name##_implementation}
#define COMMAND_NO_OPTIONS(name, help) command_t command_##name = {#name, help, NULL, 0, name##_arguments, sizeof(name##_arguments) / sizeof(name##_arguments[0]), name##_implementation}

extern command_t *commands[];
extern size_t n_commands;

bool execute_command(const command_t *cmd, int argc, char **argv, error_message_t **error_message);
const command_t *find_command(const char *name);

bool convert_argument(const char *str, argument_t *arg, argument_type_t type, error_message_t **error_message);

#define convert_int8(out, str, error_message) convert_int(out, str, sizeof(int8_t), error_message)
#define convert_int16(out, str, error_message) convert_int(out, str, sizeof(int16_t), error_message)
#define convert_int32(out, str, error_message) convert_int(out, str, sizeof(int32_t), error_message)
#define convert_int64(out, str, error_message) convert_int(out, str, sizeof(int64_t), error_message)
#define convert_uint8(out, str, error_message) convert_uint(out, str, sizeof(uint8_t), error_message)
#define convert_uint16(out, str, error_message) convert_uint(out, str, sizeof(uint16_t), error_message)
#define convert_uint32(out, str, error_message) convert_uint(out, str, sizeof(uint32_t), error_message)
#define convert_uint64(out, str, error_message) convert_uint(out, str, sizeof(uint64_t), error_message)

bool convert_int(void *out, const char *str, int size, error_message_t **error_message);
bool convert_uint(void *out, const char *str, int size, error_message_t **error_message);

error_message_t *error_message_new(const char *message);
error_message_t *error_message_new_format(const char *fmt, ...) PRINTF_LIKE(1, 2);
const char *error_message_get(error_message_t *error);
void error_message_free(error_message_t *error);

invocation_t *invocation_new(size_t options_capacity, size_t arguments_capacity);
void invocation_free(invocation_t *invocation);
bool invocation_add_option(invocation_t *invocation, int id, argument_t value);
bool invocation_add_argument(invocation_t *invocation, argument_t value);
argument_t *invocation_get_option(invocation_t *invocation, int id);

bool shell_loop(const char *script_filename);

words_t *words_new(void);
void words_free(words_t *words);
bool words_parse(words_t *words, const char *line, error_message_t **error_message);
void words_clear(words_t *words);

#endif /* SHELL_H */
