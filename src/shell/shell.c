/*
  zipsh.c -- shell for interacting with zip archives in multiple ways
  Copyright (C) 2012-2025 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <info@libzip.org>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell_int.h"

#include "compat/src_compat.h"

words_t *words = NULL;

bool execute_line(char *line, error_message_t **error_message) {
    bool ok = true;

    if (!words_parse(words, line, error_message)) {
        return false;
    }

    if (words->n_words > 0) {
        error_message_t *local_error_message = NULL;
        const command_t *cmd = find_command(words->words[0]);
        if (cmd == NULL) {
            if (error_message) {
                *error_message = error_message_new_format("unknown command '%s'", words->words[0]);
            }
            ok = false;
        }
        else if (!execute_command(cmd, (int)words->n_words, words->words, &local_error_message)) {
            if (error_message) {
                *error_message = error_message_new_format("%s: %s", words->words[0], error_message_get(local_error_message));
                error_message_free(local_error_message);
            }
            ok = false;
        }
    }

    return ok;
}


bool execute_lines(const char *filename, FILE *input) {
    size_t line_number = 0;
    error_message_t *error_message = NULL;
    char *line = NULL;
    size_t len = 0;
    bool ok = true;

    while (getline(&line, &len, input) != -1) {
        line_number++;
        if (!execute_line(line, &error_message)) {
            fprintf(stderr, "%s:%zu: %s\n", filename, line_number, error_message_get(error_message));
            error_message_free(error_message);
            error_message = NULL;
            ok = false;
        }
    }
    free(line);
    return ok;
}


bool shell_loop(const char *script_filename) {
    FILE *input = stdin;
    const char *file_name = "*stdin*";
    bool ok = true;

    if ((words = words_new()) == NULL) {
        fprintf(stderr, "Failed to allocate words structure\n");
        exit(1);
    }

    if (script_filename != NULL) {
        input = fopen(script_filename, "r");
        if (input == NULL) {
            fprintf(stderr, "Failed to open file: %s\n", script_filename);
            exit(1);
        }
        file_name = script_filename;
    }

    if (use_readline(script_filename)) {
        ok = execute_lines_readline();
    }
    else {
        ok = execute_lines(file_name, input);
    }
    if (input != stdin) {
        fclose(input);
    }
    words_free(words);
    return ok;
}
