#include <stdio.h>
#include <string.h>

#include "shell.h"

bool help_implementation(invocation_t *invocation, error_message_t **error_message) {
    int longest_command_name = 0;
    for (size_t i = 0; i < n_commands; i++) {
        size_t len = strlen(commands[i]->name);
        if (len > longest_command_name) {
            longest_command_name = (int)len;
        }
    }
    const char *command_name = invocation->arguments[0].string;
    if (command_name == NULL) {
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
        // Display help for the specific command
        printf("Usage: %s", cmd->name);
        if (cmd->n_options > 0) {
            printf(" [options]");
        }
        for (size_t i = 0; i < cmd->n_arguments; i++) {
            if (cmd->arguments[i].required) {
                printf(" %s", cmd->arguments[i].name);
            }
            else {
                printf(" [%s]", cmd->arguments[i].name);
            }
        }
        printf("\n");
        printf("%s\n", cmd->help);
        if (cmd->n_options > 0) {
            int longest_option_name = 0;
            for (size_t i = 0; i < cmd->n_options; i++) {
                size_t len = 4; /* short option */
                if (cmd->options[i].long_name) {
                    len += strlen(cmd->options[i].long_name) + 4;
                }
                if (cmd->options[i].argument_type != ARGUMENT_TYPE_BOOL) {
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
                if (cmd->options[i].id < 256) {
                    printf("  -%c", cmd->options[i].id);
                    if (cmd->options[i].long_name) {
                        printf(",");
                    }
                }
                else {
                    printf("    ");
                }
                if (cmd->options[i].long_name) {
                    width += (int)strlen(cmd->options[i].long_name) + 4;
                    printf(" --%s", cmd->options[i].long_name);
                }
                if (cmd->options[i].argument_type != ARGUMENT_TYPE_BOOL) {
                    if (cmd->options[i].argument_name) {
                        width += (int)strlen(cmd->options[i].argument_name) + 1;
                        printf(" %s", cmd->options[i].argument_name);
                    }
                    else {
                        width += 4; /* "arg" */
                        printf(" arg");
                    }
                }
                for (int i = width; i < longest_option_name; i++) {
                    printf(" ");
                }
                printf("  %s\n", cmd->options[i].help);
            }
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
    }
    return true;
}

/* clang-format off */
argument_definition_t help_arguments[] = {
    {"command", "command to display help for", ARGUMENT_TYPE_STRING, false, {.string = NULL}},
};
/* clang-format on */

COMMAND_NO_OPTIONS(help, "display help information");
