#include "shell_int.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <histedit.h>

bool use_readline(const char *script_filename) {
    return script_filename == NULL && isatty(fileno(stdin));
}

static char *prompt_func(EditLine *el) {
    return "zipsh> ";
}

static unsigned char complete_wrapper(EditLine *el, int ch) {
    const LineInfo *line_info = el_line(el);
    size_t position = line_info->cursor - line_info->buffer;
    size_t last_word_length;
    char **matches = complete(line_info->buffer, position, &last_word_length);
    char *prefix;
    size_t prefix_length, i, full_match;

    if (matches == NULL || matches[0] == NULL) {
        free_completions(matches);
        return CC_ERROR;
    }

    prefix = matches[0];
    prefix_length = strlen(prefix);
    full_match = 1;

    for (i = 0; matches[i] != NULL; i++) {
        size_t match_length = 0;
        for (match_length = 0; matches[i][match_length] != '\0' && match_length < prefix_length; match_length++) {
            if (matches[i][match_length] != prefix[match_length]) {
                break;
            }
        }
        if (match_length < prefix_length) {
            prefix_length = match_length;
            full_match = 0;
        }
    }

    if (prefix_length > last_word_length) {
        prefix[prefix_length] = '\0';
        el_insertstr(el, prefix + last_word_length);
        if (full_match && matches[1] == NULL) {
            el_insertstr(el, " ");
        }
        free_completions(matches);
        return CC_REFRESH;
    }

    /* TODO: sort matches, display them in a user-friendly way, limit number of matches displayed */
    printf("\n");
    for (i = 0; matches[i] != NULL; i++) {
        printf("%s\n", matches[i]);
    }
    free_completions(matches);
    return CC_REDISPLAY;
}

bool execute_lines_readline() {
    int line_len;
    EditLine *el;
    History *hist;
    HistEvent ev;

    if ((el = el_init("zipsh", stdin, stdout, stderr)) == NULL) {
        return false;
    }
    if ((hist = history_init()) == NULL) {
        el_end(el);
        return false;
    }

    history(hist, &ev, H_SETSIZE, 100);
    el_set(el, EL_HIST, history, hist);
    el_set(el, EL_PROMPT, prompt_func);
    el_set(el, EL_EDITOR, "emacs");
    el_set(el, EL_SIGNAL, 1);
    el_source(el, NULL);
    el_set(el, EL_ADDFN, "complete", "completion function", complete_wrapper);
    el_set(el, EL_BIND, "\t", "complete", NULL);

    while (true) {
        const char *line = el_gets(el, &line_len);
        if (line == NULL) {
            break;
        }
        history(hist, &ev, H_ENTER, line);
        error_message_t *error_message = NULL;
        if (!execute_line(line, &error_message)) {
            fprintf(stderr, "%s\n", error_message_get(error_message));
            error_message_free(error_message);
        }
    }

    history_end(hist);
    el_end(el);
    return true;
}
