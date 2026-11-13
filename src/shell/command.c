#include <stdlib.h>
#include <string.h>

#include "shell.h"


const command_t *find_command(const char *name) {
    size_t i;

    for (i = 0; i < n_commands; i++) {
        if (strcmp(name, commands[i]->name) == 0) {
            return commands[i];
        }
    }

    return NULL;
}


bool execute_command(const command_t *cmd, int argc, char **argv, error_message_t **error_message) {
    bool ok = true;
    invocation_t *invocation = invocation_new(cmd, NULL, argc, argv, error_message, NULL);

    if (invocation == NULL) {
        return false;
    }

    ok = cmd->function(invocation, error_message);
    invocation_free(invocation);
    return ok;
}


void complete_command(completions_t *completions, const argument_type_t *type, const char *str) {
    for (size_t i = 0; i < n_commands; i++) {
        if (strncmp(commands[i]->name, str, strlen(str)) == 0) {
            completions_add(completions, commands[i]->name);
        }
    }
}


option_definition_t *command_find_short_option(const command_t *cmd, char short_name) {
    for (size_t i = 0; i < cmd->n_options; i++) {
        if (cmd->options[i].id == short_name) {
            return &cmd->options[i];
        }
    }
    return NULL;
}

option_definition_t *command_find_long_option(const command_t *cmd, const char *long_name, size_t long_name_length) {
    for (size_t i = 0; i < cmd->n_options; i++) {
        if (strncmp(cmd->options[i].long_name, long_name, long_name_length) == 0) {
            return &cmd->options[i];
        }
    }
    return NULL;
}
