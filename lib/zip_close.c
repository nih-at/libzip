/*
  $NiH: zip_close.c,v 1.37.4.3 2004/03/23 18:38:08 dillo Exp $

  zip_close.c -- close zip archive and update changes
  Copyright (C) 1999 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "zip.h"
#include "zipint.h"

#if 0
static int _zip_entry_copy(struct zip *dest, struct zip *src,
			   int entry_no, struct zip_meta *meta);
static int _zip_entry_add(struct zip *dest, struct zip_entry *se);
static int _zip_writecdir(struct zip *zfp);
static void _zip_write2(FILE *fp, int i);
static void _zip_write4(FILE *fp, int i);
static void _zip_writestr(FILE *fp, char *str, int len);
static int _zip_writecdentry(FILE *fp, struct zip_entry *zfe, int localp);
static struct zip_entry *_zip_create_entry(struct zip *dest, 
			                   struct zip_entry *src_entry,
			                   char *name, struct zip_meta *meta);
static void _zip_u2d_time(time_t time, int *ddate, int *dtime);
static int _zip_fwrite(char *b, int s, int n, FILE *f);
#endif



int
zip_close(struct zip *za)
{
    int changed, survivors;
    int i, count, tfd, ret, error;
    char *temp;
    FILE *tfp;
    mode_t mask;
    struct zip_cdir *cd;
    struct zip_dirent de;

    changed = survivors = 0;
    for (i=0; i<za->nentry; i++) {
	if (za->entry[i].state != ZIP_ST_UNCHANGED)
	    changed = 1;
	if (za->entry[i].state != ZIP_ST_DELETED)
	    survivors++;
    }

    if (!changed) {
	_zip_free(za);
	return 0;
    }

    /* don't create zip files with no entries */
    if (survivors == 0) {
	ret = 0;
	if (za->zn)
	    ret = remove(za->zn);
	_zip_free(za);
	/* XXX: inconsistent: za freed, returned -1 */
	return ret;
    }	       
	
    if ((cd=malloc(sizeof(*cd))) == NULL) {
	_zip_error_set(&za->error, ZERR_MEMORY, 0);
	return -1;
    }
    cd->nentry = 0;

    if ((cd->entry=malloc(sizeof(*(cd->entry))*survivors)) == NULL) {
	_zip_error_set(&za->error, ZERR_MEMORY, 0);
	free(cd);
	return -1;
    }

    if ((temp=malloc(strlen(za->zn)+8)) == NULL) {
	_zip_error_set(&za->error, ZERR_MEMORY, 0);
	_zip_cdir_free(cd);
	return -1;
    }

    sprintf(temp, "%s.XXXXXX", za->zn);

    if ((tfd=mkstemp(temp)) == -1) {
	_zip_error_set(&za->error, ZERR_TMPOPEN, errno);
	_zip_cdir_free(cd);
	free(temp);
	return -1;
    }
    
    if ((tfp=fdopen(tfd, "r+b")) == NULL) {
	_zip_error_set(&za->error, ZERR_TMPOPEN, errno);
	_zip_cdir_free(cd);
	close(tfd);
	remove(temp);
	free(temp);
	return -1;
    }

    count = 0;
    error = 0;
    for (i=0; i<za->nentry; i++) {
	if (za->entry[i].state == ZIP_ST_DELETED)
	    continue;

	if (i < za->cdir->nentry) {
	    if (fseek(za->zp, za->cdir->entry[i].offset, SEEK_SET) != 0) {
		_zip_error_set(&za->error, ZERR_SEEK, errno);
		error = 1;
		break;
	    }
	    if (_zip_dirent_read(&de, za->zp, NULL, 0, 1, &za->error) != 0) {
		error = 1;
		break;
	    }
	}
	else {
	    de.filename = NULL;
	    de.filename_len = 0;
	    de.extrafield = NULL;
	    de.extrafield_len = 0;
	    de.comment = NULL;
	    de.comment_len = 0;
	}

	if (za->entry[i].ch_filename) {
	    free(de.filename);
	    de.filename = za->entry[i].ch_filename;
	    de.filename_len = strlen(de.filename);
	    za->entry[i].ch_filename = NULL;
	}
	
	if (za->entry[i].state == ZIP_ST_REPLACED
	    || za->entry[i].state == ZIP_ST_ADDED) {
	    de.last_mod = za->entry[i].ch_mtime;
	    /* XXX: set comp method, write de, write data */
	}
	else {
	    /* XXX: write de, copy data */
	}
	    
	/* XXX: merge de into cd */

	_zip_dirent_finalize(&de);
    }
    if (error) {
	/* XXX: error cleanup */
	return -1;
    }

    /* XXX: write cd */

    _zip_cdir_free(cd);

    if (fclose(tfp) != 0) {
	/* XXX: handle fclose(tfp) error */
	remove(temp);
	free(temp);
	return -1;
    }
    
    if (rename(temp, za->zn) != 0) {
	_zip_error_set(&za->error, ZERR_RENAME, errno);
	remove(temp);
	free(temp);
	return -1;
    }
    if (za->zp) {
	fclose(za->zp);
	za->zp = NULL;
    }
    mask = umask(0);
    umask(mask);
    chmod(za->zn, 0666&~mask);

    _zip_free(za);
    
    return 0;
}



#if 0
static int
_zip_entry_copy(struct zip *dest, struct zip *src, int entry_no,
		struct zip_meta *meta)
{
    char buf[BUFSIZE];
    unsigned int len, remainder;
    struct zip_entry *ze;

    ze = _zip_create_entry(dest, src->entry+entry_no, NULL, meta);
    if (!ze)
	return -1;

    if (_zip_writecdentry(dest->zp, ze, 1) != 0) {
	_zip_error_set(&src->error, ZERR_WRITE, errno);
	return -1;
    }

    if (fseek(src->zp, src->entry[entry_no].meta->local_offset
	      + LENTRYSIZE + src->entry[entry_no].meta->lef_len
	      + src->entry[entry_no].file_fnlen, SEEK_SET) != 0) {
	_zip_error_set(&src->error, ZERR_SEEK, errno);
	return -1;
    }

    
    remainder = src->entry[entry_no].meta->comp_size;
    len = BUFSIZE;
    while (remainder) {
	if (len > remainder)
	    len = remainder;
	if (fread(buf, 1, len, src->zp)!=len) {
	    _zip_error_set(&src->error, ZERR_READ, errno);
	    return -1;
	}
	if (fwrite(buf, 1, len, dest->zp)!=len) {
	    _zip_error_set(&src->error, ZERR_WRITE, errno);
	    return -1;
	}
	remainder -= len;
    }

    return 0;
}



static int
_zip_entry_add(struct zip *za, struct zip_entry *se)
{
    z_stream zstr;
    char b1[BUFSIZE], b2[BUFSIZE];
    int n, size, crc, ret;
    int flush, end, idx;
    struct zip_meta *meta;

    if (_zip_create_entry(za, NULL, se->fn, se->ch_meta) == NULL)
	return -1;
    
    --za->nentry;
    idx = za->nentry;

    za->entry[idx].meta->local_offset = ftell(za->zp);

    if (se->ch_func(se->ch_data, NULL, 0, ZIP_CMD_INIT) != 0)
	return -1;

    if (_zip_writecdentry(za->zp, za->entry+idx, 1) != 0) {
	zip_err = ZERR_WRITE;
	return -1;
    }

    if ((meta=zip_new_meta()) < 0)
	return -1;

    size = 0;
    
    if ((se->ch_comp == 0) || (se->ch_meta
			       && (se->ch_meta->comp_method == 0))) {
	/* we have to compress */
	if (se->ch_meta && ((se->ch_meta->comp_method > 0)
			    && (se->ch_meta->comp_method != 8))) {
	    /* unknown compression method */
	    zip_err = ZERR_INVAL;
	    return -1;
	}
	zstr.zalloc = Z_NULL;
	zstr.zaree = Z_NULL;
	zstr.opaque = NULL;
	zstr.avail_in = 0;
	zstr.avail_out = 0;

	/* -15: undocumented feature of zlib to _not_ write a zlib header */
	deflateInit2(&zstr, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 9,
		     Z_DEFAULT_STRATEGY);

	zstr.next_out = b2;
	zstr.avail_out = BUFSIZE;
	zstr.avail_in = 0;
	flush = 0;
	crc = crc32(0, NULL, 0);

	end = 0;
	while (!end) {
	    if (zstr.avail_in == 0 && !flush) {
		if ((n=se->ch_func(se->ch_data, b1, BUFSIZE,
				     ZIP_CMD_READ)) < 0)
		    return -1;
		zstr.next_in = b1;
		zstr.avail_in = n;
		size += n;
		crc = crc32(crc, b1, n);

		if (n == 0)
		    flush = Z_FINISH;
	    }

	    ret = deflate(&zstr, flush);
	    if (ret != Z_OK && ret != Z_STREAM_END) {
		zip_err = ZERR_ZLIB;
		return -1;
	    }
	    
	    if (zstr.avail_out != BUFSIZE) {
		if (_zip_fwrite(b2, 1, BUFSIZE-zstr.avail_out, za->zp) < 0)
		    return -1;
		zstr.next_out = b2;
		zstr.avail_out = BUFSIZE;
	    }

	    if (ret == Z_STREAM_END) {
		if (se->ch_func(se->ch_data, meta, 0, ZIP_CMD_META) < 0)
		    return -1;
		meta->comp_method = meta->version_need = -1;
		meta->crc = crc;
		meta->uncomp_size = size;
		meta->comp_size = zstr.total_out;

		deflateEnd(&zstr);
		end = 1;
	    }
	}

    }
    else { /* we get compressed data */
	while ((n=se->ch_func(se->ch_data, b1, BUFSIZE, ZIP_CMD_READ)) > 0) {
	    size += n;
	    if (_zip_fwrite(b1, 1, n, za->zp) < 0)
		return -1;
	}
	if (n < 0)
	    return -1;
	if (se->ch_func(se->ch_data, meta, 0, ZIP_CMD_META) < 0)
	    return -1;
	meta->comp_size = size;
    }

    se->ch_func(se->ch_data, NULL, 0, ZIP_CMD_CLOSE);
    free(se->ch_data);
    se->ch_func = NULL;
    se->ch_data = NULL;

    if (fseek(za->zp, za->entry[idx].meta->local_offset, SEEK_SET) < 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }

    meta->local_offset = -1;
    _zip_merge_meta_fix(za->entry[idx].meta, meta);
    if (se->ch_meta) {
	se->ch_meta->local_offset =  se->ch_meta->comp_method
	    = se->ch_meta->uncomp_size = se->ch_meta->comp_size
	    = se->ch_meta->crc = -1;
	_zip_merge_meta_fix(za->entry[idx].meta, se->ch_meta);
    }
    zip_free_meta(meta);

    if (_zip_writecdentry(za->zp, za->entry+idx, 1) != 0) {
	zip_err = ZERR_WRITE;
	return -1;
    }

    if (fseek(za->zp, 0, SEEK_END) < 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }
    
    za->nentry++;
    return 0;
}



static int
_zip_fwrite(char *b, int s, int n, FILE *f)
{
    int ret, writ;

    writ = 0;

    while (writ<n) {
	if ((ret=fwrite(b, s, n-writ, f)) < 0) {
	    zip_err = ZERR_WRITE;
	    return ret;
	}
	writ += ret;
    }

    return writ;
}



static int
_zip_writecdir(struct zip *zap)
{
    int i;
    long cd_offset, cd_size;

    cd_offset = ftell(zap->zp);

    for (i=0; i<zap->nentry; i++) {
	if (_zip_writecdentry(zap->zp, zap->entry+i, 0) != 0) {
	    zip_err = ZERR_WRITE;
	    return -1;
	}
    }

    cd_size = ftell(zap->zp) - cd_offset;
    
    clearerr(zap->zp);
    fprintf(zap->zp, EOCD_MAGIC);
    fprintf(zap->zp, "%c%c%c%c", 0, 0, 0, 0);
    _zip_write2(zap->zp, zap->nentry);
    _zip_write2(zap->zp, zap->nentry);
    _zip_write4(zap->zp, cd_size);
    _zip_write4(zap->zp, cd_offset);
    _zip_write2(zap->zp, zap->comlen);
    _zip_writestr(zap->zp, zap->com, zap->comlen);

    return 0;
}



static void
_zip_writestr(FILE *fp, char *str, int len)
{
#if WIZ
    int i;
    for (i=0; i<len; i++)
	putc(str[i], fp);
#else
    fwrite(str, 1, len, fp);
#endif
    
    return;
}



/* _zip_writecdentry:
   if localp, writes local header for zae to za->zp,
   else write central directory entry for zae to za->zp.
   if after writing ferror(fp), return -1, else return 0.*/
   
static int
_zip_writecdentry(FILE *fp, struct zip_entry *zae, int localp)
{
    int ltime, ldate;

    fprintf(fp, "%s", localp?LOCAL_MAGIC:CENTRAL_MAGIC);
    
    if (!localp)
	_zip_write2(fp, zae->meta->version_made);
    _zip_write2(fp, zae->meta->version_need);
    _zip_write2(fp, zae->meta->bitflags);
    _zip_write2(fp, zae->meta->comp_method);
    _zip_u2d_time(zae->meta->last_mod, &ldate, &ltime);
    _zip_write2(fp, ltime);
    _zip_write2(fp, ldate);

    _zip_write4(fp, zae->meta->crc);
    _zip_write4(fp, zae->meta->comp_size);
    _zip_write4(fp, zae->meta->uncomp_size);

    _zip_write2(fp, strlen(zae->fn));
    if (localp)
	_zip_write2(fp, zae->meta->lef_len);
    else {
	_zip_write2(fp, zae->meta->ef_len);
	_zip_write2(fp, zae->meta->fc_len);
	_zip_write2(fp, zae->meta->disknrstart);
	_zip_write2(fp, zae->meta->int_attr);
	_zip_write4(fp, zae->meta->ext_attr);
	_zip_write4(fp, zae->meta->local_offset);
    }
    
    _zip_writestr(fp, zae->fn, strlen(zae->fn));
    if (localp) {
	if (zae->meta->lef_len)
	    _zip_writestr(fp, zae->meta->lef, zae->meta->lef_len);
    }
    else {
	if (zae->meta->ef_len)
	    _zip_writestr(fp, zae->meta->ef, zae->meta->ef_len);
	if (zae->meta->fc_len)
	    _zip_writestr(fp, zae->meta->fc, zae->meta->fc_len);
    }

    if (ferror(fp))
	return -1;
    
    return 0;
}



static struct zip_entry *
_zip_create_entry(struct zip *dest, struct zip_entry *se,
		  char *name, struct zip_meta *meta)
{
    struct zip_entry *de;

    if (!dest)
	return NULL;
    
    if ((de=_zip_new_entry(dest)) == NULL)
	return NULL;

    if (!se) {
	/* set default values for central directory entry */
	de->meta->version_made = 20;
	de->meta->version_need = 20;
	/* maximum compression */
	de->meta->bitflags = 2;
	/* deflate algorithm */
	de->meta->comp_method = 8;
	de->meta->last_mod = time(NULL)+1;
	de->meta->fc_len = 0;
	de->meta->ef_len = 0;
	de->meta->lef_len = 0;
	de->meta->disknrstart = 0;
	/* binary data */
	de->meta->int_attr = 0;
	/* init CRC-32, compressed and uncompressed size
	   XXX: will be updated later */
	de->meta->crc = crc32(0, 0, 0);
	de->meta->comp_size = 0;
	de->meta->uncomp_size = 0;
	de->meta->ext_attr = 0;
	de->meta->ef = NULL;
	de->meta->lef = NULL;
	de->meta->fc = NULL;
    } else {
	/* copy values from original za_entry */
	de->meta->version_made = se->meta->version_made;
	de->meta->version_need = se->meta->version_need;
	de->meta->bitflags = se->meta->bitflags;
	de->meta->comp_method = se->meta->comp_method;
	de->meta->last_mod = se->meta->last_mod;
	de->meta->disknrstart = se->meta->disknrstart;
	de->meta->int_attr = se->meta->int_attr;
	de->meta->crc = se->meta->crc;
	de->meta->comp_size = se->meta->comp_size;
	de->meta->uncomp_size = se->meta->uncomp_size;
	de->meta->ext_attr = se->meta->ext_attr;
	if (se->meta->ef_len != (unsigned short)-1 && se->meta->ef) {
	    de->meta->ef_len = se->meta->ef_len;
	    de->meta->ef = (char *)_zip_memdup(se->meta->ef,
					       se->meta->ef_len);
	}
	else {
	    de->meta->ef_len = 0;
	    de->meta->ef = NULL;
	}
	if (se->meta->lef_len != (unsigned short)-1 && se->meta->lef) {
	    de->meta->lef_len = se->meta->lef_len;
	    de->meta->lef = (char *)_zip_memdup(se->meta->lef,
						se->meta->lef_len);
	}
	else {
	    de->meta->lef_len = 0;
	    de->meta->lef = NULL;
	}
	if (se->meta->fc_len != (unsigned short)-1 && se->meta->fc) {
	    de->meta->fc_len = se->meta->fc_len;
	    de->meta->fc = (char *)_zip_memdup(se->meta->fc,
					       se->meta->fc_len);
	}
	else {
	    de->meta->fc_len = 0;
	    de->meta->fc = NULL;
	}
    }
	
    de->meta->local_offset = ftell(dest->zp);

    if (name)
	de->fn = strdup(name);
    else if (se && se->fn)
	de->fn = strdup(se->fn);
    else {
	de->fn = strdup("-");
    }

    if (!de->fn) {
	dest->nentry--;
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    if (_zip_merge_meta(de->meta, meta) < 0) {
	dest->nentry--;
	return NULL;
    }

    return de;
}



static void
_zip_u2d_time(time_t time, int *ddate, int *dtime)
{
    struct tm *tm;

    tm = localtime(&time);
    *ddate = ((tm->tm_year+1900-1980)<<9)+ ((tm->tm_mon+1)<<5)
	+ tm->tm_mday;
    *dtime = ((tm->tm_hour)<<11)+ ((tm->tm_min)<<5)
	+ ((tm->tm_sec)>>1);

    return;
}



int
_zip_local_header_read(struct zip *za, int idx)
{
    struct zip_entry *ze;
    
    if (za->entry[idx].meta->lef_len != 0)
	return 0;

    if ((ze=_zip_new_entry(NULL)) == NULL) {
	zip_err = ZERR_MEMORY;
	return -1;
    }

    if (fseek(za->zp, za->entry[idx].meta->local_offset, SEEK_SET) < 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }

    if (_zip_readcdentry(za->zp, ze, NULL, 0, 1, 1) < 0)
	return -1;

    za->entry[idx].meta->lef_len = ze->meta->lef_len;
    za->entry[idx].meta->lef = ze->meta->lef;
    ze->meta->lef = NULL;
    _zip_free_entry(ze);

    return 0;
}
#endif
