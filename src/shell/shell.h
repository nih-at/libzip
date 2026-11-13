#ifndef SHELL_H
#define SHELL_H

#include "compat.h"

#include <stdio.h>

struct error_message {
    char *message;
    bool free_message;
};

typedef struct error_message error_message_t;

struct completions {
    char *prefix;
    char **matches;
    size_t n_matches;
    size_t matches_capacity;
};

typedef struct completions completions_t;


#if defined(__GNUC__) && __GNUC__ >= 4
#define PRINTF_LIKE(n, m) __attribute__((__format__(__printf__, n, m)))
#else
#define PRINTF_LIKE(n, m)
#endif

union argument {
    bool boolean;
    const char *string;
    zip_int8_t i8;
    zip_int16_t i16;
    zip_int32_t i32;
    zip_int64_t i64;
    zip_uint8_t u8;
    zip_uint16_t u16;
    zip_uint32_t u32;
    zip_uint64_t u64;
    void *pointer;
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

typedef struct argument_type argument_type_t;

struct argument_type {
    char *name;
    bool (*convert)(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message);
    void (*complete)(completions_t *completions, const argument_type_t *type, const char *str);
    void (*free_ud)(argument_t ud);
    argument_t ud;
};

enum argument_cardinality {
    ARGUMENT_OPTIONAL,
    ARGUMENT_REQUIRED,
    ARGUMENT_COLLECT
};

typedef enum argument_cardinality argument_cardinality_t;

struct argument_definition {
    const char *name;
    const char *help;
    const argument_type_t *type;
    argument_cardinality_t cardinality;
    argument_t default_value;
};

typedef struct argument_definition argument_definition_t;

struct option_definition {
    int id; /* short name or unique value > 255 */
    const char *long_name;
    const char *argument_name;
    const char *help;
    const argument_type_t *argument_type;
};

typedef struct option_definition option_definition_t;

struct command {
    const char *name;
    const char *help;
    option_definition_t *options;
    size_t n_options;
    const argument_definition_t *arguments;
    size_t n_arguments;
    bool (*function)(invocation_t *invocation, error_message_t **error_message);
};

typedef struct command command_t;

struct words {
    char **words;       /* array of parsed words */
    size_t n_words;     /* number of parsed words */
    char open_quote;    /* currently open quote character, NUL if none (only for partial parse) */

    size_t words_capacity;  /* capacity of the words array */
    char *text;             /* buffer holding the parsed text */
    size_t text_capacity;   /* capacity of the text buffer */
    size_t next_word_start; /* start index of the word currently being parsed in the text buffer */
    size_t next_char;       /* index of the next character to be parsed in the text buffer */
};

typedef struct words words_t;

extern const argument_type_t argument_type_bool_instance;
extern const argument_type_t argument_type_string_instance;
extern const argument_type_t argument_type_int8_instance;
extern const argument_type_t argument_type_int16_instance;
extern const argument_type_t argument_type_int32_instance;
extern const argument_type_t argument_type_int64_instance;
extern const argument_type_t argument_type_uint8_instance;
extern const argument_type_t argument_type_uint16_instance;
extern const argument_type_t argument_type_uint32_instance;
extern const argument_type_t argument_type_uint64_instance;
extern const argument_type_t argument_type_filename_instance;
#define argument_type_bool (&argument_type_bool_instance)
#define argument_type_string (&argument_type_string_instance)
#define argument_type_int8 (&argument_type_int8_instance)
#define argument_type_int16 (&argument_type_int16_instance)
#define argument_type_int32 (&argument_type_int32_instance)
#define argument_type_int64 (&argument_type_int64_instance)
#define argument_type_uint8 (&argument_type_uint8_instance)
#define argument_type_uint16 (&argument_type_uint16_instance)
#define argument_type_uint32 (&argument_type_uint32_instance)
#define argument_type_uint64 (&argument_type_uint64_instance)
#define argument_type_filename (&argument_type_filename_instance)

struct argument_type_enum_entry {
    const char *string;
    argument_t value;
};

typedef struct argument_type_enum_entry argument_type_enum_entry_t;



#define ARGUMENT_TYPE_ENUM(name, entries) { name, convert_enum, complete_enum, NULL, {.pointer=entries} }

bool convert_enum(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message);
void complete_enum(completions_t *completions, const argument_type_t *type, const char *str);


#define COMMAND(name, help) command_t command_##name = {#name, help, name##_options, sizeof(name##_options) / sizeof(name##_options[0]), name##_arguments, sizeof(name##_arguments) / sizeof(name##_arguments[0]), name##_implementation}
#define COMMAND_NO_ARGUMENTS(name, help) command_t command_##name = {#name, help, name##_options, sizeof(name##_options) / sizeof(name##_options[0]), NULL, 0, name##_implementation}
#define COMMAND_NO_ARGUMENTS_OR_OPTIONS(name, help) command_t command_##name = {#name, help, NULL, 0, NULL, 0, name##_implementation}
#define COMMAND_NO_OPTIONS(name, help) command_t command_##name = {#name, help, NULL, 0, name##_arguments, sizeof(name##_arguments) / sizeof(name##_arguments[0]), name##_implementation}

extern command_t *commands[];
extern size_t n_commands;

bool execute_command(const command_t *cmd, int argc, char **argv, error_message_t **error_message);
const command_t *find_command(const char *name);

bool convert_argument(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message);
completions_t *complete_argument(const argument_type_t *type, const char *str, size_t prefix_length);
option_definition_t *command_find_short_option(const command_t *cmd, char short_name);
option_definition_t *command_find_long_option(const command_t *cmd, const char *long_name, size_t long_name_len);

error_message_t *error_message_new(const char *message);
error_message_t *error_message_new_format(const char *fmt, ...) PRINTF_LIKE(1, 2);
const char *error_message_get(error_message_t *error);
void error_message_free(error_message_t *error);

invocation_t *invocation_new(const command_t *cmd, int *early_exit_options, size_t argc, char **argv, error_message_t **error_message, completions_t **completions);
void invocation_free(invocation_t *invocation);
bool invocation_add_option(invocation_t *invocation, int id, argument_t value);
bool invocation_add_argument(invocation_t *invocation, argument_t value);
argument_t *invocation_get_option(invocation_t *invocation, int id);

bool shell_loop(const char *script_filename);

words_t *words_new(void);
void words_free(words_t *words);
bool words_parse(words_t *words, const char *line, error_message_t **error_message);
bool words_parse_partial(words_t *words, const char *line, const char *end, error_message_t **error_message);
void words_clear(words_t *words);

bool print_command_help(const command_t *cmd, const char *prog_name, error_message_t **error_message);

const char* enum_argument_to_string(const argument_type_t *type, argument_t value, bool string_for_unknown);

#define VERSION_OPTION_ID 256

#define SHELL_OPTIONS \
    {'c', "command", NULL, "Execute command from command line", NULL}, \
    {'h', "help", NULL, "Show help message", NULL}, \
    {VERSION_OPTION_ID, "version", NULL, "Show version information", NULL}

#define SHELL_ARGUMENT \
    {"command|file", "Command to execute or file to read commands from", argument_type_string, ARGUMENT_COLLECT, {.string = NULL}}

struct shell_command {
    command_t command;
    int shell_argument_index;
    bool (*setup)(invocation_t *, error_message_t **);
    bool (*cleanup)(bool ok, error_message_t **);
};

typedef struct shell_command shell_command_t;

#define SHELL_MAIN(name, help, index) \
    shell_command_t shell_command_##name = {{#name, help, name##_options, sizeof(name##_options) / sizeof(name##_options[0]), name##_arguments, sizeof(name##_arguments) / sizeof(name##_arguments[0]), NULL}, index, name##_setup, name##_cleanup};

    int shell_main(shell_command_t *shell, int argc, char *argv[]);

bool is_pattern(const char *str);

completions_t *completions_new(const char *prefix, size_t prefix_length);
void completions_free(completions_t *completions);
void completions_sort(completions_t *completions);
bool completions_add(completions_t *completions, const char *match);
char **completions_get_matches(const completions_t *completions);

#endif /* SHELL_H */
