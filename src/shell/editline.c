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
    /* TODO: set up completion function */

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
