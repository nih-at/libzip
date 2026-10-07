/*
  zip_allocate.c -- allocate memory for an array, checking for overflow
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

#include <stdlib.h>

#include "zipint.h"

/* Compute nmemb * element_size + extra_bytes as a size_t.

   Returns false and sets error to ZIP_ER_MEMORY if the result would
   overflow size_t; the computation itself never overflows and never
   divides by zero, so it is safe for arbitrary (for example, archive
   derived) arguments. error may be NULL. */

bool _zip_size_of_array(zip_uint64_t nmemb, zip_uint64_t element_size, zip_uint64_t extra_bytes, size_t *sizep, zip_error_t *error) {
    zip_uint64_t max_size = (zip_uint64_t)SIZE_MAX;

    if (extra_bytes > max_size || (element_size > 0 && nmemb > (max_size - extra_bytes) / element_size)) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return false;
    }

    *sizep = (size_t)(nmemb * element_size + extra_bytes);

    return true;
}


/* Allocate nmemb * element_size + extra_bytes bytes, checking the size
   computation for overflow. error may be NULL. */

void *_zip_allocate(zip_uint64_t nmemb, zip_uint64_t element_size, zip_uint64_t extra_bytes, zip_error_t *error) {
    size_t size;
    void *memory;

    if (!_zip_size_of_array(nmemb, element_size, extra_bytes, &size, error)) {
        return NULL;
    }

    if ((memory = malloc(size)) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    return memory;
}
