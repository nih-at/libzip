#include "compat/src_compat.h"
#include "shell.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static completions_t *get_option_completions(const command_t *cmd) {
    completions_t *completions = completions_new("-", 1);
    char s[1024];

    for (size_t i = 0; i < cmd->n_options; i++) {
        if (cmd->options[i].id < 256) {
            s[0] = cmd->options[i].id;
            s[1] = '\0';
            completions_add(completions, s);
        }
        if (cmd->options[i].long_name) {
            s[0] = '-';
            strcpy(s + 1, cmd->options[i].long_name);
            completions_add(completions, s);
        }
    }
    return completions;
}

static completions_t *get_argument_completions(const char *prefix, size_t prefix_len, const argument_type_t *argument_type, const char *word) {
    completions_t *completions = completions_new(prefix, prefix_len);
    argument_type->complete(completions, argument_type, word);
    return completions;
}

static completions_t *get_long_option_completions(const command_t *cmd, const char *word) {
    completions_t *completions = completions_new("--", 2);
    size_t word_length = strlen(word);

    for (size_t i = 0; i < cmd->n_options; i++) {
        if (cmd->options[i].long_name && strncmp(cmd->options[i].long_name, word, word_length) == 0) {
            completions_add(completions, cmd->options[i].long_name);
        }
    }
    return completions;
}

static bool add_option(invocation_t *invocation, const option_definition_t *option, const char *word, error_message_t **error_message) {
    error_message_t *local_error_message = NULL;
    argument_t value;

    if (!convert_argument(option->argument_type, &value, word, &local_error_message)) {
        *error_message = error_message_new_format("invalid value for option '%s': %s", word, error_message_get(local_error_message));
        error_message_free(local_error_message);
        return false;
    }
    invocation_add_option(invocation, option->id, value);
    return true;
}

bool is_early_exit_option(int id, const int *early_exit_options) {
    if (early_exit_options == NULL) {
        return false;
    }
    for (size_t i = 0; early_exit_options[i] != 0; i++) {
        if (id == early_exit_options[i]) {
            return true;
        }
    }
    return false;
}

static bool parse_commandline(invocation_t *invocation, const command_t *cmd, int *early_exit_options, int argc, char **argv, error_message_t **error_message, completions_t **completions) {
    bool in_options = true;
    bool complete = (completions != NULL);
    bool early_exit = false;
    size_t argument_index = 0;

    for (size_t i = 1; i < (size_t)argc; i++) {
        const char *word = argv[i];
        bool complete_this_word = (complete && i == (size_t)(argc - 1));

        if (in_options && word[0] == '-' && word[1] != '\0') {
            if (word[1] == '-') {
                char *equals;
                const option_definition_t *option = NULL;

                if (word[2] == '\0') {
                    in_options = false;
                    continue;
                }

                equals = strchr(word, '=');
                if (equals) {
                    option = command_find_long_option(cmd, word + 2, equals - (word + 2));
                    if (!option) {
                        if (!complete) {
                            *error_message = error_message_new_format("unknown option: %.*s", (int)(equals - word), word);
                            return false;
                        }
                    }
                    else {
                        if (complete_this_word) {
                            *completions = get_argument_completions(word, (equals + 1) - word, option->argument_type, equals + 1);
                            return true;
                        }
                        else {
                            if (!add_option(invocation, option, equals + 1, error_message) && !complete) {
                                return false;
                            }
                        }
                    }
                }
                else {
                    if (complete_this_word) {
                        *completions = get_long_option_completions(cmd, word + 2);
                        return true;
                    }
                    else {
                        const option_definition_t *option = command_find_long_option(cmd, word + 2, SIZE_MAX);

                        if (!option) {
                            if (complete) {
                                /* Assume unknown option has no argument. */
                                continue;
                            }
                            else {
                                *error_message = error_message_new_format("unknown option '%s'", word);
                                return false;
                            }
                        }

                        early_exit |= is_early_exit_option(option->id, early_exit_options);

                        if (option->argument_type) {
                            if (i == argc - 1) {
                                *error_message = error_message_new_format("missing value for option '%s'", word);
                                return false;
                            }
                            else {
                                const char *next_word = argv[i + 1];
                                if (complete && i == argc - 2) {
                                    *completions = get_argument_completions("", 0, option->argument_type, next_word);
                                    return true;
                                }
                                else {
                                    if (!add_option(invocation, option, next_word, error_message) && !complete) {
                                        return false;
                                    }
                                }
                            }
                        }
                        else {
                            argument_t value = {.boolean = true};
                            invocation_add_option(invocation, option->id, value);
                        }
                    }
                }
            }
            else {
                for (const char *p = word + 1; *p; p++) {
                    const option_definition_t *option = command_find_short_option(cmd, *p);
                    if (!option) {
                        if (complete) {
                            /* Assume unknown option has no argument. */
                            continue;
                        }
                        else {
                            *error_message = error_message_new_format("unknown option '-%c'", *p);
                            return false;
                        }
                    }

                    early_exit |= is_early_exit_option(option->id, early_exit_options);

                    if (option->argument_type) {
                        if (*(p + 1) == '\0') {
                            if (i == argc - 1) {
                                *error_message = error_message_new_format("missing value for option '-%c'", *p);
                                return false;
                            }
                            else {
                                const char *next_word = argv[i + 1];
                                if (complete && i == argc - 2) {
                                    *completions = get_argument_completions("", 0, option->argument_type, next_word);
                                    return true;
                                }
                                else {
                                    if (!add_option(invocation, option, next_word, error_message) && !complete) {
                                        return false;
                                    }
                                }
                            }
                        }
                        else {
                            if (complete_this_word) {
                                *completions = get_argument_completions(word, (p + 1) - word, option->argument_type, p + 1);
                                return true;
                            }
                            else {
                                if (!add_option(invocation, option, p + 1, error_message) && !complete) {
                                    return false;
                                }
                            }
                            break;
                        }
                    }
                    else {
                        argument_t value = {.boolean = true};
                        invocation_add_option(invocation, option->id, value);
                    }
                }
            }
        }
        else {
            const argument_type_t *argument_type;

            if (word[0] == '-' && complete_this_word) {
                *completions = get_option_completions(cmd);
                return true;
            }

            // in_options = false; /* TODO: Or do we want to allow options after positional arguments? */

            if (argument_index >= cmd->n_arguments) {
                if (!complete) {
                    *error_message = error_message_new("too many arguments");
                    return false;
                }
                /* Can't complete unknown arguments. */
                return true;
            }

            argument_type = cmd->arguments[argument_index].type;
            if (complete_this_word) {
                *completions = get_argument_completions("", 0, argument_type, word);
                return true;
            }
            else {
                error_message_t *local_error_message = NULL;
                argument_t value;
                if (!convert_argument(argument_type, &value, word, &local_error_message)) {
                    if (!complete) {
                        *error_message = error_message_new_format("invalid value for argument %s: %s", cmd->arguments[argument_index].name, error_message_get(local_error_message));
                        error_message_free(local_error_message);
                        return false;
                    }
                }
                invocation_add_argument(invocation, value);
            }
            if (cmd->arguments[argument_index].cardinality != ARGUMENT_COLLECT) {
                argument_index++;
            }
        }
    }

    if (!complete && !early_exit) {
        while (argument_index < cmd->n_arguments) {
            if (cmd->arguments[argument_index].cardinality == ARGUMENT_OPTIONAL) {
                invocation_add_argument(invocation, cmd->arguments[argument_index].default_value);
            }
            else {
                *error_message = error_message_new_format("missing required argument %s", cmd->arguments[argument_index].name);
            }
            argument_index++;
        }
    }

    return true;
}

invocation_t *invocation_new(const command_t *cmd, int *early_exit_options, size_t argc, char **argv, error_message_t **error_message, completions_t **completions) {
    invocation_t *invocation = malloc(sizeof(invocation_t));
    if (invocation == NULL) {
        *error_message = error_message_new("out of memory");
        return NULL;
    }

    invocation->n_options = 0;
    invocation->options = NULL;
    invocation->arguments = NULL;
    invocation->options_capacity = cmd->n_options;
    invocation->arguments_capacity = argc > cmd->n_arguments ? argc : cmd->n_arguments;
    invocation->n_arguments = 0;
    invocation->n_options = 0;

    if (invocation->options_capacity > 0) {
        invocation->options = malloc(invocation->options_capacity * sizeof(option_t));
        if (invocation->options == NULL) {
            invocation_free(invocation);
            *error_message = error_message_new("out of memory");
            return NULL;
        }
    }
    if (invocation->arguments_capacity > 0) {
        invocation->arguments = malloc(invocation->arguments_capacity * sizeof(argument_t));
        if (invocation->arguments == NULL) {
            invocation_free(invocation);
            *error_message = error_message_new("out of memory");
            return NULL;
        }
    }

    if (!parse_commandline(invocation, cmd, early_exit_options, argc, argv, error_message, completions)) {
        invocation_free(invocation);
        return NULL;
    }

    return invocation;
}

void invocation_free(invocation_t *invocation) {
    if (invocation == NULL) {
        return;
    }
    free(invocation->options);
    free(invocation->arguments);
    free(invocation);
}

bool invocation_add_option(invocation_t *invocation, int id, argument_t value) {
    if (invocation->n_options >= invocation->options_capacity) {
        size_t new_capacity = invocation->options_capacity == 0 ? 1 : invocation->options_capacity * 2;
        option_t *new_options = realloc(invocation->options, new_capacity * sizeof(option_t));
        if (new_options == NULL) {
            return false;
        }
        invocation->options = new_options;
        invocation->options_capacity = new_capacity;
    }
    invocation->options[invocation->n_options].id = id;
    invocation->options[invocation->n_options].value = value;
    invocation->n_options++;
    return true;
}

bool invocation_add_argument(invocation_t *invocation, argument_t value) {
    if (invocation->n_arguments >= invocation->arguments_capacity) {
        size_t new_capacity = invocation->arguments_capacity == 0 ? 1 : invocation->arguments_capacity * 2;
        argument_t *new_arguments = realloc(invocation->arguments, new_capacity * sizeof(argument_t));
        if (new_arguments == NULL) {
            return false;
        }
        invocation->arguments = new_arguments;
        invocation->arguments_capacity = new_capacity;
    }
    invocation->arguments[invocation->n_arguments] = value;
    invocation->n_arguments++;
    return true;
}

argument_t *invocation_get_option(invocation_t *invocation, int id) {
    for (size_t i = 0; i < invocation->n_options; i++) {
        if (invocation->options[i].id == id) {
            return &invocation->options[i].value;
        }
    }
    return NULL;
}
