#ifndef _HAD_SHELL_INT_H
#define _HAD_SHELL_INT_H

#include "shell.h"

char **complete_command(const argument_type_t *type, const char *str);

bool use_readline(const char *script_filename);
bool execute_lines_readline();
bool execute_line(const char *line, error_message_t **error_message);

char **complete(const char *text, size_t position);

#endif /* _HAD_SHELL_INT_H */