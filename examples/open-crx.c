/*
  open-crx.c -- Open Google Chrome CRX files
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

/*
  This program opens a CRX version 3 file (Google Chrome extension).

  A CRX file contains a Chrome specific header followed by a zip archive. The header contains a length field that specifies the length of the header. The zip archive starts immediately after the header. Offsets within the zip archive are relative to the start of the zip archive, not the start of the CRX file. This means that a CRX file is not a valid zip archive and can't be opened directly.

  Offset       Size           Contents
  0            4 bytes        ASCII magic: "Cr24"
  4            4 bytes        uint32 little-endian: version = 3
  8            4 bytes        uint32 little-endian: header_size = N
  12           N bytes        CRX3 header, encoded as Protocol Buffers
  12 + N       remaining      ZIP archive containing the extension

  This program shows how to use libzip to open a zip archive that is a part of a larger file and can be adapted to other file formats.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <zip.h>

#include "compat.h"

#define CRX_MAGIC "Cr24"
#define CRX_FIXED_HEADER_SIZE 12

int main(int argc, char *argv[]) {
    const char *crx_file;
    zip_source_t *file_src, *window_src;
    zip_error_t error;
    char crx3_header[12];
    zip_int64_t nn;
    zip_uint32_t version;
    zip_uint64_t header_size;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <crx_file>\n", argv[0]);
        return 1;
    }

    crx_file = argv[1];

    zip_error_init(&error);

    /* Open CRX file as source. */
    if ((file_src = zip_source_file_create(crx_file, 0, -1, &error)) == NULL) {
        fprintf(stderr, "%s: can't open '%s': %s\n", argv[0], crx_file, zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }

    /* Open source for reading. */
    if (zip_source_open(file_src) < 0) {
        fprintf(stderr, "%s: can't open zip source for '%s': %s\n", argv[0], crx_file, zip_error_strerror(&error));
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }

    /* Read and parse the CRX header */
    if ((nn = zip_source_read(file_src, crx3_header, sizeof(crx3_header))) != sizeof(crx3_header)) {
        if (nn < 0) {
            fprintf(stderr, "%s: can't read CRX header in '%s': %s\n", argv[0], crx_file, zip_error_strerror(zip_source_error(file_src)));
        }
        else {
            fprintf(stderr, "%s: can't read CRX header in '%s': unexpected end of file\n", argv[0], crx_file);
        }
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }

    if (memcmp(crx3_header, CRX_MAGIC, 4) != 0) {
        fprintf(stderr, "%s: '%s' is not a valid CRX file\n", argv[0], crx_file);
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }

    version = crx3_header[4] | (crx3_header[5] << 8) | (crx3_header[6] << 16) | (crx3_header[7] << 24);

    if (version != 3) {
        fprintf(stderr, "%s: unsupported CRX version %u in '%s'\n", argv[0], version, crx_file);
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }

    header_size = crx3_header[8] | (crx3_header[9] << 8) | (crx3_header[10] << 16) | (crx3_header[11] << 24) + CRX_FIXED_HEADER_SIZE;

    /* We're done reading from the source. */
    if (zip_source_close(file_src) < 0) {
        fprintf(stderr, "%s: can't close zip source for '%s': %s\n", argv[0], crx_file, zip_error_strerror(&error));
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }

    /* Create window source with the zip archive portion of the CRX file. */
    window_src = zip_source_window_create(file_src, header_size + CRX_FIXED_HEADER_SIZE, -1, &error);
    if (!window_src) {
        fprintf(stderr, "%s: can't create window source for '%s': %s\n", argv[0], crx_file, zip_error_strerror(&error));
        zip_source_free(file_src);
        zip_error_fini(&error);
        return 1;
    }
    /* We no longer need file_source, so decrease its reference count. The window source will keep it alive as long as needed. */
    zip_source_free(file_src);

    /* Open the zip archive from the window source. */
    zip_t *za = zip_open_from_source(window_src, ZIP_RDONLY, &error);
    if (!za) {
        fprintf(stderr, "%s: can't open zip archive in '%s': %s\n", argv[0], crx_file, zip_error_strerror(&error));
        zip_source_free(window_src);
        zip_error_fini(&error);
        return 1;
    }

    zip_error_fini(&error);

    /* Use za here. */
    printf("Successfully opened zip archive from CRX file '%s' with %" PRIu64 " bytes header and %" PRId64 " entries.\n", crx_file, header_size + CRX_FIXED_HEADER_SIZE, zip_get_num_entries(za, 0));

    zip_close(za);
    /* za owns and frees window_source. */
    return 0;
}
