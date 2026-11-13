#include "shell_int.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

static char **complete_wrapper(const char *text, int start, int end) {
    completions_t *completions = complete(text, start, NULL, NULL);

    /* Don't fall back to filename matching if no matches were found. */
    if (completions == NULL || completions->n_matches == 0) {
        char **matches = malloc(sizeof(char *));
        matches[0] = NULL;
        return matches;
    }

    return completions_get_matches(completions);
}

bool use_readline(const char *script_filename) {
    return script_filename == NULL && isatty(fileno(stdin));
}


bool execute_lines_readline() {
    rl_attempted_completion_function = complete_wrapper;
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
