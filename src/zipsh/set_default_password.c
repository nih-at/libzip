#include "zipsh.h"

#define ARGUMENT_PASSWORD 0

/* clang-format off */
argument_definition_t set_default_password_arguments[] = {
    {"password", "password to set for the archive", argument_type_string, ARGUMENT_OPTIONAL, {.string = NULL}},
};
/* clang-format on */

bool set_default_password_implementation(invocation_t *invocation, error_message_t **error_message) {
    if (zip_set_default_password(archive, invocation->arguments[ARGUMENT_PASSWORD].string) != 0) {
        *error_message = error_message_new_format("%s", zip_strerror(archive));
        return false;
    }
    return true;
}

COMMAND_NO_OPTIONS(set_default_password, "set the default password");
