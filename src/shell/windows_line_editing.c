#include "compat/src_compat.h"
#include "shell_int.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

bool use_readline(const char *script_filename) {
    return script_filename == NULL && isatty(fileno(stdin));
}

bool execute_lines_readline() {
    char *line = NULL;
    size_t len = 0;
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD original_mode, mode;

    // Enable line input mode for console
    GetConsoleMode(stdin_handle, &original_mode);
    mode = original_mode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(stdin_handle, mode);

    while (getline(&line, &len, stdin) != -1) {
        error_message_t *error_message = NULL;
        if (!execute_line(line, &error_message)) {
            fprintf(stderr, "%s\n", error_message_get(error_message));
            error_message_free(error_message);
        }
    }

    SetConsoleMode(stdin_handle, original_mode);
    free(line);
    return true;
}
