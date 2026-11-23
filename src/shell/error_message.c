#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"

#include "compat/src_compat.h"

void error_message_init(error_message_t *error) {
    if (error == NULL) {
        return;
    }
    error->message = NULL;
    error->free_message = false;
}

void error_message_set(error_message_t *error, const char *message) {
    if (error == NULL) {
        return;
    }

    error_message_fini(error);
    error->message = (char *)message;
    error->free_message = false;
}

void error_message_set_format(error_message_t *error, const char *format, ...) {
    char *message;
    va_list args;

    if (error == NULL) {
        return;
    }

    error_message_fini(error);

    va_start(args, format);
    if (vasprintf(&message, format, args) == -1) {
        error->message = "out of memory";
        error->free_message = false;
    }
    else {
        error->message = message;
        error->free_message = true;
    }
    va_end(args);
}

void error_message_fini(error_message_t *error) {
    if (error == NULL) {
        return;
    }
    if (error->free_message) {
        free(error->message);
    }
    error->message = NULL;
    error->free_message = false;
}

const char *error_message_get(const error_message_t *error) {
    if (error == NULL || error->message == NULL) {
        return "no error";
    }
    return error->message;
}
