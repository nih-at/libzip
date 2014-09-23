/*
  zip_dirent.c -- read directory entry (local or central), clean dirent
  Copyright (C) 1999-2014 Dieter Baron and Thomas Klausner

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "zipint.h"

static time_t _zip_d2u_time(zip_uint16_t, zip_uint16_t);
static struct zip_string *_zip_dirent_process_ef_utf_8(const struct zip_dirent *de, zip_uint16_t id, struct zip_string *str);
static struct zip_extra_field *_zip_ef_utf8(zip_uint16_t, struct zip_string *, struct zip_error *);


void
_zip_cdir_free(struct zip_cdir *cd)
{
    zip_uint64_t i;

    if (!cd)
	return;

    for (i=0; i<cd->nentry; i++)
	_zip_entry_finalize(cd->entry+i);
    free(cd->entry);
    _zip_string_free(cd->comment);
    free(cd);
}


int
_zip_cdir_grow(struct zip_cdir *cd, zip_uint64_t nentry, struct zip_error *error)
{
    struct zip_entry *entry;
    zip_uint64_t i;

    if (nentry < cd->nentry_alloc) {
	zip_error_set(error, ZIP_ER_INTERNAL, 0);
	return -1;
    }

    if (nentry == cd->nentry_alloc)
	return 0;

    if ((entry=((struct zip_entry *)
		realloc(cd->entry, sizeof(*(cd->entry))*(size_t)nentry))) == NULL) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	return -1;
    }
    
    for (i=cd->nentry_alloc; i<nentry; i++)
	_zip_entry_init(entry+i);

    cd->nentry_alloc = nentry;
    cd->entry = entry;

    return 0;
}


struct zip_cdir *
_zip_cdir_new(zip_uint64_t nentry, struct zip_error *error)
{
    struct zip_cdir *cd;
    zip_uint64_t i;
    
    if ((cd=(struct zip_cdir *)malloc(sizeof(*cd))) == NULL) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    if (nentry == 0)
	cd->entry = NULL;
    else if ((cd->entry=(struct zip_entry *)malloc(sizeof(*(cd->entry))*(size_t)nentry)) == NULL) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	free(cd);
	return NULL;
    }

    for (i=0; i<nentry; i++)
	_zip_entry_init(cd->entry+i);

    cd->nentry = cd->nentry_alloc = nentry;
    cd->size = cd->offset = 0;
    cd->comment = NULL;

    return cd;
}


zip_int64_t
_zip_cdir_write(struct zip *za, const struct zip_filelist *filelist, zip_uint64_t survivors)
{
    zip_uint64_t offset, size;
    struct zip_string *comment;
    zip_uint8_t buf[EOCDLEN + EOCD64LEN], *p;
    zip_int64_t off;
    zip_uint64_t i;
    int is_zip64;
    int ret;

    if ((off = zip_source_tell_write(za->src)) < 0) {
        _zip_error_set_from_source(&za->error, za->src);
        return -1;
    }
    offset = (zip_uint64_t)off;

    is_zip64 = 0;

    for (i=0; i<survivors; i++) {
	struct zip_entry *entry = za->entry+filelist[i].idx;

	if ((ret=_zip_dirent_write(za, entry->changes ? entry->changes : entry->orig, ZIP_FL_CENTRAL)) < 0)
	    return -1;
	if (ret)
	    is_zip64 = 1;
    }

    if ((off = zip_source_tell_write(za->src)) < 0) {
        _zip_error_set_from_source(&za->error, za->src);
        return -1;
    }
    size = (zip_uint64_t)off - offset;

    if (offset > ZIP_UINT32_MAX || survivors > ZIP_UINT16_MAX)
	is_zip64 = 1;

    p = buf;
    if (is_zip64) {
	_zip_put_data(&p, EOCD64_MAGIC, 4);
	_zip_put_64(&p, EOCD64LEN-12);
	_zip_put_16(&p, 45);
	_zip_put_16(&p, 45);
	_zip_put_32(&p, 0);
	_zip_put_32(&p, 0);
	_zip_put_64(&p, survivors);
	_zip_put_64(&p, survivors);
	_zip_put_64(&p, size);
	_zip_put_64(&p, offset);
	_zip_put_data(&p, EOCD64LOC_MAGIC, 4);
	_zip_put_32(&p, 0);
	_zip_put_64(&p, offset+size);
	_zip_put_32(&p, 1);
    }
    
    _zip_put_data(&p, EOCD_MAGIC, 4);
    _zip_put_32(&p, 0);
    _zip_put_16(&p, survivors >= ZIP_UINT16_MAX ? ZIP_UINT16_MAX : (zip_uint16_t)survivors);
    _zip_put_16(&p, survivors >= ZIP_UINT16_MAX ? ZIP_UINT16_MAX : (zip_uint16_t)survivors);
    _zip_put_32(&p, size >= ZIP_UINT32_MAX ? ZIP_UINT32_MAX : (zip_uint32_t)size);
    _zip_put_32(&p, offset >= ZIP_UINT32_MAX ? ZIP_UINT32_MAX : (zip_uint32_t)offset);

    comment = za->comment_changed ? za->comment_changes : za->comment_orig;

    _zip_put_16(&p, comment ? comment->length : 0);

    if (_zip_write(za, buf, (zip_uint64_t)(p-buf)) < 0) {
	return -1;
    }

    if (comment) {
	if (_zip_write(za, comment->raw, comment->length) < 0) {
	    return -1;
	}
    }

    return (zip_int64_t)size;
}


struct zip_dirent *
_zip_dirent_clone(const struct zip_dirent *sde)
{
    struct zip_dirent *tde;

    if ((tde=(struct zip_dirent *)malloc(sizeof(*tde))) == NULL)
	return NULL;

    if (sde)
	memcpy(tde, sde, sizeof(*sde));
    else
	_zip_dirent_init(tde);
    
    tde->changed = 0;
    tde->cloned = 1;

    return tde;
}


void
_zip_dirent_finalize(struct zip_dirent *zde)
{
    if (!zde->cloned || zde->changed & ZIP_DIRENT_FILENAME)
	_zip_string_free(zde->filename);
    if (!zde->cloned || zde->changed & ZIP_DIRENT_EXTRA_FIELD)
	_zip_ef_free(zde->extra_fields);
    if (!zde->cloned || zde->changed & ZIP_DIRENT_COMMENT)
	_zip_string_free(zde->comment);
}


void
_zip_dirent_free(struct zip_dirent *zde)
{
    if (zde == NULL)
	return;

    _zip_dirent_finalize(zde);
    free(zde);
}


void
_zip_dirent_init(struct zip_dirent *de)
{
    de->changed = 0;
    de->local_extra_fields_read = 0;
    de->cloned = 0;

    de->version_madeby = 20 | (ZIP_OPSYS_DEFAULT << 8);
    de->version_needed = 20; /* 2.0 */
    de->bitflags = 0;
    de->comp_method = ZIP_CM_DEFAULT;
    de->last_mod = 0;
    de->crc = 0;
    de->comp_size = 0;
    de->uncomp_size = 0;
    de->filename = NULL;
    de->extra_fields = NULL;
    de->comment = NULL;
    de->disk_number = 0;
    de->int_attrib = 0;
    de->ext_attrib = ZIP_EXT_ATTRIB_DEFAULT;
    de->offset = 0;
}


int
_zip_dirent_needs_zip64(const struct zip_dirent *de, zip_flags_t flags)
{
    if (de->uncomp_size >= ZIP_UINT32_MAX || de->comp_size >= ZIP_UINT32_MAX
	|| ((flags & ZIP_FL_CENTRAL) && de->offset >= ZIP_UINT32_MAX))
	return 1;

    return 0;
}


struct zip_dirent *
_zip_dirent_new(void)
{
    struct zip_dirent *de;

    if ((de=(struct zip_dirent *)malloc(sizeof(*de))) == NULL)
	return NULL;

    _zip_dirent_init(de);
    return de;
}


/* _zip_dirent_read(zde, fp, bufp, left, localp, error):
   Fills the zip directory entry zde.

   If bufp is non-NULL, data is taken from there and bufp is advanced
   by the amount of data used; otherwise data is read from fp as needed.
   
   if leftp is non-NULL, no more bytes than specified by it are used,
   and *leftp is reduced by the number of bytes used.

   If local != 0, it reads a local header instead of a central
   directory entry.

   Returns 0 if successful. On error, error is filled in and -1 is
   returned.

   TODO: leftp and file position undefined on error.
*/

int
_zip_dirent_read(struct zip_dirent *zde, struct zip_source *src,
		 const unsigned char **bufp, zip_uint64_t *leftp, int local,
		 struct zip_error *error)
{
    unsigned char buf[CDENTRYSIZE];
    const unsigned char *cur;
    zip_uint16_t dostime, dosdate;
    zip_uint32_t size;
    zip_uint16_t filename_len, comment_len, ef_len;

    if (local)
	size = LENTRYSIZE;
    else
	size = CDENTRYSIZE;

    if (leftp && (*leftp < size)) {
	zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
    }

    if (bufp) {
	/* use data from buffer */
	cur = *bufp;
    }
    else {
	zip_int64_t n;

	if ((n = zip_source_read(src, buf, size)) < 0) {
            _zip_error_set_from_source(error, src);
	    return -1;
	}
	if ((zip_uint64_t)n != size) {
	    zip_error_set(error, ZIP_ER_NOZIP, errno);
	    return -1;
	}
	cur = buf;
    }

    if (memcmp(cur, (local ? LOCAL_MAGIC : CENTRAL_MAGIC), 4) != 0) {
	zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
    }
    cur += 4;


    /* convert buffercontents to zip_dirent */

    _zip_dirent_init(zde);
    if (!local)
	zde->version_madeby = _zip_get_16(&cur);
    else
	zde->version_madeby = 0;
    zde->version_needed = _zip_get_16(&cur);
    zde->bitflags = _zip_get_16(&cur);
    zde->comp_method = _zip_get_16(&cur);
    
    /* convert to time_t */
    dostime = _zip_get_16(&cur);
    dosdate = _zip_get_16(&cur);
    zde->last_mod = _zip_d2u_time(dostime, dosdate);
    
    zde->crc = _zip_get_32(&cur);
    zde->comp_size = _zip_get_32(&cur);
    zde->uncomp_size = _zip_get_32(&cur);
    
    filename_len = _zip_get_16(&cur);
    ef_len = _zip_get_16(&cur);
    
    if (local) {
	comment_len = 0;
	zde->disk_number = 0;
	zde->int_attrib = 0;
	zde->ext_attrib = 0;
	zde->offset = 0;
    } else {
	comment_len = _zip_get_16(&cur);
	zde->disk_number = _zip_get_16(&cur);
	zde->int_attrib = _zip_get_16(&cur);
	zde->ext_attrib = _zip_get_32(&cur);
	zde->offset = _zip_get_32(&cur);
    }

    zde->filename = NULL;
    zde->extra_fields = NULL;
    zde->comment = NULL;

    size += (zip_uint32_t)filename_len+ef_len+comment_len;

    if (leftp && (*leftp < size)) {
	zip_error_set(error, ZIP_ER_INCONS, 0);
	return -1;
    }

    if (filename_len) {
	zde->filename = _zip_read_string(bufp ? &cur : NULL, src, filename_len, 1, error);
        if (!zde->filename) {
            if (zip_error_code_zip(error) == ZIP_ER_EOF) {
                zip_error_set(error, ZIP_ER_INCONS, 0);
            }
	    return -1;
        }

	if (zde->bitflags & ZIP_GPBF_ENCODING_UTF_8) {
	    if (_zip_guess_encoding(zde->filename, ZIP_ENCODING_UTF8_KNOWN) == ZIP_ENCODING_ERROR) {
		zip_error_set(error, ZIP_ER_INCONS, 0);
		return -1;
	    }
	}
    }

    if (ef_len) {
	zip_uint8_t *ef = _zip_read_data(bufp ? &cur : NULL, src, ef_len, 0, error);

	if (ef == NULL)
	    return -1;
	if ((zde->extra_fields=_zip_ef_parse(ef, ef_len, local ? ZIP_EF_LOCAL : ZIP_EF_CENTRAL, error)) == NULL) {
	    free(ef);
	    return -1;
	}
	free(ef);
	if (local)
	    zde->local_extra_fields_read = 1;
    }

    if (comment_len) {
	zde->comment = _zip_read_string(bufp ? &cur : NULL, src, comment_len, 0, error);
	if (!zde->comment)
	    return -1;

	if (zde->bitflags & ZIP_GPBF_ENCODING_UTF_8) {
	    if (_zip_guess_encoding(zde->comment, ZIP_ENCODING_UTF8_KNOWN) == ZIP_ENCODING_ERROR) {
		zip_error_set(error, ZIP_ER_INCONS, 0);
		return -1;
	    }
	}
    }

    zde->filename = _zip_dirent_process_ef_utf_8(zde, ZIP_EF_UTF_8_NAME, zde->filename);
    zde->comment = _zip_dirent_process_ef_utf_8(zde, ZIP_EF_UTF_8_COMMENT, zde->comment);

    /* Zip64 */

    if (zde->uncomp_size == ZIP_UINT32_MAX || zde->comp_size == ZIP_UINT32_MAX || zde->offset == ZIP_UINT32_MAX) {
	zip_uint32_t needed_len;
	zip_uint16_t got_len;
	const zip_uint8_t *ef = _zip_ef_get_by_id(zde->extra_fields, &got_len, ZIP_EF_ZIP64, 0, local ? ZIP_EF_LOCAL : ZIP_EF_CENTRAL, error);
	/* TODO: if got_len == 0 && !ZIP64_EOCD: no error, 0xffffffff is valid value */
	if (ef == NULL)
	    return -1;


	if (local)
	    needed_len = 16;
	else {
	    needed_len = 0;
	    if (zde->uncomp_size == ZIP_UINT32_MAX)
		needed_len += 8;
	    if (zde->comp_size == ZIP_UINT32_MAX)
		needed_len += 8;
	    if (zde->offset == ZIP_UINT32_MAX)
		needed_len += 8;
	    if (zde->disk_number == ZIP_UINT16_MAX)
		needed_len += 4;
	}

	if (got_len != needed_len) {
	    zip_error_set(error, ZIP_ER_INCONS, 0);
	    return -1;
	}
	
	if (zde->uncomp_size == ZIP_UINT32_MAX)
	    zde->uncomp_size = _zip_get_64(&ef);
	else if (local)
	    ef += 8;
	if (zde->comp_size == ZIP_UINT32_MAX)
	    zde->comp_size = _zip_get_64(&ef);
	if (!local) {
	    if (zde->offset == ZIP_UINT32_MAX)
		zde->offset = _zip_get_64(&ef);
	    if (zde->disk_number == ZIP_UINT16_MAX)
		zde->disk_number = _zip_get_32(&ef);
	}
    }
    
    /* zip_source_seek / zip_source_tell don't support values > ZIP_INT64_MAX */
    if (zde->offset > ZIP_INT64_MAX) {
	zip_error_set(error, ZIP_ER_SEEK, EFBIG);
	return -1;
    }
    
    zde->extra_fields = _zip_ef_remove_internal(zde->extra_fields);

    if (bufp)
      *bufp = cur;
    if (leftp)
	*leftp -= size;

    return 0;
}


static struct zip_string *
_zip_dirent_process_ef_utf_8(const struct zip_dirent *de, zip_uint16_t id, struct zip_string *str)
{
    zip_uint16_t ef_len;
    zip_uint32_t ef_crc;

    const zip_uint8_t *ef = _zip_ef_get_by_id(de->extra_fields, &ef_len, id, 0, ZIP_EF_BOTH, NULL);

    if (ef == NULL || ef_len < 5 || ef[0] != 1)
	return str;

    ef++;
    ef_crc = _zip_get_32(&ef);

    if (_zip_string_crc32(str) == ef_crc) {
	struct zip_string *ef_str = _zip_string_new(ef, (zip_uint16_t)(ef_len-5), ZIP_FL_ENC_UTF_8, NULL);

	if (ef_str != NULL) {
	    _zip_string_free(str);
	    str = ef_str;
	}
    }
    
    return str;
}


zip_int32_t
_zip_dirent_size(zip_source_t *src, zip_uint16_t flags, zip_error_t *error)
{
    zip_int32_t size;
    int local = (flags & ZIP_EF_LOCAL);
    int i;
    unsigned char b[6];
    const unsigned char *p;

    size = local ? LENTRYSIZE : CDENTRYSIZE;

    if (zip_source_seek(src, local ? 26 : 28, SEEK_CUR) < 0) {
        _zip_error_set_from_source(error, src);
	return -1;
    }

    if (_zip_read(src, b, local ? 4 : 6, error) < 0) {
	return -1;
    }

    p = b;
    for (i=0; i<(local ? 2 : 3); i++) {
	size += _zip_get_16(&p);
    }

    return size;
}


/* _zip_dirent_torrent_normalize(de);
   Set values suitable for torrentzip.
*/

void
_zip_dirent_torrent_normalize(struct zip_dirent *de)
{
    static struct tm torrenttime;
    static time_t last_mod = 0;

    if (last_mod == 0) {
#ifdef HAVE_STRUCT_TM_TM_ZONE
	time_t now;
	struct tm *l;
#endif

	torrenttime.tm_sec = 0;
	torrenttime.tm_min = 32;
	torrenttime.tm_hour = 23;
	torrenttime.tm_mday = 24;
	torrenttime.tm_mon = 11;
	torrenttime.tm_year = 96;
	torrenttime.tm_wday = 0;
	torrenttime.tm_yday = 0;
	torrenttime.tm_isdst = 0;

#ifdef HAVE_STRUCT_TM_TM_ZONE
	time(&now);
	l = localtime(&now);
	torrenttime.tm_gmtoff = l->tm_gmtoff;
	torrenttime.tm_zone = l->tm_zone;
#endif

	last_mod = mktime(&torrenttime);
    }
    
    de->version_madeby = 0;
    de->version_needed = 20; /* 2.0 */
    de->bitflags = 2; /* maximum compression */
    de->comp_method = ZIP_CM_DEFLATE;
    de->last_mod = last_mod;

    de->disk_number = 0;
    de->int_attrib = 0;
    de->ext_attrib = 0;

    _zip_ef_free(de->extra_fields);
    de->extra_fields = NULL;
    _zip_string_free(de->comment);
    de->comment = NULL;
}


/* _zip_dirent_write
   Writes zip directory entry.

   If flags & ZIP_EF_LOCAL, it writes a local header instead of a central
   directory entry.  If flags & ZIP_EF_FORCE_ZIP64, a ZIP64 extra field is written, even if not needed.

   Returns 0 if successful, 1 if successful and wrote ZIP64 extra field. On error, error is filled in and -1 is
   returned.
*/

int
_zip_dirent_write(struct zip *za, struct zip_dirent *de, zip_flags_t flags)
{
    zip_uint16_t dostime, dosdate;
    enum zip_encoding_type com_enc, name_enc;
    struct zip_extra_field *ef;
    zip_uint8_t ef_zip64[24], *ef_zip64_p;
    int is_zip64;
    int is_really_zip64;
    zip_uint8_t buf[CDENTRYSIZE], *p;

    ef = NULL;

    is_zip64 = 0;

    p = buf;

    _zip_put_data(&p, (flags & ZIP_FL_LOCAL) ? LOCAL_MAGIC : CENTRAL_MAGIC, 4);

    name_enc = _zip_guess_encoding(de->filename, ZIP_ENCODING_UNKNOWN);
    com_enc = _zip_guess_encoding(de->comment, ZIP_ENCODING_UNKNOWN);

    if ((name_enc == ZIP_ENCODING_UTF8_KNOWN  && com_enc == ZIP_ENCODING_ASCII) ||
	(name_enc == ZIP_ENCODING_ASCII && com_enc == ZIP_ENCODING_UTF8_KNOWN) ||
	(name_enc == ZIP_ENCODING_UTF8_KNOWN  && com_enc == ZIP_ENCODING_UTF8_KNOWN))
	de->bitflags |= ZIP_GPBF_ENCODING_UTF_8;
    else {
	de->bitflags &= (zip_uint16_t)~ZIP_GPBF_ENCODING_UTF_8;
	if (name_enc == ZIP_ENCODING_UTF8_KNOWN) {
	    ef = _zip_ef_utf8(ZIP_EF_UTF_8_NAME, de->filename, &za->error);
	    if (ef == NULL)
		return -1;
	}
	if ((flags & ZIP_FL_LOCAL) == 0 && com_enc == ZIP_ENCODING_UTF8_KNOWN){
	    struct zip_extra_field *ef2 = _zip_ef_utf8(ZIP_EF_UTF_8_COMMENT, de->comment, &za->error);
	    if (ef2 == NULL) {
		_zip_ef_free(ef);
		return -1;
	    }
	    ef2->next = ef;
	    ef = ef2;
	}
    }

    ef_zip64_p = ef_zip64;
    if (flags & ZIP_FL_LOCAL) {
	if ((flags & ZIP_FL_FORCE_ZIP64) || de->comp_size > ZIP_UINT32_MAX || de->uncomp_size > ZIP_UINT32_MAX) {
	    _zip_put_64(&ef_zip64_p, de->uncomp_size);
	    _zip_put_64(&ef_zip64_p, de->comp_size);
	}
    }
    else {
	if ((flags & ZIP_FL_FORCE_ZIP64) || de->comp_size > ZIP_UINT32_MAX || de->uncomp_size > ZIP_UINT32_MAX || de->offset > ZIP_UINT32_MAX) {
	    if (de->uncomp_size >= ZIP_UINT32_MAX)
		_zip_put_64(&ef_zip64_p, de->uncomp_size);
	    if (de->comp_size >= ZIP_UINT32_MAX)
		_zip_put_64(&ef_zip64_p, de->comp_size);
	    if (de->offset >= ZIP_UINT32_MAX)
		_zip_put_64(&ef_zip64_p, de->offset);
	}
    }

    if (ef_zip64_p != ef_zip64) {
	struct zip_extra_field *ef64 = _zip_ef_new(ZIP_EF_ZIP64, (zip_uint16_t)(ef_zip64_p-ef_zip64), ef_zip64, ZIP_EF_BOTH);
	ef64->next = ef;
	ef = ef64;
	is_zip64 = 1;
    }

    if ((flags & (ZIP_FL_LOCAL|ZIP_FL_FORCE_ZIP64)) == (ZIP_FL_LOCAL|ZIP_FL_FORCE_ZIP64))
	is_really_zip64 = _zip_dirent_needs_zip64(de, flags);
    else
	is_really_zip64 = is_zip64;
    
    if ((flags & ZIP_FL_LOCAL) == 0)
	_zip_put_16(&p, is_really_zip64 ? 45 : de->version_madeby);
    _zip_put_16(&p, is_really_zip64 ? 45 : de->version_needed);
    _zip_put_16(&p, de->bitflags&0xfff9); /* clear compression method specific flags */
    _zip_put_16(&p, (zip_uint16_t)de->comp_method);

    _zip_u2d_time(de->last_mod, &dostime, &dosdate);
    _zip_put_16(&p, dostime);
    _zip_put_16(&p, dosdate);

    _zip_put_32(&p, de->crc);
    if (de->comp_size < ZIP_UINT32_MAX)
	_zip_put_32(&p, (zip_uint32_t)de->comp_size);
    else
	_zip_put_32(&p, ZIP_UINT32_MAX);
    if (de->uncomp_size < ZIP_UINT32_MAX)
	_zip_put_32(&p, (zip_uint32_t)de->uncomp_size);
    else
	_zip_put_32(&p, ZIP_UINT32_MAX);

    _zip_put_16(&p, _zip_string_length(de->filename));
    _zip_put_16(&p, (zip_uint16_t)(_zip_ef_size(de->extra_fields, flags) + _zip_ef_size(ef, ZIP_EF_BOTH)));
    
    if ((flags & ZIP_FL_LOCAL) == 0) {
	_zip_put_16(&p, _zip_string_length(de->comment));
	_zip_put_16(&p, (zip_uint16_t)de->disk_number);
	_zip_put_16(&p, de->int_attrib);
	_zip_put_32(&p, de->ext_attrib);
	if (de->offset < ZIP_UINT32_MAX)
	    _zip_put_32(&p, (zip_uint32_t)de->offset);
	else
	    _zip_put_32(&p, ZIP_UINT32_MAX);
    }

    if (_zip_write(za, buf, (zip_uint64_t)(p-buf)) < 0) {
	return -1;
    }

    if (de->filename) {
	if (_zip_string_write(za, de->filename) < 0) {
	    return -1;
	}
    }

    if (ef) {
	if (_zip_ef_write(za, ef, ZIP_EF_BOTH) < 0) {
	    return -1;
	}
    }
    if (de->extra_fields) {
	if (_zip_ef_write(za, de->extra_fields, flags) < 0) {
	    return -1;
	}
    }

    if ((flags & ZIP_FL_LOCAL) == 0) {
	if (de->comment) {
	    if (_zip_string_write(za, de->comment) < 0) {
		return -1;
	    }
	}
    }

    _zip_ef_free(ef);

    return is_zip64;
}


static time_t
_zip_d2u_time(zip_uint16_t dtime, zip_uint16_t ddate)
{
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    
    /* let mktime decide if DST is in effect */
    tm.tm_isdst = -1;
    
    tm.tm_year = ((ddate>>9)&127) + 1980 - 1900;
    tm.tm_mon = ((ddate>>5)&15) - 1;
    tm.tm_mday = ddate&31;

    tm.tm_hour = (dtime>>11)&31;
    tm.tm_min = (dtime>>5)&63;
    tm.tm_sec = (dtime<<1)&62;

    return mktime(&tm);
}


static struct zip_extra_field *
_zip_ef_utf8(zip_uint16_t id, struct zip_string *str, struct zip_error *error)
{
    const zip_uint8_t *raw;
    zip_uint8_t *data, *p;
    zip_uint32_t len;
    struct zip_extra_field *ef;

    raw = _zip_string_get(str, &len, ZIP_FL_ENC_RAW, NULL);

    if (len+5 > ZIP_UINT16_MAX) {
        /* TODO: error */
    }
    
    if ((data=(zip_uint8_t *)malloc(len+5)) == NULL) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    p = data;
    *(p++) = 1;
    _zip_put_32(&p, _zip_string_crc32(str));
    memcpy(p, raw, len);
    p += len;

    ef = _zip_ef_new(id, (zip_uint16_t)(p-data), data, ZIP_EF_BOTH);
    free(data);
    return ef;
}


struct zip_dirent *
_zip_get_dirent(struct zip *za, zip_uint64_t idx, zip_flags_t flags, struct zip_error *error)
{
    if (error == NULL)
	error = &za->error;

    if (idx >= za->nentry) {
	zip_error_set(error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    if ((flags & ZIP_FL_UNCHANGED) || za->entry[idx].changes == NULL) {
	if (za->entry[idx].orig == NULL) {
	    zip_error_set(error, ZIP_ER_INVAL, 0);
	    return NULL;
	}
	if (za->entry[idx].deleted && (flags & ZIP_FL_UNCHANGED) == 0) {
	    zip_error_set(error, ZIP_ER_DELETED, 0);
	    return NULL;
	}
	return za->entry[idx].orig;
    }
    else
	return za->entry[idx].changes;
}




void
_zip_u2d_time(time_t intime, zip_uint16_t *dtime, zip_uint16_t *ddate)
{
    struct tm *tm;

    tm = localtime(&intime);
    *ddate = (zip_uint16_t)(((tm->tm_year+1900-1980)<<9) + ((tm->tm_mon+1)<<5) + tm->tm_mday);
    *dtime = (zip_uint16_t)(((tm->tm_hour)<<11) + ((tm->tm_min)<<5) + ((tm->tm_sec)>>1));

    return;
}
