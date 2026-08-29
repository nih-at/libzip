/*
  zip_extra_field_api.c -- public extra fields API functions
  Copyright (C) 2012-2024 Dieter Baron and Thomas Klausner

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


#include "zipint.h"


ZIP_EXTERN bool zip_file_extra_field_delete(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_idx, zip_flags_t flags) {
    zip_dirent_t *de;

    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return false;
    }

    if (((flags & ZIP_EF_BOTH) == ZIP_EF_BOTH) && (ef_idx != ZIP_EXTRA_FIELD_ALL)) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return false;
    }

    if (_zip_get_dirent(za, idx, 0, NULL) == NULL) {
        return false;
    }

    if (ZIP_IS_RDONLY(za)) {
        zip_error_set(&za->error, ZIP_ER_RDONLY, 0);
        return false;
    }

    if (_zip_file_extra_field_prepare_for_change(za, idx) < 0) {
        return false;
    }

    de = za->entry[idx].changes;

    _zip_extra_fields_delete_by_id(&de->extra_fields, ZIP_EXTRA_FIELD_ALL, ef_idx, flags);
    return true;
}


ZIP_EXTERN int zip_file_extra_field_delete_by_id(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_id, zip_uint16_t ef_idx, zip_flags_t flags) {
    zip_dirent_t *de;

    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if (((flags & ZIP_EF_BOTH) == ZIP_EF_BOTH) && (ef_idx != ZIP_EXTRA_FIELD_ALL)) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if (_zip_get_dirent(za, idx, 0, NULL) == NULL) {
        return -1;
    }

    if (ZIP_IS_RDONLY(za)) {
        zip_error_set(&za->error, ZIP_ER_RDONLY, 0);
        return -1;
    }
    if (ZIP_WANT_TORRENTZIP(za)) {
        zip_error_set(&za->error, ZIP_ER_NOT_ALLOWED, 0);
        return -1;
    }

    if (_zip_file_extra_field_prepare_for_change(za, idx) < 0) {
        return -1;
    }

    de = za->entry[idx].changes;

    _zip_extra_fields_delete_by_id(&de->extra_fields, ef_id, ef_idx, flags);

    return 0;
}


ZIP_EXTERN const zip_uint8_t *zip_file_extra_field_get(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_idx, zip_uint16_t *idp, zip_uint16_t *lenp, zip_flags_t flags) {
    static const zip_uint8_t empty[1] = {'\0'};

    zip_dirent_t *de;
    zip_extra_field_t *ef;

    if ((flags & ZIP_EF_BOTH) == 0 || ((flags & ZIP_EF_BOTH) == ZIP_EF_BOTH)) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    if ((de = _zip_get_dirent(za, idx, flags, &za->error)) == NULL) {
        return NULL;
    }

    if (flags & ZIP_FL_LOCAL) {
        if (_zip_read_local_ef(za, idx) < 0) {
            return NULL;
        }
    }

    ef = (flags & ZIP_FL_LOCAL) ? de->extra_fields.local : de->extra_fields.central;
    while (ef_idx > 0 && ef) {
        ef = ef->next;
        ef_idx--;
    }

    if (ef) {
        if (idp) {
            *idp = ef->id;
        }
        if (lenp) {
            *lenp = ef->size;
        }
        if (ef->size > 0) {
            return ef->data;
        }
        else {
            return empty;
        }
    }

    zip_error_set(&za->error, ZIP_ER_NOENT, 0);
    return NULL;
}


ZIP_EXTERN const zip_uint8_t *zip_file_extra_field_get_by_id(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_id, zip_uint16_t ef_idx, zip_uint16_t *lenp, zip_flags_t flags) {
    zip_dirent_t *de;

    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    if ((de = _zip_get_dirent(za, idx, flags, &za->error)) == NULL) {
        return NULL;
    }

    if (flags & ZIP_FL_LOCAL) {
        if (_zip_read_local_ef(za, idx) < 0) {
            return NULL;
        }
    }

    return _zip_extra_fields_get_by_id(&de->extra_fields, lenp, ef_id, ef_idx, flags, &za->error);
}


ZIP_EXTERN zip_int16_t zip_file_extra_fields_count(zip_t *za, zip_uint64_t idx, zip_flags_t flags) {
    zip_dirent_t *de;

    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if ((de = _zip_get_dirent(za, idx, flags, &za->error)) == NULL) {
        return -1;
    }

    if (flags & ZIP_FL_LOCAL) {
        if (_zip_read_local_ef(za, idx) < 0) {
            return -1;
        }
    }

    return _zip_extra_fields_count(&de->extra_fields, -1, flags);
}


ZIP_EXTERN zip_int16_t zip_file_extra_fields_count_by_id(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_id, zip_flags_t flags) {
    zip_dirent_t *de;

    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if ((de = _zip_get_dirent(za, idx, flags, &za->error)) == NULL) {
        return -1;
    }

    if (flags & ZIP_FL_LOCAL) {
        if (_zip_read_local_ef(za, idx) < 0) {
            return -1;
        }
    }

    return _zip_extra_fields_count(&de->extra_fields, (zip_int32_t)ef_id, flags);
}


ZIP_EXTERN int zip_file_extra_field_set(zip_t *za, zip_uint64_t idx, zip_uint16_t ef_id, zip_uint16_t ef_idx, const zip_uint8_t *data, zip_uint16_t len, zip_flags_t flags) {
    if ((flags & ZIP_EF_BOTH) == 0) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if (_zip_get_dirent(za, idx, 0, NULL) == NULL) {
        return -1;
    }

    if (ZIP_IS_RDONLY(za)) {
        zip_error_set(&za->error, ZIP_ER_RDONLY, 0);
        return -1;
    }
    if (ZIP_WANT_TORRENTZIP(za)) {
        zip_error_set(&za->error, ZIP_ER_NOT_ALLOWED, 0);
        return -1;
    }

    if (ZIP_EF_IS_INTERNAL(ef_id)) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if (_zip_file_extra_field_prepare_for_change(za, idx) < 0) {
        return -1;
    }

    return _zip_extra_fields_set(&za->entry[idx].changes->extra_fields, flags, ef_id, ef_idx, data, len, &za->error);
}


int _zip_file_extra_field_prepare_for_change(zip_t *za, zip_uint64_t idx) {
    zip_entry_t *e;

    if (idx >= za->nentry) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    e = za->entry + idx;

    if (e->changes && (e->changes->changed & ZIP_DIRENT_EXTRA_FIELD)) {
        return 0;
    }

    if (e->orig) {
        if (_zip_read_local_ef(za, idx) < 0) {
            return -1;
        }
    }

    if (e->changes == NULL) {
        if ((e->changes = _zip_dirent_clone(e->orig)) == NULL) {
            zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
            return -1;
        }
    }

    _zip_extra_fields_clone(&e->changes->extra_fields, &za->error);
    e->changes->changed |= ZIP_DIRENT_EXTRA_FIELD;

    return 0;
}
