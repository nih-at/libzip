/*
  in-memory.c -- modify zip file in memory
  Copyright (C) 2014-2022 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <zip.h>

char const comp_data[] = "\xcb\x48\xcd\xc9\xc9\x57\x28\xcf\x2f\xca\x49\x51\xb0\xd2\xe4\x02\x00";
zip_uint64_t uncomp_size = 15;
zip_uint32_t uncomp_crc = 0xbf5d32ed;

int main() {
    int errorp;
    zip_error_t error;
    zip_t *za;
    zip_source_t *source;
    zip_int64_t index;

    if ((za = zip_open("archive.zip", ZIP_CREATE | ZIP_EXCL, &errorp)) == NULL) {
        zip_error_init_with_code(&error, errorp);
        fprintf(stderr, "can't open archive: %s\n", zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    if ((source = zip_source_buffer(za, comp_data, sizeof(comp_data) - 1, 0)) == NULL) {
        fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
        zip_discard(za);
        zip_error_fini(&error);
        return 1;
    }

    if ((errorp = zip_source_buffer_set_compression(source, ZIP_CM_DEFLATE, uncomp_size, uncomp_crc)) < 0) {
        zip_error_init_with_code(&error, errorp);
        fprintf(stderr, "can't set raw compression: %s\n", zip_error_strerror(&error));
        zip_discard(za);
        zip_error_fini(&error);
        return 1;
    }

    if ((index = zip_file_add(za, "hello", source, ZIP_FL_ENC_UTF_8)) < 0) {
        fprintf(stderr, "can't add file: %s\n", zip_error_strerror(&error));
        zip_discard(za);
        zip_error_fini(&error);
        return 1;
    }

    if ((errorp = zip_close(za)) < 0) {
        zip_error_init_with_code(&error, errorp);
        fprintf(stderr, "can't close the archive: %s\n", zip_error_strerror(&error));
        zip_discard(za);
        zip_error_fini(&error);
        return 1;
    }

    return 0;
}
