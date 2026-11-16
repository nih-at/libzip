#include "shell.h"

#include "compat/src_compat.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static shell_prompt_function_t current_prompt_function = NULL;
static char *current_prompt = NULL;

const char *shell_get_prompt(void) {
    if (current_prompt_function) {
        return current_prompt_function();
    }
    else if (current_prompt) {
        return current_prompt;
    }
    return "> ";
}

void shell_set_prompt_function(shell_prompt_function_t prompt_function) {
    current_prompt_function = prompt_function;
}

void shell_set_prompt(const char *format, ...) {
    va_list args;
    char *previous_prompt = current_prompt;
    va_start(args, format);
    if (vasprintf(&current_prompt, format, args) == -1) {
        current_prompt = previous_prompt;
    }
    else {
        free(previous_prompt);
    }
    va_end(args);
}
