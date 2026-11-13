#ifndef _HAD_SHELL_INT_H
#define _HAD_SHELL_INT_H

#include "shell.h"

void complete_command(completions_t *completions, const argument_type_t *type, const char *str);

bool use_readline(const char *script_filename);
bool execute_lines_readline();
bool execute_line(const char *line, error_message_t **error_message);

completions_t *complete(const char *text, size_t position, size_t *last_word_length, char *open_quote);
void free_completions(char **matches);

#endif /* _HAD_SHELL_INT_H */