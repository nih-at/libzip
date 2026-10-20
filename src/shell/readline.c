#include "shell_int.h"

#ifdef USE_READLINE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

words_t *completion_words = NULL;

bool use_readline(const char *script_filename) {
    return script_filename == NULL && isatty(fileno(stdin));
}

char **complete(const char *text, int start, int end) {
    const command_t *cmd;
    size_t i, arg_index;

    if (completion_words == NULL) {
        completion_words = words_new();
    }
    words_clear(completion_words);
    error_message_t *error_message = NULL;
    if (!words_parse_partial(completion_words, rl_line_buffer, rl_line_buffer + end, &error_message)) {
        if (error_message) {
            error_message_free(error_message);
        }
        return NULL;
    }

    if (completion_words->n_words < 2) {
        /* Completing command. */
        return complete_command(argument_type_string, text);
    }

    cmd = find_command(completion_words->words[0]);
    if (cmd == NULL) {
        return NULL;
    }

    arg_index = 0;
    for (i = 1; i < completion_words->n_words - 1; i++) {
        if (completion_words->words[i][0] == '-') {
            continue;
            /* TODO: handle option arguments */
        }
        arg_index++;
    }
    if (arg_index >= cmd->n_arguments) {
        return NULL;
    }
    return complete_argument(cmd->arguments[arg_index].type, text);
}

bool execute_lines_readline() {
    /* editline's readline compatibility layer has a bug that causes a segfault if no matches are returned, so disable completion for now. */
    /* rl_attempted_completion_function = complete; */
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
