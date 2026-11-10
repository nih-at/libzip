#include "zipsh.h"

#include <stdlib.h>

extra_field_options_t *extra_field_options_create(invocation_t *invocation, error_message_t **error_message) {
    extra_field_options_t *options;
    argument_t *arg;

    options = malloc(sizeof(extra_field_options_t));
    if (!options) {
        *error_message = error_message_new("Memory allocation failed");
        return NULL;
    }

    if ((arg = invocation_get_option(invocation, 'I'))) {
        options->id = arg->u16;
        options->id_specified = true;
    }
    else {
        options->id = 0;
        options->id_specified = false;
    }

    options->flags = 0;
    if ((arg = invocation_get_option(invocation, 'c'))) {
        options->flags |= ZIP_FL_CENTRAL;
    }
    if ((arg = invocation_get_option(invocation, 'l'))) {
        options->flags |= ZIP_FL_LOCAL;
    }
    if (options->flags == 0) {
        options->flags = ZIP_FL_CENTRAL | ZIP_FL_LOCAL;
    }

    return options;
}

void extra_field_options_free(extra_field_options_t *extra_field_options) {
    if (extra_field_options) {
        free(extra_field_options);
    }
}
