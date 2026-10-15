#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"

error_message_t *error_message_new(const char *message) {
    error_message_t *error = malloc(sizeof(error_message_t));
    if (error == NULL) {
        return NULL;
    }
    error->message = (char *)message;
    error->free_message = false;
    return error;
}

error_message_t *error_message_new_format(const char *format, ...) {
    error_message_t *error = malloc(sizeof(error_message_t));
    if (error == NULL) {
        return NULL;
    }
    va_list args;
    va_start(args, format);
    if (vasprintf(&error->message, format, args) == -1) {
        va_end(args);
        free(error);
        return NULL;
    }

    va_end(args);
    error->free_message = true;
    return error;
}

void error_message_free(error_message_t *error) {
    if (error) {
        if (error->free_message) {
            free(error->message);
        }
        free(error);
    }
}

const char *error_message_get(error_message_t *error) {
    if (error == NULL) {
        return "no error";
    }
    if (error->message == NULL) {
        return "unknown error";
    }
    return error->message;
}
