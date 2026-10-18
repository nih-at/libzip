#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "shell_int.h"

#ifdef USE_READLINE
bool use_readline(const char *script_filename) {
    return script_filename == NULL && isatty(fileno(stdin));
}

bool execute_lines_readline() {
    while (true) {
        char *line = readline("zipsh> ");
        if (line == NULL) {
            break;
        }
        if (*line != '\0') {
            add_history(line);
            error_message_t *error_message = NULL;
            if (!execute_line(line, &error_message)) {
                fprintf(stderr, "%s\n", error_message_get(error_message));
                error_message_free(error_message);
            }
        }
        free(line);
    }
    return true;
}

#else
bool use_readline(const char *script_filename) {
    return false;
}

bool execute_lines_readline() {
    return false;
}
#endif /* USE_READLINE */
