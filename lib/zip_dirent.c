/*
  zip_dirent.c -- read directory entry (local or central), clean dirent
  Copyright (C) 1999-2012 Dieter Baron and Thomas Klausner

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

static time_t _zip_d2u_time(int, int);
static struct zip_string *_zip_read_string(const unsigned char **, FILE *, int, int, struct zip_error *);



void
_zip_cdir_free(struct zip_cdir *cd)
{
    int i;

    if (!cd)
	return;

    for (i=0; i<cd->nentry; i++)
	_zip_entry_finalize(cd->entry+i);
    free(cd->entry);
    _zip_string_free(cd->comment);
    free(cd);
}



int
_zip_cdir_grow(struct zip_cdir *cd, int nentry, struct zip_error *error)
{
    struct zip_entry *entry;
    int i;

    if (nentry < cd->nentry_alloc) {
	_zip_error_set(error, ZIP_ER_INTERNAL, 0);
	return -1;
    }

    if (nentry == cd->nentry_alloc)
	return 0;

    if ((entry=((struct zip_entry *)
		realloc(cd->entry, sizeof(*(cd->entry))*nentry))) == NULL) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
	return -1;
    }
    
    for (i=cd->nentry_alloc; i<nentry; i++)
	_zip_entry_init(entry+i);

    cd->nentry_alloc = nentry;
    cd->entry = entry;

    return 0;
}



struct zip_cdir *
_zip_cdir_new(int nentry, struct zip_error *error)
{
    struct zip_cdir *cd;
    int i;
    
    if ((cd=(struct zip_cdir *)malloc(sizeof(*cd))) == NULL) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    if (nentry == 0)
	cd->entry = NULL;
    else if ((cd->entry=(struct zip_entry *)malloc(sizeof(*(cd->entry))*nentry)) == NULL) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
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
_zip_cdir_write(struct zip *za, const struct zip_filelist *filelist, int survivors, FILE *fp)
{
    zip_uint64_t offset, size;
    struct zip_string *comment;
    int i;
    int is_zip64;
    int ret;

    offset = ftello(fp);

    is_zip64 = 0;

    for (i=0; i<survivors; i++) {
	struct zip_entry *entry = za->entry+filelist[i].idx;

	if ((ret=_zip_dirent_write(entry->changes ? entry->changes : entry->orig, fp, ZIP_FL_CENTRAL, &za->error)) < 0)
	    return -1;
	if (ret)
	    is_zip64 = 1;
    }

    size = ftello(fp) - offset;

    if (offset > ZIP_UINT32_MAX || survivors > ZIP_UINT16_MAX)
	is_zip64 = 1;

    if (is_zip64) {
	fwrite(EOCD64_MAGIC, 1, 4, fp);
	_zip_write8(EOCD64LEN, fp);
	_zip_write2(45, fp);
	_zip_write2(45, fp);
	_zip_write4(1, fp);
	_zip_write4(1, fp);
	_zip_write8(survivors, fp);
	_zip_write8(survivors, fp);
	_zip_write8(size, fp);
	_zip_write8(offset, fp);

	fwrite(EOCD64LOC_MAGIC, 1, 4, fp);
	_zip_write4(1, fp);
	_zip_write8(offset+size, fp);
	_zip_write4(1, fp);
		    
    }

    /* clearerr(fp); */
    fwrite(EOCD_MAGIC, 1, 4, fp);
    _zip_write4(0, fp);
    _zip_write2(survivors >= ZIP_UINT16_MAX ? ZIP_UINT16_MAX : survivors, fp);
    _zip_write2(survivors >= ZIP_UINT16_MAX ? ZIP_UINT16_MAX : survivors, fp);
    _zip_write4(size >= ZIP_UINT32_MAX ? ZIP_UINT32_MAX : size, fp);
    _zip_write4(offset >= ZIP_UINT32_MAX ? ZIP_UINT32_MAX : offset, fp);

    comment = za->comment_changed ? za->comment_changes : za->comment_orig;

    _zip_write2(comment ? comment->length : 0, fp);
    if (comment)
	fwrite(comment->raw, 1, comment->length, fp);

    if (ferror(fp)) {
	_zip_error_set(&za->error, ZIP_ER_WRITE, errno);
	return -1;
    }

    return size;
}



struct zip_dirent *
_zip_dirent_clone(const struct zip_dirent *sde)
{
    struct zip_dirent *tde;

    if ((tde=malloc(sizeof(*tde))) == NULL)
	return NULL;

    if (sde)
	memcpy(tde, sde, sizeof(*sde));
    else
	_zip_dirent_init(tde);
    
    tde->changed = 0;

    return tde;
}



void
_zip_dirent_finalize(struct zip_dirent *zde)
{
    if (zde->changed & ZIP_DIRENT_FILENAME)
	_zip_string_free(zde->filename);
    if (zde->changed & ZIP_DIRENT_EXTRA_FIELD)
	_zip_ef_free(zde->extra_fields);
    if (zde->changed & ZIP_DIRENT_COMMENT)
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

    de->version_madeby = 0;
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
    de->ext_attrib = 0;
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

    if ((de=malloc(sizeof(*de))) == NULL)
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

   XXX: leftp and file position undefined on error.
*/

int
_zip_dirent_read(struct zip_dirent *zde, FILE *fp,
		 const unsigned char **bufp, zip_uint32_t *leftp, int local,
		 struct zip_error *error)
{
    unsigned char buf[CDENTRYSIZE];
    const unsigned char *cur;
    unsigned short dostime, dosdate;
    zip_uint32_t size;
    zip_uint32_t filename_len, ef_len, comment_len;

    if (local)
	size = LENTRYSIZE;
    else
	size = CDENTRYSIZE;

    if (leftp && (*leftp < size)) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
    }

    if (bufp) {
	/* use data from buffer */
	cur = *bufp;
    }
    else {
	/* read entry from disk */
	if ((fread(buf, 1, size, fp)<size)) {
	    _zip_error_set(error, ZIP_ER_READ, errno);
	    return -1;
	}
	cur = buf;
    }

    if (memcmp(cur, (local ? LOCAL_MAGIC : CENTRAL_MAGIC), 4) != 0) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
    }
    cur += 4;


    /* convert buffercontents to zip_dirent */

    _zip_dirent_init(zde);
    if (!local)
	zde->version_madeby = _zip_read2(&cur);
    else
	zde->version_madeby = 0;
    zde->version_needed = _zip_read2(&cur);
    zde->bitflags = _zip_read2(&cur);
    zde->comp_method = _zip_read2(&cur);
    
    /* convert to time_t */
    dostime = _zip_read2(&cur);
    dosdate = _zip_read2(&cur);
    zde->last_mod = _zip_d2u_time(dostime, dosdate);
    
    zde->crc = _zip_read4(&cur);
    zde->comp_size = _zip_read4(&cur);
    zde->uncomp_size = _zip_read4(&cur);
    
    filename_len = _zip_read2(&cur);
    ef_len = _zip_read2(&cur);
    
    if (local) {
	comment_len = 0;
	zde->disk_number = 0;
	zde->int_attrib = 0;
	zde->ext_attrib = 0;
	zde->offset = 0;
    } else {
	comment_len = _zip_read2(&cur);
	zde->disk_number = _zip_read2(&cur);
	zde->int_attrib = _zip_read2(&cur);
	zde->ext_attrib = _zip_read4(&cur);
	zde->offset = _zip_read4(&cur);
    }

    zde->filename = NULL;
    zde->extra_fields = NULL;
    zde->comment = NULL;

    size += filename_len+ef_len+comment_len;

    if (leftp && (*leftp < size)) {
	_zip_error_set(error, ZIP_ER_NOZIP, 0);
	return -1;
    }

    if (filename_len) {
	zde->filename = _zip_read_string(bufp ? &cur : NULL, fp, filename_len, 1, error);
	if (!zde->filename)
	    return -1;

	if (zde->bitflags & ZIP_GPBF_ENCODING_UTF_8) {
	    if (_zip_guess_encoding(zde->filename, ZIP_ENCODING_UTF8_KNOWN) == ZIP_ENCODING_ERROR) {
		_zip_error_set(error, ZIP_ER_INCONS, 0);
		return -1;
	    }
	}
    }

    if (ef_len) {
	zip_uint8_t *ef = _zip_read_data(bufp ? &cur : NULL, fp, ef_len, 0, error);

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
	zde->comment = _zip_read_string(bufp ? &cur : NULL, fp, comment_len, 0, error);
	if (!zde->comment)
	    return -1;

	if (zde->bitflags & ZIP_GPBF_ENCODING_UTF_8) {
	    if (_zip_guess_encoding(zde->comment, ZIP_ENCODING_UTF8_KNOWN) == ZIP_ENCODING_ERROR) {
		_zip_error_set(error, ZIP_ER_INCONS, 0);
		return -1;
	    }
	}
    }


    /* Zip64 */

    if (zde->uncomp_size == ZIP_UINT32_MAX || zde->comp_size == ZIP_UINT32_MAX || zde->offset == ZIP_UINT32_MAX) {
	zip_uint16_t ef_len, needed_len;
	const zip_uint8_t *ef = _zip_ef_get_by_id(zde->extra_fields, &ef_len, ZIP_EF_ZIP64, 0, local ? ZIP_EF_LOCAL : ZIP_EF_CENTRAL, error);
	/* XXX: if ef_len == 0 && !ZIP64_EOCD: no error, 0xffffffff is valid value */
	if (ef == NULL)
	    return -1;


	if (local)
	    needed_len = 16;
	else
	    needed_len = ((zde->uncomp_size == ZIP_UINT32_MAX) + (zde->comp_size == ZIP_UINT32_MAX) + (zde->offset == ZIP_UINT32_MAX)) * 8
		+ (zde->disk_number == ZIP_UINT16_MAX) * 4;

	if (ef_len != needed_len) {
	    _zip_error_set(error, ZIP_ER_INCONS, 0);
	    return -1;
	}
	
	if (zde->uncomp_size == ZIP_UINT32_MAX)
	    zde->uncomp_size = _zip_read8(&ef);
	else if (local)
	    ef += 8;
	if (zde->comp_size == ZIP_UINT32_MAX)
	    zde->comp_size = _zip_read8(&ef);
	if (!local) {
	    if (zde->offset == ZIP_UINT32_MAX)
		zde->offset = _zip_read8(&ef);
	    if (zde->disk_number == ZIP_UINT16_MAX)
		zde->disk_number = _zip_read4(&ef);
	}

	zde->extra_fields = _zip_ef_delete_by_id(zde->extra_fields, ZIP_EF_ZIP64, ZIP_EXTRA_FIELD_ALL, local ? ZIP_EF_LOCAL : ZIP_EF_CENTRAL);
    }

    if (bufp)
      *bufp = cur;
    if (leftp)
	*leftp -= size;

    return 0;
}



zip_int32_t
_zip_dirent_size(FILE *f, zip_uint16_t flags, struct zip_error *error)
{
    zip_int32_t size;
    int local = (flags & ZIP_EF_LOCAL);
    int i;
    unsigned char b[6];
    const unsigned char *p;

    size = local ? LENTRYSIZE : CDENTRYSIZE;

    if (fseek(f, local ? 26 : 28, SEEK_CUR) < 0) {
	_zip_error_set(error, ZIP_ER_SEEK, errno);
	return -1;
    }

    if (fread(b, (local ? 4 : 6), 1, f) != 1) {
	_zip_error_set(error, ZIP_ER_READ, errno);
	return -1;
    }

    p = b;
    for (i=0; i<(local ? 2 : 3); i++) {
	size += _zip_read2(&p);
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



/* _zip_dirent_write(zde, fp, flags, error):
   Writes zip directory entry zde to file fp.

   If flags & ZIP_EF_LOCAL, it writes a local header instead of a central
   directory entry.  If flags & ZIP_EF_FORCE_ZIP64, a ZIP64 extra field is written, even if not needed.

   Returns 0 if successful, 1 if successful and wrote ZIP64 extra field. On error, error is filled in and -1 is
   returned.
*/

int
_zip_dirent_write(struct zip_dirent *zde, FILE *fp, int flags,
		  struct zip_error *error)
{
    unsigned short dostime, dosdate;
    zip_uint16_t zip64_ef_size;

    zip64_ef_size = 0;

    fwrite((flags & ZIP_FL_LOCAL) ? LOCAL_MAGIC : CENTRAL_MAGIC, 1, 4, fp);


    if ((flags & ZIP_FL_LOCAL) == 0)
	_zip_write2(zde->version_madeby, fp);
    _zip_write2(zde->version_needed, fp);    /* XXX: at least 4.5 if zip64 */
    _zip_write2(zde->bitflags, fp);
    _zip_write2(zde->comp_method, fp);

    _zip_u2d_time(zde->last_mod, &dostime, &dosdate);
    _zip_write2(dostime, fp);
    _zip_write2(dosdate, fp);
    
    _zip_write4(zde->crc, fp);
    if (zde->comp_size < ZIP_UINT32_MAX)
	_zip_write4(zde->comp_size, fp);
    else {
	_zip_write4(ZIP_UINT32_MAX-1, fp);
	zip64_ef_size += 8;
    }
    if (zde->uncomp_size < ZIP_UINT32_MAX)
	_zip_write4(zde->uncomp_size, fp);
    else {
	_zip_write4(ZIP_UINT32_MAX-1, fp);
	zip64_ef_size += 8;
    }

    if (flags & ZIP_FL_LOCAL) {
	if (zip64_ef_size > 0 || (flags & ZIP_FL_FORCE_ZIP64))
	    zip64_ef_size = 16;
    }
    else if (zde->offset >= ZIP_UINT32_MAX)
	zip64_ef_size += 8;

    _zip_write2(_zip_string_length(zde->filename), fp);
    _zip_write2(_zip_ef_size(zde->extra_fields, flags) + (zip64_ef_size ? (zip64_ef_size+4) : 0), fp);
    
    if ((flags & ZIP_FL_LOCAL) == 0) {
	_zip_write2(_zip_string_length(zde->comment), fp);
	_zip_write2(zde->disk_number, fp);
	_zip_write2(zde->int_attrib, fp);
	_zip_write4(zde->ext_attrib, fp);
	if (zde->offset < ZIP_UINT32_MAX)
	    _zip_write4(zde->offset, fp);
	else
	    _zip_write4(ZIP_UINT32_MAX-1, fp);
    }

    if (zde->filename)
	_zip_string_write(zde->filename, fp);

    if (zde->extra_fields)
	_zip_ef_write(zde->extra_fields, flags, fp);

    if (zip64_ef_size > 0) {
	_zip_write2(ZIP_EF_ZIP64, fp);
	_zip_write2(zip64_ef_size, fp);
	if ((flags & ZIP_EF_LOCAL) || zde->uncomp_size >= ZIP_UINT32_MAX)
	    _zip_write8(zde->uncomp_size, fp);
	if ((flags & ZIP_EF_LOCAL) || zde->comp_size >= ZIP_UINT32_MAX)
	    _zip_write8(zde->comp_size, fp);
	if (((flags & ZIP_EF_LOCAL) == 0) && zde->offset >= ZIP_UINT32_MAX)
	    _zip_write8(zde->offset, fp);
    }

    if ((flags & ZIP_FL_LOCAL) == 0) {
	if (zde->comment)
	    _zip_string_write(zde->comment, fp);
    }

    if (ferror(fp)) {
	_zip_error_set(error, ZIP_ER_WRITE, errno);
	return -1;
    }

    return zip64_ef_size > 0;
}



static time_t
_zip_d2u_time(int dtime, int ddate)
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



struct zip_dirent *
_zip_get_dirent(struct zip *za, zip_uint64_t idx, int flags, struct zip_error *error)
{
    if (error == NULL)
	error = &za->error;

    if (idx >= za->nentry) {
	_zip_error_set(error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    if ((flags & ZIP_FL_UNCHANGED) || za->entry[idx].changes == NULL) {
	if (za->entry[idx].orig == NULL) {
	    _zip_error_set(error, ZIP_ER_INVAL, 0);
	    return NULL;
	}
	if (za->entry[idx].deleted && (flags & ZIP_FL_UNCHANGED) == 0) {
	    _zip_error_set(error, ZIP_ER_DELETED, 0);
	    return NULL;
	}
	return za->entry[idx].orig;
    }
    else
	return za->entry[idx].changes;
}



zip_uint16_t
_zip_read2(const unsigned char **a)
{
    zip_uint16_t ret;

    ret = (*a)[0]+((*a)[1]<<8);
    *a += 2;

    return ret;
}



zip_uint32_t
_zip_read4(const unsigned char **a)
{
    zip_uint32_t ret;

    ret = ((((((*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;

    return ret;
}



zip_uint64_t
_zip_read8(const unsigned char **a)
{
    zip_uint64_t x, y;

    x = ((((((*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;
    y = (((((((*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0]);
    *a += 4;

    return x+(y<<32);
}



zip_uint8_t *
_zip_read_data(const unsigned char **buf, FILE *fp, int len, int nulp, struct zip_error *error)
{
    zip_uint8_t *r, *o;

    if (len == 0 && nulp == 0)
	return NULL;

    r = (zip_uint8_t *)malloc(nulp ? len+1 : len);
    if (!r) {
	_zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    if (buf) {
	memcpy(r, *buf, len);
	*buf += len;
    }
    else {
	if (fread(r, 1, len, fp)<len) {
	    free(r);
	    _zip_error_set(error, ZIP_ER_READ, errno);
	    return NULL;
	}
    }

    if (nulp) {
	/* replace any in-string NUL characters with spaces */
	r[len] = 0;
	for (o=r; o<r+len; o++)
	    if (*o == '\0')
		*o = ' ';
    }

    return r;
}



static struct zip_string *
_zip_read_string(const unsigned char **buf, FILE *fp, int len, int nulp, struct zip_error *error)
{
    zip_uint8_t *raw;

    if ((raw=_zip_read_data(buf, fp, len, nulp, error)) == NULL)
	return NULL;

    return _zip_string_new(raw, len, error);
}



void
_zip_write2(zip_uint16_t i, FILE *fp)
{
    putc(i&0xff, fp);
    putc((i>>8)&0xff, fp);

    return;
}



void
_zip_write4(zip_uint32_t i, FILE *fp)
{
    putc(i&0xff, fp);
    putc((i>>8)&0xff, fp);
    putc((i>>16)&0xff, fp);
    putc((i>>24)&0xff, fp);
    
    return;
}



void
_zip_write8(zip_uint64_t i, FILE *fp)
{
    putc(i&0xff, fp);
    putc((i>>8)&0xff, fp);
    putc((i>>16)&0xff, fp);
    putc((i>>24)&0xff, fp);
    putc((i>>32)&0xff, fp);
    putc((i>>40)&0xff, fp);
    putc((i>>48)&0xff, fp);
    putc((i>>56)&0xff, fp);
    
    return;
}



void
_zip_u2d_time(time_t time, unsigned short *dtime, unsigned short *ddate)
{
    struct tm *tm;

    tm = localtime(&time);
    *ddate = ((tm->tm_year+1900-1980)<<9) + ((tm->tm_mon+1)<<5)
	+ tm->tm_mday;
    *dtime = ((tm->tm_hour)<<11) + ((tm->tm_min)<<5)
	+ ((tm->tm_sec)>>1);

    return;
}
