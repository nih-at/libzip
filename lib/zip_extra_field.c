/*
  zip_extra_field.c -- manipulate extra fields
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

#include <stdlib.h>
#include <string.h>

#include "zipint.h"


zip_extra_field_t *_zip_ef_clone(const zip_extra_field_t *ef, zip_error_t *error) {
    zip_extra_field_t *head, *prev, *def;

    head = prev = NULL;

    while (ef) {
        if ((def = _zip_ef_new(ef->id, ef->size, ef->data)) == NULL) {
            zip_error_set(error, ZIP_ER_MEMORY, 0);
            _zip_ef_free(head);
            return NULL;
        }

        if (head == NULL) {
            head = def;
        }
        if (prev) {
            prev->next = def;
        }
        prev = def;

        ef = ef->next;
    }

    return head;
}


zip_extra_field_t *_zip_ef_delete_by_id(zip_extra_field_t *ef, zip_uint16_t id, zip_uint16_t id_idx) {
    zip_extra_field_t *head, *prev;
    int i;

    i = 0;
    head = ef;
    prev = NULL;
    for (; ef; ef = (prev ? prev->next : head)) {
        if ((ef->id == id) || (id == ZIP_EXTRA_FIELD_ALL)) {
            if (id_idx == ZIP_EXTRA_FIELD_ALL || i == id_idx) {
                if (prev) {
                    prev->next = ef->next;
                }
                else {
                    head = ef->next;
                }
                ef->next = NULL;
                _zip_ef_free(ef);

                if (id_idx == ZIP_EXTRA_FIELD_ALL) {
                    continue;
                }
            }

            i++;
            if (i > id_idx) {
                break;
            }
        }
        prev = ef;
    }

    return head;
}


void _zip_ef_free(zip_extra_field_t *ef) {
    zip_extra_field_t *ef2;

    while (ef) {
        ef2 = ef->next;
        free(ef->data);
        free(ef);
        ef = ef2;
    }
}

zip_extra_field_t *_zip_ef_find(zip_extra_field_t *ef_head, zip_uint16_t ef_id, zip_uint16_t ef_idx, zip_extra_field_t **prev) {
    zip_extra_field_t *ef;
    zip_uint16_t i;

    i = 0;
    for (ef = ef_head; ef; ef = ef->next) {
        if (ef->id == ef_id) {
            if (i == ef_idx) {
                return ef;
            }
            i++;
        }
        if (prev) {
            *prev = ef;
        }
    }

    return NULL;
}


const zip_uint8_t *_zip_ef_get_by_id(const zip_extra_field_t *ef_head, zip_uint16_t *lenp, zip_uint16_t id, zip_uint16_t id_idx, zip_flags_t flags) {
    static const zip_uint8_t empty[1] = {'\0'};

    zip_extra_field_t *ef = _zip_ef_find((zip_extra_field_t *)ef_head, id, id_idx, NULL);

    if (ef) {
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

    return NULL;
}

zip_extra_field_t *_zip_ef_new(zip_uint16_t id, zip_uint16_t size, const zip_uint8_t *data) {
    zip_extra_field_t *ef;

    if ((ef = (zip_extra_field_t *)malloc(sizeof(*ef))) == NULL) {
        return NULL;
    }

    ef->next = NULL;
    ef->id = id;
    ef->size = size;
    if (size > 0) {
        if ((ef->data = (zip_uint8_t *)_zip_memdup(data, size, NULL)) == NULL) {
            free(ef);
            return NULL;
        }
    }
    else {
        ef->data = NULL;
    }

    return ef;
}


bool _zip_ef_parse(const zip_uint8_t *data, zip_uint16_t len, zip_flags_t flags, zip_extra_field_t **ef_head_p, zip_error_t *error) {
    zip_buffer_t *buffer;
    zip_extra_field_t *ef, *ef2, *ef_head;

    if ((buffer = _zip_buffer_new((zip_uint8_t *)data, len)) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return false;
    }

    ef_head = ef = NULL;

    while (_zip_buffer_ok(buffer) && _zip_buffer_left(buffer) >= 4) {
        zip_uint16_t fid, flen;
        zip_uint8_t *ef_data;

        fid = _zip_buffer_get_16(buffer);
        flen = _zip_buffer_get_16(buffer);
        ef_data = _zip_buffer_get(buffer, flen);

        if (ef_data == NULL) {
            zip_error_set(error, ZIP_ER_INCONS, ZIP_ER_DETAIL_INVALID_EF_LENGTH);
            _zip_buffer_free(buffer);
            _zip_ef_free(ef_head);
            return false;
        }

        if ((ef2 = _zip_ef_new(fid, flen, ef_data)) == NULL) {
            zip_error_set(error, ZIP_ER_MEMORY, 0);
            _zip_buffer_free(buffer);
            _zip_ef_free(ef_head);
            return false;
        }

        if (ef_head) {
            ef->next = ef2;
            ef = ef2;
        }
        else {
            ef_head = ef = ef2;
        }
    }

    if (!_zip_buffer_eof(buffer)) {
        /* Android APK files align stored file data with padding in extra fields; ignore. */
        /* see https://android.googlesource.com/platform/build/+/master/tools/zipalign/ZipAlign.cpp */
        /* buffer is at most 64k long, so this can't overflow. */
        size_t glen = _zip_buffer_left(buffer);
        zip_uint8_t *garbage;
        garbage = _zip_buffer_get(buffer, glen);
        if (glen >= 4 || garbage == NULL || memcmp(garbage, "\0\0\0", (size_t)glen) != 0) {
            zip_error_set(error, ZIP_ER_INCONS, ZIP_ER_DETAIL_EF_TRAILING_GARBAGE);
            _zip_buffer_free(buffer);
            _zip_ef_free(ef_head);
            return false;
        }
    }

    _zip_buffer_free(buffer);

    if (ef_head_p) {
        *ef_head_p = ef_head;
    }
    else {
        _zip_ef_free(ef_head);
    }

    return true;
}


zip_extra_field_t *_zip_ef_remove_internal(zip_extra_field_t *ef) {
    zip_extra_field_t *ef_head;
    zip_extra_field_t *prev, *next;

    ef_head = ef;
    prev = NULL;

    while (ef) {
        if (ZIP_EF_IS_INTERNAL(ef->id)) {
            next = ef->next;
            if (ef_head == ef) {
                ef_head = next;
            }
            ef->next = NULL;
            _zip_ef_free(ef);
            if (prev) {
                prev->next = next;
            }
            ef = next;
        }
        else {
            prev = ef;
            ef = ef->next;
        }
    }

    return ef_head;
}


/**
 * Calculate the size of the extra fields in bytes.
 *
 * @param ef the extra fields
 * @param flags which extra fields to include (ZIP_EF_LOCAL, ZIP_EF_CENTRAL, or both)
 * @return the size of the extra fields in bytes, or -1 if the size exceeds ZIP_UINT16_MAX.
 */
zip_int32_t _zip_ef_size(const zip_extra_field_t *ef) {
    zip_uint32_t size;

    size = 0;
    for (; ef; ef = ef->next) {
        size = (zip_uint32_t)(size + 4 + ef->size);
        if (size > ZIP_UINT16_MAX) {
            return -1;
        }
    }

    return (zip_int32_t)size;
}


int _zip_ef_write(zip_t *za, const zip_extra_field_t *ef) {
    zip_uint8_t b[4];
    zip_buffer_t *buffer = _zip_buffer_new(b, sizeof(b));

    if (buffer == NULL) {
        return -1;
    }

    for (; ef; ef = ef->next) {
        _zip_buffer_set_offset(buffer, 0);
        _zip_buffer_put_16(buffer, ef->id);
        _zip_buffer_put_16(buffer, ef->size);
        if (!_zip_buffer_ok(buffer)) {
            zip_error_set(&za->error, ZIP_ER_INTERNAL, 0);
            _zip_buffer_free(buffer);
            return -1;
        }
        if (_zip_write(za, b, 4) < 0) {
            _zip_buffer_free(buffer);
            return -1;
        }
        if (ef->size > 0) {
            if (_zip_write(za, ef->data, ef->size) < 0) {
                _zip_buffer_free(buffer);
                return -1;
            }
        }
    }

    _zip_buffer_free(buffer);
    return 0;
}


int _zip_read_local_ef(zip_t *za, zip_uint64_t idx) {
    zip_entry_t *e;
    unsigned char b[4];
    zip_buffer_t *buffer;
    zip_uint16_t fname_len, ef_len;

    if (idx >= za->nentry) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    e = za->entry + idx;

    if (e->orig == NULL || e->orig->local_extra_fields_read) {
        return 0;
    }

    if (e->orig->offset + 26 > ZIP_INT64_MAX) {
        zip_error_set(&za->error, ZIP_ER_SEEK, EFBIG);
        return -1;
    }

    if (zip_source_seek(za->src, (zip_int64_t)(e->orig->offset + 26), SEEK_SET) < 0) {
        zip_error_set_from_source(&za->error, za->src);
        return -1;
    }

    if ((buffer = _zip_buffer_new_from_source(za->src, sizeof(b), b, &za->error)) == NULL) {
        return -1;
    }

    fname_len = _zip_buffer_get_16(buffer);
    ef_len = _zip_buffer_get_16(buffer);

    if (!_zip_buffer_eof(buffer)) {
        _zip_buffer_free(buffer);
        zip_error_set(&za->error, ZIP_ER_INTERNAL, 0);
        return -1;
    }

    _zip_buffer_free(buffer);

    if (ef_len > 0) {
        zip_extra_field_t *ef;
        zip_uint8_t *ef_raw;

        if (zip_source_seek(za->src, fname_len, SEEK_CUR) < 0) {
            zip_error_set(&za->error, ZIP_ER_SEEK, errno);
            return -1;
        }

        ef_raw = _zip_read_data(NULL, za->src, ef_len, 0, &za->error);

        if (ef_raw == NULL) {
            return -1;
        }

        if (!_zip_ef_parse(ef_raw, ef_len, ZIP_EF_LOCAL, &ef, &za->error)) {
            free(ef_raw);
            return -1;
        }
        free(ef_raw);

        if (ef) {
            ef = _zip_ef_remove_internal(ef);
            e->orig->extra_fields.local = ef;
        }
    }

    e->orig->local_extra_fields_read = 1;

    if (e->changes && e->changes->local_extra_fields_read == 0) {
        e->changes->extra_fields = e->orig->extra_fields;
        e->changes->local_extra_fields_read = 1;
    }

    return 0;
}

void _zip_extrafields_delete_by_id(zip_extra_fields_t *fields, zip_uint16_t ef_id, zip_uint16_t ef_idx, zip_flags_t flags) {
    if (flags & ZIP_EF_LOCAL) {
        fields->local = _zip_ef_delete_by_id(fields->local, ef_id, ef_idx);
    }
    if (flags & ZIP_EF_CENTRAL) {
        fields->central = _zip_ef_delete_by_id(fields->central, ef_id, ef_idx);
    }
}

const zip_uint8_t *_zip_extra_fields_get_by_id(const zip_extra_fields_t *extra_fields, zip_uint16_t *lenp, zip_uint16_t extra_field_id, zip_uint16_t extra_field_index, zip_flags_t flags, zip_error_t *error) {
    const zip_uint8_t *data;

    if (flags & ZIP_EF_LOCAL) {
        data = _zip_ef_get_by_id(extra_fields->local, lenp, extra_field_id, extra_field_index, flags);
        if (data) {
            return data;
        }
    }

    if (flags & ZIP_EF_CENTRAL) {
        data = _zip_ef_get_by_id(extra_fields->central, lenp, extra_field_id, extra_field_index, flags);
        if (data) {
            return data;
        }
    }

    zip_error_set(error, ZIP_ER_NOENT, 0);
    return NULL;
}

zip_int16_t _zip_ef_count(const zip_extra_field_t *ef, zip_int32_t id) {
    zip_int16_t n;

    n = 0;
    for (; ef; ef = ef->next) {
        if (id < 0 || ef->id == id) {
            n++;
        }
    }

    return n;
}

zip_int16_t _zip_extra_fields_count(const zip_extra_fields_t *extra_fields, zip_int32_t id, zip_flags_t flags) {
    zip_int16_t n;

    n = 0;
    if (flags & ZIP_EF_LOCAL) {
        n += _zip_ef_count(extra_fields->local, id);
    }
    if (flags & ZIP_EF_CENTRAL) {
        n += _zip_ef_count(extra_fields->central, id);
    }

    return n;
}


zip_extra_field_t *_zip_ef_set(zip_extra_field_t *ef_head, zip_uint16_t ef_id, zip_uint16_t ef_idx, const zip_uint8_t *data, zip_uint16_t len, zip_error_t *error) {
    zip_extra_field_t *ef_prev = NULL;
    zip_extra_field_t *ef_new, *ef;
    zip_int32_t new_len;

    ef = _zip_ef_find(ef_head, ef_id, ef_idx, &ef_prev);

    if (ef == NULL && ef_idx != ZIP_EXTRA_FIELD_NEW) {
        zip_error_set(error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    new_len = _zip_ef_size(ef_head);
    /* This should not happen, but we want to assume lengths are >= 0 later. */
    if (new_len < 0) {
        zip_error_set(error, ZIP_ER_EF_TOO_LARGE, 0);
        return NULL;
    }

    if (ef != NULL) {
        new_len -= ef->size + 4;
    }
    new_len += len + 4;

    if (new_len > ZIP_UINT16_MAX) {
        zip_error_set(error, ZIP_ER_EF_TOO_LARGE, 0);
        return NULL;
    }

    /* If we are updating an existing field with its current data pointer, just update the size.
       This also ensures that data remains valid, which is needed if we set both local and central extra fields. */
    if (ef && ef->data == data) {
        /* Since ef->data can be user supplied, we don't know its allocation size, so a bigger len may be fine. */
        ef->size = len;
        return ef_head;
    }

    /* Otherwise, create a new extra field. */
    if ((ef_new = _zip_ef_new(ef_id, len, data)) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    if (ef != NULL) {
        ef_new->next = ef->next;
        ef->next = NULL;
        _zip_ef_free(ef);
    }
    else if (ef_prev) {
        ef_new->next = ef_prev->next;
    }

    if (ef_prev) {
        ef_prev->next = ef_new;
    }
    else {
        ef_head = ef_new;
    }

    return ef_head;
}

bool _zip_extra_fields_set(zip_extra_fields_t *extra_fields, zip_flags_t flags, zip_uint16_t ef_id, zip_uint16_t ef_idx, const zip_uint8_t *data, zip_uint16_t len, zip_error_t *error) {
    zip_extra_field_t *ef;

    if (flags & ZIP_EF_LOCAL) {
        ef = _zip_ef_set(extra_fields->local, ef_id, ef_idx, data, len, error);
        if (ef == NULL) {
            return false;
        }
        extra_fields->local = ef;
    }
    if (flags & ZIP_EF_CENTRAL) {
        ef = _zip_ef_set(extra_fields->central, ef_id, ef_idx, data, len, error);
        if (ef == NULL) {
            return false;
        }
        extra_fields->central = ef;
    }

    return true;
}

bool _zip_extra_fields_clone(zip_extra_fields_t *extra_fields, zip_error_t *error) {
    if (extra_fields->local) {
        if ((extra_fields->local = _zip_ef_clone(extra_fields->local, error)) == NULL) {
            /* Clear central so we don't refer to the original. */
            extra_fields->central = NULL;
            return false;
        }
    }
    if (extra_fields->central) {
        if ((extra_fields->central = _zip_ef_clone(extra_fields->central, error)) == NULL) {
            return false;
        }
    }

    return true;
}


void _zip_extra_fields_fini(zip_extra_fields_t *extra_fields) {
    _zip_ef_free(extra_fields->local);
    extra_fields->local = NULL;
    _zip_ef_free(extra_fields->central);
    extra_fields->central = NULL;
}

void _zip_extra_fields_init(zip_extra_fields_t *extra_fields) {
    extra_fields->local = NULL;
    extra_fields->central = NULL;
}


void _zip_extra_fields_delete_by_id(zip_extra_fields_t *extra_fields, zip_uint16_t extra_field_id, zip_uint16_t extra_field_index, zip_flags_t flags) {
    if (flags & ZIP_EF_LOCAL) {
        extra_fields->local = _zip_ef_delete_by_id(extra_fields->local, extra_field_id, extra_field_index);
    }
    if (flags & ZIP_EF_CENTRAL) {
        extra_fields->central = _zip_ef_delete_by_id(extra_fields->central, extra_field_id, extra_field_index);
    }
}
