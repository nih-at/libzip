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
    invocation_t *invocation = invocation_new(cmd, argc, argv, NULL, error_message);

    if (invocation == NULL) {
        return false;
    }

    ok = cmd->function(invocation, error_message);
    invocation_free(invocation);
    return ok;
}


char **complete_command(const argument_type_t *type, const char *str) {
    size_t count = 0;
    char **matches = NULL;

    for (size_t i = 0; i < n_commands; i++) {
        if (strncmp(commands[i]->name, str, strlen(str)) == 0) {
            count++;
        }
    }

    matches = malloc((count + 1) * sizeof(char *));
    if (matches) {
        size_t j = 0;
        for (size_t i = 0; i < n_commands; i++) {
            if (strncmp(commands[i]->name, str, strlen(str)) == 0) {
                matches[j++] = strdup(commands[i]->name);
            }
        }
        matches[j] = NULL;
    }

    return matches;
}
