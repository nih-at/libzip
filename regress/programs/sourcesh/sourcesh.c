#include "sourcesh.h"


#define ARGUMENT_SHELL 0

/* clang-format off */
argument_definition_t sourcesh_arguments[] = {
     SHELL_ARGUMENT
};

option_definition_t sourcesh_options[] = {
    SHELL_OPTIONS
};
/* clang-format on */

bool sourcesh_setup(invocation_t *invocation, error_message_t **error_message) {
    return true;
}

bool sourcesh_cleanup(bool ok, error_message_t **error_message) {
    free_all_opened_sources();
    return true;
}

SHELL_MAIN(sourcesh, NULL, ARGUMENT_SHELL)

int main(int argc, char *argv[]) {
    return shell_main(&shell_command_sourcesh, argc, argv);
}
