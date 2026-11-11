#include "compat/src_compat.h"
#include "shell.h"

#include <stdlib.h>

static int parse_options(const command_t *cmd, int argc, char **argv, invocation_t *invocation, int *early_exit_options, bool *early_exit, error_message_t **error_message) {
    char short_options[256];
    char *p = short_options;
    struct option *long_options;
    int long_options_index = 0;
    int c;

    long_options = (struct option *)malloc((cmd->n_options + 1) * sizeof(struct option));
    if (!long_options) {
        *error_message = error_message_new("out of memory");
        return -1;
    }

    for (size_t i = 0; i < cmd->n_options; i++) {
        if (cmd->options[i].long_name) {
            long_options[long_options_index].name = cmd->options[i].long_name;
            long_options[long_options_index].has_arg = (cmd->options[i].argument_type == NULL) ? no_argument : required_argument;
            long_options[long_options_index].flag = NULL;
            long_options[long_options_index].val = cmd->options[i].id;
            long_options_index++;
        }
        if (cmd->options[i].id < 256) {
            *p++ = cmd->options[i].id;
            if (cmd->options[i].argument_type != NULL) {
                *p++ = ':';
            }
        }
    }
    *p = '\0';
    long_options[long_options_index].name = NULL;

    optind = 1;
    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        bool found = false;
        if (early_exit_options) {
            for (int i = 0; early_exit_options[i] != 0; i++) {
                if (c == early_exit_options[i]) {
                    *early_exit = true;
                }
            }
        }
        for (size_t i = 0; i < cmd->n_options; i++) {
            if (c == cmd->options[i].id) {
                argument_t value;
                found = true;
                if (cmd->options[i].argument_type == NULL) {
                    value.boolean = true;
                }
                else if (optarg == NULL) {
                    *error_message = error_message_new_format("Missing argument for option %s", cmd->options[i].long_name ? cmd->options[i].long_name : (char[]){(char)cmd->options[i].id, '\0'});
                    free(long_options);
                    return -1;
                }
                else {
                    error_message_t *local_error_message = NULL;
                    if (!convert_argument(cmd->options[i].argument_type, &value, optarg, &local_error_message)) {
                        *error_message = error_message_new_format("invalid value for argument option %s: %s", cmd->options[i].long_name ? cmd->options[i].long_name : (char[]){(char)cmd->options[i].id, '\0'}, error_message_get(local_error_message));
                        error_message_free(local_error_message);
                        free(long_options);
                        return -1;
                    }
                }
                invocation_add_option(invocation, cmd->options[i].id, value);
                break;
            }
        }
        if (!found) {
            *error_message = error_message_new_format("Unknown option: %s", argv[optind - 1]);
            free(long_options);
            return -1;
        }
    }

    return optind;
}


static bool parse_commandline(invocation_t *invocation, const command_t *cmd, int argc, char **argv, int *early_exit_options, error_message_t **error_message) {
    int argv_index;
    bool early_exit = false;
    size_t invocation_index, argument_index;


    if (invocation == NULL) {
        *error_message = error_message_new("out of memory");
        return false;
    }

    argv_index = parse_options(cmd, argc, argv, invocation, early_exit_options, &early_exit, error_message);
    if (argv_index < 0) {
        return false;
    }
    if (early_exit) {
        return true;
    }

    invocation_index = 0;
    argument_index = 0;
    while (argument_index < cmd->n_arguments) {
        if (argv_index < argc) {
            error_message_t *local_error_message = NULL;
            if (cmd->arguments[argument_index].cardinality == ARGUMENT_COLLECT) {
                for (; argv_index < argc; argv_index++) {
                    error_message_t *local_error_message = NULL;
                    if (!convert_argument(cmd->arguments[argument_index].type, &invocation->arguments[invocation_index], argv[argv_index], &local_error_message)) {
                        *error_message = error_message_new_format("invalid value for argument %s: %s", cmd->arguments[argument_index].name, error_message_get(local_error_message));
                        return false;
                    }
                    invocation_index++;
                }
                break;
            }
            if (!convert_argument(cmd->arguments[argument_index].type, &invocation->arguments[invocation_index], argv[argv_index], &local_error_message)) {
                *error_message = error_message_new_format("invalid value for argument %s: %s", cmd->arguments[argument_index].name, error_message_get(local_error_message));
                return false;
            }
            invocation_index++;
            argument_index++;
            argv_index++;
        }
        else if (cmd->arguments[argument_index].cardinality != ARGUMENT_REQUIRED) {
            invocation->arguments[invocation_index] = cmd->arguments[argument_index].default_value;
            invocation_index++;
            argument_index++;
        }
        else {
            /* TODO: Handle ARGUMENT_COLLECT case */
            *error_message = error_message_new_format("missing argument %s", cmd->arguments[argument_index].name);
            return false;
        }
    }

    if (argv_index < argc) {
        *error_message = error_message_new("too many arguments for command");
        return false;
    }

    invocation->n_arguments = invocation_index;

    return true;
}

invocation_t *invocation_new(const command_t *cmd, int argc, char **argv, int *early_exit_options, error_message_t **error_message) {
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

    if (!parse_commandline(invocation, cmd, argc, argv, early_exit_options, error_message)) {
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
