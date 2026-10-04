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


ZIP_EXTERN int zip_fclose(zip_file_t *zf) {
    int ret;
    zip_uint64_t i;

    if (zf == NULL) {
        return ZIP_ER_INVAL;
    }

    ret = 0;
    if (zf->error.zip_err) {
        ret = zf->error.zip_err;
        if (zip_error_system_type(&zf->error) == ZIP_ET_SYS) {
            errno = zf->error.sys_err;
        }
    }

    if (zf->za != NULL) {
        for (i = 0; i < zf->za->nopen_file; i++) {
            if (zf->za->open_file[i] == zf) {
                zf->za->open_file[i] = zf->za->open_file[--zf->za->nopen_file];
                break;
            }
        }
    }

    if (zf->src) {
        if (ZIP_SOURCE_IS_OPEN_READING(zf->src)) {
            if (zip_source_close(zf->src) < 0 && ret == 0) {
                zip_error_set_from_source(&zf->error, zf->src);
                ret = zf->error.zip_err;
            }
        }
        zip_source_free(zf->src);
    }

    zip_error_fini(&zf->error);
    free(zf);

    return ret;
}
