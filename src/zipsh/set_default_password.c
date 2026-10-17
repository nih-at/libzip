#include "zipsh.h"

bool set_default_password_implementation(invocation_t *invocation, error_message_t **error_message) {
    if (zip_set_default_password(archive, invocation->arguments[0].string) != 0) {
        *error_message = error_message_new_format("%s", zip_strerror(archive));
        return false;
    }
    return true;
}

/* clang-format off */
argument_definition_t set_default_password_arguments[] = {
    {"password", "password to set for the archive", ARGUMENT_TYPE_STRING, false, {.string = NULL}},
};
/* clang-format on */

COMMAND_NO_OPTIONS(set_default_password, "set the default password");
