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

#include "zipconf.h"
#include "zipsh.h"


#define ARGUMENT_ARCHIVE 0
#define ARGUMENT_SHELL 1

/* clang-format off */
argument_definition_t zipsh_arguments[] = {
    {"archive", "ZIP archive to open", argument_type_string, ARGUMENT_REQUIRED, {.string = NULL}},
     SHELL_ARGUMENT
};

option_definition_t zipsh_options[] = {
    SHELL_OPTIONS,
    ZIPSH_OPTIONS
};
/* clang-format on */


bool zipsh_setup(invocation_t *invocation, error_message_t **error_message) {
    archive_name = invocation->arguments[ARGUMENT_ARCHIVE].string;

    return open_archive(invocation, error_message);
}

bool zipsh_cleanup(bool ok, error_message_t **error_message) {
    return close_archive(error_message);
}

SHELL_MAIN(zipsh, NULL, ARGUMENT_SHELL)

int main(int argc, char *argv[]) {
    return shell_main(&shell_command_zipsh, argc, argv);
}
