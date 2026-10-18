#ifndef _HAD_SHELL_INT_H
#define _HAD_SHELL_INT_H

#include "shell.h"

bool use_readline(const char *script_filename);
bool execute_lines_readline();
bool execute_line(char *line, error_message_t **error_message);

#endif /* _HAD_SHELL_INT_H */