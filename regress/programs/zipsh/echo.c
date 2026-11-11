#include "shell.h"
#include "zipsh.h"

/* clang-format off */
argument_definition_t echo_arguments[] = {
    {"text", "text to echo", argument_type_string, ARGUMENT_COLLECT, {.string = NULL}},
};
/* clang-format on */

bool echo_implementation(invocation_t *invocation, error_message_t **error_message) {
    for (size_t i = 0; i < invocation->n_arguments; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%s", invocation->arguments[i].string);
    }
    printf("\n");
    return true;
}

COMMAND_NO_OPTIONS(echo, "echo the provided text");
