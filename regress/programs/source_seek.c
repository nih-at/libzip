/*
  source_seek.c -- regress source seek edge cases
  Copyright (C) 2026 Dieter Baron and Thomas Klausner

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

#include <stdint.h>
#include <stdio.h>

#include "zip.h"

static int
test_unknown_length_window(void) {
    zip_error_t error;
    zip_source_t *base, *window;
    char data[] = "0123456789";
    char c;

    zip_error_init(&error);

    if ((base = zip_source_buffer_create(data, sizeof(data) - 1, 0, &error)) == NULL) {
        fprintf(stderr, "can't create buffer source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return -1;
    }
    if ((window = zip_source_window_create(base, 5, -1, &error)) == NULL) {
        fprintf(stderr, "can't create window source: %s\n", zip_error_strerror(&error));
        zip_source_free(base);
        zip_error_fini(&error);
        return -1;
    }
    zip_error_fini(&error);
    zip_source_free(base);

    if (zip_source_open(window) < 0) {
        fprintf(stderr, "can't open window source\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_read(window, &c, 1) != 1 || c != '5') {
        fprintf(stderr, "window source returned wrong initial byte\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_seek(window, 0, SEEK_SET) < 0) {
        fprintf(stderr, "can't seek to beginning of window source\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_read(window, &c, 1) != 1 || c != '5') {
        fprintf(stderr, "window source SEEK_SET ignored window start\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_seek(window, 2, SEEK_SET) < 0) {
        fprintf(stderr, "can't seek forward inside window source\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_read(window, &c, 1) != 1 || c != '7') {
        fprintf(stderr, "window source SEEK_SET used wrong offset\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_seek(window, -4, SEEK_CUR) == 0) {
        fprintf(stderr, "window source allowed seek before window start\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_seek(window, -1, SEEK_END) < 0) {
        fprintf(stderr, "can't seek relative to end of window source\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_tell(window) != 4) {
        fprintf(stderr, "window source reported wrong position after SEEK_END\n");
        zip_source_free(window);
        return -1;
    }
    if (zip_source_read(window, &c, 1) != 1 || c != '9') {
        fprintf(stderr, "window source SEEK_END ignored window start\n");
        zip_source_free(window);
        return -1;
    }
    zip_source_free(window);

    return 0;
}


static int
test_large_fragment_source(void) {
    zip_error_t error;
    zip_source_t *source;
    zip_buffer_fragment_t fragments[2];
    zip_uint8_t a = 'a', b = 'b';

    fragments[0].data = &a;
    fragments[0].length = ZIP_INT64_MAX;
    fragments[1].data = &b;
    fragments[1].length = 10;

    zip_error_init(&error);
    if ((source = zip_source_buffer_fragment_create(fragments, 2, 0, &error)) == NULL) {
        fprintf(stderr, "can't create fragment source: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return -1;
    }
    zip_error_fini(&error);

    if (zip_source_open(source) < 0) {
        fprintf(stderr, "can't open fragment source\n");
        zip_source_free(source);
        return -1;
    }
    if (zip_source_seek(source, -10, SEEK_END) < 0) {
        fprintf(stderr, "can't seek to representable offset near end of fragment source\n");
        zip_source_free(source);
        return -1;
    }
    if (zip_source_tell(source) != ZIP_INT64_MAX) {
        fprintf(stderr, "fragment source reported wrong position after SEEK_END\n");
        zip_source_free(source);
        return -1;
    }
    if (zip_source_seek(source, -1, SEEK_END) == 0) {
        fprintf(stderr, "fragment source allowed unrepresentable offset\n");
        zip_source_free(source);
        return -1;
    }
    zip_source_free(source);

    return 0;
}


static int
test_past_end_zip_source(const char *archive_name) {
    zip_error_t error;
    zip_source_t *source;
    zip_t *archive;
    int ze;

    ze = 0;
    if ((archive = zip_open(archive_name, ZIP_RDONLY, &ze)) == NULL) {
        fprintf(stderr, "can't open test archive: %d\n", ze);
        return -1;
    }

    zip_error_init(&error);
    source = zip_source_zip_file_create(archive, 0, 0, UINT64_MAX, -1, NULL, &error);
    if (source != NULL) {
        fprintf(stderr, "zip source allowed start past end of file\n");
        zip_source_free(source);
        zip_error_fini(&error);
        zip_discard(archive);
        return -1;
    }
    if (zip_error_code_zip(&error) != ZIP_ER_INVAL) {
        fprintf(stderr, "zip source returned wrong error for start past end of file\n");
        zip_error_fini(&error);
        zip_discard(archive);
        return -1;
    }

    zip_error_fini(&error);
    zip_discard(archive);
    return 0;
}


int
main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s archive\n", argv[0]);
        return 1;
    }

    if (test_unknown_length_window() < 0) {
        return 1;
    }
    if (test_large_fragment_source() < 0) {
        return 1;
    }
    if (test_past_end_zip_source(argv[1]) < 0) {
        return 1;
    }

    return 0;
}
