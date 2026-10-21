#include "shell_int.h"

words_t *completion_words = NULL;

char **complete(const char *line, size_t position) {
    const command_t *cmd;
    size_t i, arg_index;

    if (completion_words == NULL) {
        completion_words = words_new();
    }
    words_clear(completion_words);
    error_message_t *error_message = NULL;
    if (!words_parse_partial(completion_words, line, line + position, &error_message)) {
        if (error_message) {
            error_message_free(error_message);
        }
        return NULL;
    }

    if (completion_words->n_words < 2) {
        /* Completing command. */
        return complete_command(argument_type_string, line);
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
    return complete_argument(cmd->arguments[arg_index].type, completion_words->words[arg_index]);
}
