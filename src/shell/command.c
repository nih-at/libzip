#include <stdlib.h>
#include <string.h>

#include "shell.h"

#include "compat/src_compat.h"

const command_t *find_command(const char *name) {
    size_t i;

    for (i = 0; i < n_commands; i++) {
        if (strcmp(name, commands[i]->name) == 0) {
            return commands[i];
        }
    }

    return NULL;
}

int parse_options(const command_t *cmd, int argc, char **argv, invocation_t *invocation, error_message_t **error_message) {
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
            long_options[long_options_index].has_arg = (cmd->options[i].argument_type == ARGUMENT_TYPE_BOOL) ? no_argument : required_argument;
            long_options[long_options_index].flag = NULL;
            long_options[long_options_index].val = cmd->options[i].id;
            long_options_index++;
        }
        if (cmd->options[i].id < 256) {
            *p++ = cmd->options[i].id;
            if (cmd->options[i].argument_type != ARGUMENT_TYPE_BOOL) {
                *p++ = ':';
            }
        }
    }
    *p = '\0';
    long_options[long_options_index].name = NULL;

    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        bool found = false;
        for (size_t i = 0; i < cmd->n_options; i++) {
            if (c == cmd->options[i].id) {
                argument_t value;
                found = true;
                if (cmd->options[i].argument_type == ARGUMENT_TYPE_BOOL) {
                    value.b = true;
                }
                else if (optarg == NULL) {
                    *error_message = error_message_new_format("Missing argument for option %s", cmd->options[i].long_name ? cmd->options[i].long_name : (char[]){(char)cmd->options[i].id, '\0'});
                    free(long_options);
                    return -1;
                }
                else {
                    error_message_t *local_error_message = NULL;
                    if (!convert_argument(optarg, &value, cmd->options[i].argument_type, &local_error_message)) {
                        *error_message = error_message_new_format("Invalid argument for option %s: %s", cmd->options[i].long_name ? cmd->options[i].long_name : (char[]){(char)cmd->options[i].id, '\0'}, error_message_get(local_error_message));
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


bool execute_command(const command_t *cmd, int argc, char **argv, error_message_t **error_message) {
    bool ok = true;
    invocation_t *invocation = invocation_new(cmd->n_options, cmd->n_arguments);
    int first_argument;

    if (invocation == NULL) {
        *error_message = error_message_new("out of memory");
        return false;
    }

    first_argument = parse_options(cmd, argc, argv, invocation, error_message);
    if (first_argument < 0) {
        invocation_free(invocation);
        return false;
    }

    if (argc - first_argument > cmd->n_arguments) {
        *error_message = error_message_new("Too many arguments for command");
        invocation_free(invocation);
        return false;
    }

    for (size_t i = 0; i < cmd->n_arguments; i++) {
        if (i + first_argument < argc) {
            error_message_t *local_error_message = NULL;
            if (!convert_argument(argv[i + first_argument], &invocation->arguments[i], cmd->arguments[i].type, &local_error_message)) {
                *error_message = error_message_new_format("Invalid argument for %s: %s", cmd->arguments[i].name, error_message_get(local_error_message));
                invocation_free(invocation);
                return false;
            }
        }
        else if (!cmd->arguments[i].required) {
            invocation->arguments[i] = cmd->arguments[i].default_value;
        }
        else {
            *error_message = error_message_new_format("Missing argument: %s", cmd->arguments[i].name);
            invocation_free(invocation);
            return false;
        }
    }
    ok = cmd->function(invocation, error_message);
    invocation_free(invocation);
    return ok;
}
