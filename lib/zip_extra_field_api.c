/*
  zip_extra_field_api.c -- public extra fields API functions
  Copyright (C) 2012 Dieter Baron and Thomas Klausner

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



#include "zipint.h"



ZIP_EXTERN int
zip_delete_file_extra_field(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_idx)
{
    /* XXX */
    return -1;
}



ZIP_EXTERN int
zip_delete_file_extra_field_by_id(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_id, zip_uint16_t ef_idx)
{
    /* XXX */
    return -1;
}



ZIP_EXTERN const zip_uint8_t *
zip_get_file_extra_field(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_idx, zip_uint16_t *idp, zip_uint16_t *lenp)
{
    static const zip_uint8_t empty[1];

    struct zip_dirent *de;
    struct zip_extra_field *ef;
    int i;

    if ((de=_zip_get_dirent(za, idx, flags, &za->error)) == NULL)
	return NULL;

    if ((flags & ZIP_EF_BOTH) == 0)
	flags = ZIP_EF_BOTH;

    if (flags & ZIP_FL_LOCAL)
	if (_zip_read_local_ef(za, idx) < 0)
	    return NULL;

    i = 0;
    for (ef=de->extra_fields; ef; ef=ef->next) {
	if (ef->flags & flags & ZIP_EF_BOTH) {
	    if (i < ef_idx) {
		i++;
		continue;
	    }

	    if (idp)
		*idp = ef->id;
	    if (lenp)
		*lenp = ef->size;
	    if (ef->size > 0)
		return ef->data;
	    else
		return empty;
	}
    }

    _zip_error_set(&za->error, ZIP_ER_NOENT, 0);
    return NULL;

}



ZIP_EXTERN const zip_uint8_t *
zip_get_file_extra_field_by_id(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_id, zip_uint16_t ef_idx, zip_uint16_t *lenp)
{
    struct zip_dirent *de;

    if ((de=_zip_get_dirent(za, idx, flags, &za->error)) == NULL)
	return NULL;

    if ((flags & ZIP_EF_BOTH) == 0)
	flags = ZIP_EF_BOTH;

    if (flags & ZIP_FL_LOCAL)
	if (_zip_read_local_ef(za, idx) < 0)
	    return NULL;

    return _zip_ef_get_by_id(de->extra_fields, lenp, ef_id, ef_idx, flags, &za->error);
}



ZIP_EXTERN zip_int16_t
zip_get_file_num_extra_fields(struct zip *za, zip_uint64_t idx, zip_uint32_t flags)
{
    struct zip_dirent *de;
    struct zip_extra_field *ef;
    zip_uint16_t n;

    if ((de=_zip_get_dirent(za, idx, flags, &za->error)) == NULL)
	return -1;

    if ((flags & ZIP_EF_BOTH) == 0)
	flags = ZIP_EF_BOTH;

    if (flags & ZIP_FL_LOCAL)
	if (_zip_read_local_ef(za, idx) < 0)
	    return -1;

    n = 0;
    for (ef=de->extra_fields; ef; ef=ef->next)
	if (ef->flags & flags & ZIP_EF_BOTH)
	    n++;

    return n;
}



ZIP_EXTERN zip_int16_t
zip_get_file_num_extra_fields_by_id(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_id)
{
    struct zip_dirent *de;
    struct zip_extra_field *ef;
    zip_uint16_t n;

    if ((de=_zip_get_dirent(za, idx, flags, &za->error)) == NULL)
	return -1;

    if ((flags & ZIP_EF_BOTH) == 0)
	flags = ZIP_EF_BOTH;

    if (flags & ZIP_FL_LOCAL)
	if (_zip_read_local_ef(za, idx) < 0)
	    return -1;

    n = 0;
    for (ef=de->extra_fields; ef; ef=ef->next)
	if (ef->id == ef_id && (ef->flags & flags & ZIP_EF_BOTH))
	    n++;

    return n;
}



ZIP_EXTERN int
zip_set_file_extra_field(struct zip *za, zip_uint64_t idx, zip_uint32_t flags, zip_uint16_t ef_id, zip_uint16_t ef_idx, const zip_uint8_t *data, zip_uint16_t len)
{
    /* XXX */
    return -1;
}
