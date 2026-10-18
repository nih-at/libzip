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

zip_t *archive = NULL;

int main(int argc, char *argv[]) {
    const char *script_filename = NULL;
    const char *archive_name = NULL;
    int err;
    bool ok;

    archive = NULL;

    switch (argc) {
    case 3:
        script_filename = argv[2];
        // fall through
    case 2:
        archive_name = argv[1];
        archive = zip_open(archive_name, 0, &err);
        if (archive == NULL) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            fprintf(stderr, "can't open archive '%s': %s\n", archive_name, zip_error_strerror(&error));
            zip_error_fini(&error);
            exit(1);
        }
        break;
    default:
        fprintf(stderr, "Usage: %s archive [command_file]\n", argv[0]);
        exit(1);
    }

    ok = shell_loop(script_filename);

    if (zip_close(archive) < 0) {
        fprintf(stderr, "can't close '%s': %s\n", archive_name, zip_strerror(archive));
        zip_discard(archive);
    }
    return ok ? 0 : 1;
}
