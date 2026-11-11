#include "shell_int.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const argument_type_t argument_type_help_instance = {"help", NULL, complete_command, NULL, {0}};
#define argument_type_help (&argument_type_help_instance)

int compare_option(const void *a, const void *b) {
    const option_definition_t *opt_a = *(const option_definition_t **)a;
    const option_definition_t *opt_b = *(const option_definition_t **)b;
    char a_short[2], b_short[2];
    const char *a_name, *b_name;
    int ret;

    if (opt_a->id < 256) {
        a_short[0] = (char)opt_a->id;
        a_short[1] = '\0';
        a_name = a_short;
    }
    else {
        a_name = opt_a->long_name;
    }
    if (opt_b->id < 256) {
        b_short[0] = (char)opt_b->id;
        b_short[1] = '\0';
        b_name = b_short;
    }
    else {
        b_name = opt_b->long_name;
    }

    ret = strcasecmp(a_name, b_name);
    if (ret == 0) {
        ret = a_name[0] - b_name[0];
    }

    return ret;
}

bool print_command_help(const command_t *cmd, const char *prog_name, error_message_t **error) {
    printf("Usage: %s", prog_name ? prog_name : cmd->name);
    if (cmd->n_options > 0) {
        printf(" [options]");
    }

    for (size_t i = 0; i < cmd->n_arguments; i++) {
        switch (cmd->arguments[i].cardinality) {
        case ARGUMENT_REQUIRED:
            printf(" %s", cmd->arguments[i].name);
            break;
        case ARGUMENT_OPTIONAL:
            printf(" [%s]", cmd->arguments[i].name);
            break;
        case ARGUMENT_COLLECT:
            printf(" [%s ...]", cmd->arguments[i].name);
            break;
        }
    }
    printf("\n");

    if (cmd->help) {
        printf("%s\n", cmd->help);
    }

    if (cmd->n_options > 0) {
        option_definition_t **options;
        int longest_option_name = 0;

        /* Sort options. */
        if ((options = (option_definition_t **)malloc(cmd->n_options * sizeof(option_definition_t *))) == NULL) {
            *error = error_message_new("out of memory");
            return false;
        }
        for (size_t i = 0; i < cmd->n_options; i++) {
            options[i] = &cmd->options[i];
        }
        qsort(options, cmd->n_options, sizeof(option_definition_t *), compare_option);

        /* Get longest option name for formatting. */
        for (size_t i = 0; i < cmd->n_options; i++) {
            size_t len = 4; /* short option */
            if (cmd->options[i].long_name) {
                len += strlen(cmd->options[i].long_name) + 4;
            }
            if (cmd->options[i].argument_type != NULL) {
                if (cmd->options[i].argument_name) {
                    len += strlen(cmd->options[i].argument_name) + 1;
                }
                else {
                    len += 4; /* "arg" */
                }
            }
            if (len > longest_option_name) {
                longest_option_name = (int)len;
            }
        }

        printf("\nOptions:\n");
        for (size_t i = 0; i < cmd->n_options; i++) {
            int width = 4; /* short option */
            if (options[i]->id < 256) {
                printf("  -%c", options[i]->id);
                if (options[i]->long_name) {
                    printf(",");
                }
            }
            else {
                printf("     ");
            }
            if (options[i]->long_name) {
                width += (int)strlen(options[i]->long_name) + 4;
                printf(" --%s", options[i]->long_name);
            }
            if (options[i]->argument_type != NULL) {
                if (options[i]->argument_name) {
                    width += (int)strlen(options[i]->argument_name) + 1;
                    printf(" %s", options[i]->argument_name);
                }
                else {
                    width += 4; /* "arg" */
                    printf(" arg");
                }
            }
            for (int i = width; i < longest_option_name; i++) {
                printf(" ");
            }
            printf("  %s\n", options[i]->help);
        }
        free(options);
    }
    if (cmd->n_arguments > 0) {
        int longest_argument_name = 0;
        for (size_t i = 0; i < cmd->n_arguments; i++) {
            size_t len = strlen(cmd->arguments[i].name);
            if (len > longest_argument_name) {
                longest_argument_name = (int)len;
            }
        }
        printf("\nArguments:\n");
        for (size_t i = 0; i < cmd->n_arguments; i++) {
            printf("  %-*s  %s\n", longest_argument_name, cmd->arguments[i].name, cmd->arguments[i].help);
        }
    }

    return true;
}

#define ARGUMENT_COMMAND 0

/* clang-format off */
argument_definition_t help_arguments[] = {
    {"command", "command to display help for", argument_type_help, ARGUMENT_OPTIONAL, {.string = NULL}},
};
/* clang-format on */

bool help_implementation(invocation_t *invocation, error_message_t **error_message) {
    const char *command_name = invocation->arguments[ARGUMENT_COMMAND].string;

    if (command_name == NULL) {
        int longest_command_name = 0;
        for (size_t i = 0; i < n_commands; i++) {
            size_t len = strlen(commands[i]->name);
            if (len > longest_command_name) {
                longest_command_name = (int)len;
            }
        }
        printf("Available commands:\n");
        for (size_t i = 0; i < n_commands; i++) {
            printf("  %-*s  %s\n", (int)longest_command_name, commands[i]->name, commands[i]->help);
        }
    }
    else {
        const command_t *cmd = find_command(command_name);
        if (cmd == NULL) {
            if (error_message) {
                *error_message = error_message_new_format("Unknown command: %s", command_name);
            }
            return false;
        }
        if (!print_command_help(cmd, NULL, error_message)) {
            return false;
        }
    }
    return true;
}

COMMAND_NO_OPTIONS(help, "display help information");
