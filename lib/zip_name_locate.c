/*
  zip_name_locate.c -- get index by name
  Copyright (C) 1999-2022 Dieter Baron and Thomas Klausner

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


#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "zipint.h"


ZIP_EXTERN zip_int64_t zip_name_locate(zip_t *za, const char *fname, zip_flags_t flags) {
    return _zip_name_locate(za, fname, flags, &za->error);
}


zip_int64_t _zip_name_locate(zip_t *za, const char *fname, zip_flags_t flags, zip_error_t *error) {
    size_t fname_length;
    zip_string_t *str = NULL;
    const char *fn, *p;
    zip_uint64_t i;

    if (za == NULL) {
        return -1;
    }

    if (fname == NULL) {
        zip_error_set(error, ZIP_ER_INVAL, 0);
        return -1;
    }

    fname_length = strlen(fname);

    if (fname_length > ZIP_UINT16_MAX) {
        zip_error_set(error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if ((flags & (ZIP_FL_ENC_UTF_8 | ZIP_FL_ENC_RAW)) == 0 && fname[0] != '\0') {
        zip_uint32_t converted_length;

        if ((str = _zip_string_new((const zip_uint8_t *)fname, (zip_uint16_t)strlen(fname), flags, error)) == NULL) {
            return -1;
        }
        if ((fname = (const char *)_zip_string_get(str, &converted_length, 0, error)) == NULL) {
            _zip_string_free(str);
            return -1;
        }
        fname_length = converted_length;
    }

    if (flags & (ZIP_FL_NOCASE | ZIP_FL_NODIR | ZIP_FL_ENC_RAW | ZIP_FL_ENC_STRICT)) {
        /* can't use hash table */

        for (i = 0; i < za->nentry; i++) {
            zip_uint32_t fn_length;
            size_t fn_compare_length;

            fn = _zip_get_name_len(za, i, flags, &fn_length, error);

            /* newly added (partially filled) entry or error */
            if (fn == NULL) {
                continue;
            }

            fn_compare_length = fn_length;

            if (flags & ZIP_FL_NODIR) {
                p = strrchr(fn, '/');
                if (p) {
                    fn_compare_length -= (size_t)(p + 1 - fn);
                    fn = p + 1;
                }
            }

            /* compare against the full name, so that a name which only
               matches up to an embedded NUL byte is not found */
            if (fn_compare_length == fname_length && ((flags & ZIP_FL_NOCASE) ? strncasecmp(fname, fn, fname_length) : memcmp(fname, fn, fname_length)) == 0) {
                _zip_error_clear(error);
                _zip_string_free(str);
                return (zip_int64_t)i;
            }
        }

        zip_error_set(error, ZIP_ER_NOENT, 0);
        _zip_string_free(str);
        return -1;
    }
    else {
        zip_int64_t ret = _zip_hash_lookup(za->names, (const zip_uint8_t *)fname, (zip_uint32_t)fname_length, flags, error);
        _zip_string_free(str);
        return ret;
    }
}
