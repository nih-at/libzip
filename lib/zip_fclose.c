/*
  zip_fclose.c -- close file in zip archive
  Copyright (C) 1999-2025 Dieter Baron and Thomas Klausner

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


#include <stdlib.h>

#include "zipint.h"

static void
check_for_pending_read_error(zip_file_t *zf) {
    zip_stat_t st;
    zip_int64_t position;
    zip_int64_t n;
    zip_uint8_t dummy;

    zip_stat_init(&st);
    if (zip_source_stat(zf->src, &st) < 0 || (st.valid & ZIP_STAT_SIZE) == 0) {
        return;
    }
    if ((position = zip_source_tell(zf->src)) < 0 || (zip_uint64_t)position != st.size) {
        return;
    }

    n = zip_source_read(zf->src, &dummy, 1);
    if (n < 0) {
        zip_error_set_from_source(&zf->error, zf->src);
    }
    else if (n > 0) {
        zip_error_set(&zf->error, ZIP_ER_INCONS, MAKE_DETAIL_WITH_INDEX(ZIP_ER_DETAIL_INVALID_FILE_LENGTH, MAX_DETAIL_INDEX));
    }
}


ZIP_EXTERN int zip_fclose(zip_file_t *zf) {
    int ret;

    if (zf == NULL) {
        return ZIP_ER_INVAL;
    }

    if (zf->src) {
        if (zf->error.zip_err == 0) {
            check_for_pending_read_error(zf);
        }
        zip_source_free(zf->src);
    }

    ret = 0;
    if (zf->error.zip_err) {
        ret = zf->error.zip_err;
    }

    zip_error_fini(&zf->error);
    free(zf);
    return ret;
}
