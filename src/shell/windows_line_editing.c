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
    command_status_t status;

    // Enable line input mode for console
    GetConsoleMode(stdin_handle, &original_mode);
    mode = original_mode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(stdin_handle, mode);

    while (true) {
        printf("%s ", shell_get_prompt());
        if (getline(&line, &len, stdin) == -1) {
            break;
        }
        error_message_t *error_message = NULL;
        status = execute_line(line, &error_message);
        if (status == COMMAND_STATUS_ERROR) {
            fprintf(stderr, "%s\n", error_message_get(error_message));
            error_message_free(error_message);
        }
        else if (status == COMMAND_STATUS_EXIT) {
            break;
        }
    }

    SetConsoleMode(stdin_handle, original_mode);
    free(line);
    return true;
}
