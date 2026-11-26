/*
  algorithm_bounds.c -- regress compression algorithm maximum_compressed_size bounds
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

#include <stdio.h>

#include "zipint.h"

static int check_algorithm(const char *name, const zip_compression_algorithm_t *alg) {
    static const zip_uint64_t inputs[] = {
        0,
        1,
        16384,
        ZIP_UINT64_MAX - 16384,
        ZIP_UINT64_MAX - 16383,
        ZIP_UINT64_MAX - 10,
        ZIP_UINT64_MAX
    };
    size_t i;

    for (i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
        zip_uint64_t in = inputs[i];
        zip_uint64_t out = alg->maximum_compressed_size(in);

        if (out < in) {
            fprintf(stderr, "%s: maximum_compressed_size(%" PRIu64 ") wrapped to %" PRIu64 "\n", name, in, out);
            return -1;
        }
        if (in >= ZIP_UINT64_MAX - 16384 && out != ZIP_UINT64_MAX) {
            fprintf(stderr, "%s: maximum_compressed_size(%" PRIu64 ") did not saturate: %" PRIu64 "\n", name, in, out);
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [ignored]\n", argv[0]);
        return 1;
    }

    if (check_algorithm("deflate", &zip_algorithm_deflate_compress) < 0) {
        return 1;
    }
#ifdef HAVE_LIBBZ2
    if (check_algorithm("bzip2", &zip_algorithm_bzip2_compress) < 0) {
        return 1;
    }
#endif
#ifdef HAVE_LIBLZMA
    if (check_algorithm("xz", &zip_algorithm_xz_compress) < 0) {
        return 1;
    }
#endif
#ifdef HAVE_LIBZSTD
    if (check_algorithm("zstd", &zip_algorithm_zstd_compress) < 0) {
        return 1;
    }
#endif

    return 0;
}
