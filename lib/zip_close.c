#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "zip.h"
#include "zipint.h"

static int _zip_entry_copy(struct zip *dest, struct zip *src,
			  int entry_no, char *name, struct zip_meta *meta);
static int _zip_entry_add(struct zip *dest, struct zip_entry *se);
static int _zip_writecdir(struct zip *zfp);
static void _zip_write2(FILE *fp, int i);
static void _zip_write4(FILE *fp, int i);
static void _zip_writestr(FILE *fp, char *str, int len);
static int _zip_writecdentry(FILE *fp, struct zip_entry *zfe, int localp);
static int _zip_create_entry(struct zip *dest, struct zip_entry *src_entry,
			     char *name, struct zip_meta *meta);
static void _zip_u2d_time(time_t time, int *ddate, int *dtime);
static int _zip_fwrite(char *b, int s, int n, FILE *f);



/* zip_close:
   Tries to commit all changes and close the zipfile; if it fails,
   zip_err (and errno) are set and *zf is unchanged, except for
   problems in _zip_free. */ 

int
zip_close(struct zip *zf)
{
    int i, count, tfd;
    char *temp;
    FILE *tfp;
    struct zip *tzf;

    if (zf->changes == 0)
	return _zip_free(zf);

    /* look if there really are any changes */
    count = 0;
    for (i=0; i<zf->nentry; i++) {
	if (zf->entry[i].state != ZIP_ST_UNCHANGED) {
	    count = 1;
	    break;
	}
    }

    /* no changes, all has been unchanged */
    if (count == 0)
	return _zip_free(zf);
    
    temp = (char *)malloc(strlen(zf->zn)+8);
    if (!temp) {
	zip_err = ZERR_MEMORY;
	return -1;
    }

    sprintf(temp, "%s.XXXXXX", zf->zn);

    tfd = mkstemp(temp);
    if (tfd == -1) {
	zip_err = ZERR_TMPOPEN;
	return -1;
    }
    
    if ((tfp=fdopen(tfd, "r+b")) == NULL) {
	zip_err = ZERR_TMPOPEN;
	remove(temp);
	free(temp);
	close(tfd);
	return -1;
    }

    tzf = _zip_new();
    tzf->zp = tfp;
    tzf->zn = temp;
    tzf->nentry = 0;
    tzf->comlen = zf->comlen;
    tzf->cd_size = tzf->cd_offset = 0;
    tzf->com = (unsigned char *)_zip_memdup(zf->com, zf->comlen);
    /*    tzf->entry = (struct zip_entry *)malloc(sizeof(struct
	  zip_entry)*ALLOC_SIZE);
	  if (!tzf->entry) {
	  zip_err = ZERR_MEMORY;
	  return -1;
	  }

	  tzf->nentry_alloc = ALLOC_SIZE;
    */
    tzf->entry = NULL;
    tzf->nentry_alloc = 0;
    
    count = 0;
    if (zf->entry) {
	for (i=0; i<zf->nentry; i++) {
	    switch (zf->entry[i].state) {
	    case ZIP_ST_UNCHANGED:
	    case ZIP_ST_RENAMED:
		if (_zip_local_header_read(zf, i) < 0) {
		    remove(tzf->zn);
		    _zip_free(tzf);
		    return -1;
		}
		if (_zip_entry_copy(tzf, zf, i, NULL, zf->entry[i].ch_meta)) {
		    /* zip_err set by _zip_entry_copy */
		    remove(tzf->zn);
		    _zip_free(tzf);
		    return -1;
		}
		break;
	    case ZIP_ST_DELETED:
		break;
	    case ZIP_ST_REPLACED:
	    case ZIP_ST_ADDED:
		/* XXX: rewrite to use new change data format */
		if (_zip_entry_add(tzf, zf->entry+i)) {
		    /* zip_err set by _zip_entry_add */
		    remove(tzf->zn);
		    _zip_free(tzf);
		    return -1;
		}
		break;
	    default:
		/* can't happen */
		break;
	    }
	}
    }

    _zip_writecdir(tzf);
    
    if (fclose(tzf->zp)==0) {
	if (rename(tzf->zn, zf->zn) != 0) {
	    zip_err = ZERR_RENAME;
	    _zip_free(tzf);
	    return -1;
	}
    }

    free(temp);
    _zip_free(zf);

    return 0;
}



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
	zip_err = ZERR_WRITE;
	return -1;
    }

    if (fseek(src->zp, src->entry[entry_no].meta->local_offset
	      + LENTRYSIZE + src->entry[entry_no].meta->lef_len
	      + src->entry[entry_no].file_fnlen, SEEK_SET) != 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }

    
    remainder = src->entry[entry_no].meta->comp_size;
    len = BUFSIZE;
    while (remainder) {
	if (len > remainder)
	    len = remainder;
	if (fread(buf, 1, len, src->zp)!=len) {
	    zip_err = ZERR_READ;
	    return -1;
	}
	if (fwrite(buf, 1, len, dest->zp)!=len) {
	    zip_err = ZERR_WRITE;
	    return -1;
	}
	remainder -= len;
    }

    return 0;
}



static int
_zip_entry_add(struct zip *zf, struct zip_entry *se)
{
    z_stream zstr;
    char b1[BUFSIZE], b2[BUFSIZE];
    int n, size, crc;
    int flush, end, idx;
    struct zip_meta *meta;

    _zip_create_entry(zf, NULL, se->fn, se->ch_meta);
    --zf->nentry;
    idx = zf->nentry;

    zf->entry[idx].meta->local_offset = ftell(zf->zp);

    if (se->ch_func(se->ch_data, NULL, 0, ZIP_CMD_INIT) != 0)
	return -1;

    if (_zip_writecdentry(zf->zp, zf->entry+idx, 1) != 0) {
	zip_err = ZERR_WRITE;
	return -1;
    }

    if ((meta=zip_new_meta()) < 0)
	return -1;

    if (se->ch_comp == 0) { /* we have to compress */
	zstr.zalloc = Z_NULL;
	zstr.zfree = Z_NULL;
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
	size = 0;

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

	    switch (deflate(&zstr, flush)) {
	    case Z_OK:
		if (zstr.avail_out) {
		    if (_zip_fwrite(b2, 1, BUFSIZE-zstr.avail_out, zf->zp) < 0)
			return -1;
		    zstr.next_out = b2;
		    zstr.avail_out = BUFSIZE;
		}
		break;
		
	    case Z_STREAM_END:
		if (se->ch_func(se->ch_data, meta, 0, ZIP_CMD_META) < 0)
		    return -1;
		meta->crc = crc;
		meta->uncomp_size = size;
		meta->comp_size = zstr.total_out;

		deflateEnd(&zstr);
		end = 1;
		break;

	    case Z_STREAM_ERROR:
	    case Z_BUF_ERROR:
		zip_err = ZERR_ZLIB;
		return -1;
	    }
	}

    }
    else { /* we get compressed data */
	while ((n=se->ch_func(se->ch_data, b1, BUFSIZE, ZIP_CMD_READ)) > 0) {
	    size += n;
	    if (_zip_fwrite(b2, 1, zstr.avail_out, zf->zp) < 0)
		return -1;
	}
	if (n < 0)
	    return -1;
	if (se->ch_func(se->ch_data, meta, 0, ZIP_CMD_META) < 0)
	    return -1;
	meta->comp_size = size;
    }

    se->ch_func(se->ch_data, NULL, 0, ZIP_CMD_CLOSE);

    if (fseek(zf->zp, zf->entry[idx].meta->local_offset, SEEK_SET) < 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }

    meta->ef_len = meta->lef_len = meta->fc_len = -1;
    _zip_merge_meta(zf->entry[idx].meta, meta);
    
    if (_zip_writecdentry(zf->zp, zf->entry+idx, 1) != 0) {
	zip_err = ZERR_WRITE;
	return -1;
    }

    zip_free_meta(meta);

    zf->nentry++;
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
_zip_writecdir(struct zip *zfp)
{
    int i;
    long cd_offset, cd_size;

    cd_offset = ftell(zfp->zp);

    for (i=0; i<zfp->nentry; i++) {
	if (_zip_writecdentry(zfp->zp, zfp->entry+i, 0) != 0) {
	    zip_err = ZERR_WRITE;
	    return -1;
	}
    }

    cd_size = ftell(zfp->zp) - cd_offset;
    
    clearerr(zfp->zp);
    fprintf(zfp->zp, EOCD_MAGIC);
    fprintf(zfp->zp, "%c%c%c%c", 0, 0, 0, 0);
    _zip_write2(zfp->zp, zfp->nentry);
    _zip_write2(zfp->zp, zfp->nentry);
    _zip_write4(zfp->zp, cd_size);
    _zip_write4(zfp->zp, cd_offset);
    _zip_write2(zfp->zp, zfp->comlen);
    _zip_writestr(zfp->zp, zfp->com, zfp->comlen);

    return 0;
}



static void
_zip_write2(FILE *fp, int i)
{
    putc(i&0xff, fp);
    putc((i>>8)&0xff, fp);

    return;
}



static void
_zip_write4(FILE *fp, int i)
{
    putc(i&0xff, fp);
    putc((i>>8)&0xff, fp);
    putc((i>>16)&0xff, fp);
    putc((i>>24)&0xff, fp);
    
    return;
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
   if localp, writes local header for zfe to zf->zp,
   else write central directory entry for zfe to zf->zp.
   if after writing ferror(fp), return -1, else return 0.*/
   
static int
_zip_writecdentry(FILE *fp, struct zip_entry *zfe, int localp)
{
    int ltime, ldate;

    fprintf(fp, "%s", localp?LOCAL_MAGIC:CENTRAL_MAGIC);
    
    if (!localp)
	_zip_write2(fp, zfe->meta->version_made);
    _zip_write2(fp, zfe->meta->version_need);
    _zip_write2(fp, zfe->meta->bitflags);
    _zip_write2(fp, zfe->meta->comp_method);
    _zip_u2d_time(zfe->meta->last_mod, &ldate, &ltime);
    _zip_write2(fp, ltime);
    _zip_write2(fp, ldate);

    _zip_write4(fp, zfe->meta->crc);
    _zip_write4(fp, zfe->meta->comp_size);
    _zip_write4(fp, zfe->meta->uncomp_size);

    _zip_write2(fp, strlen(zfe->fn));
    if (localp)
	_zip_write2(fp, zfe->meta->lef_len);
    else {
	_zip_write2(fp, zfe->meta->lef_len);
	_zip_write2(fp, zfe->meta->fc_len);
	_zip_write2(fp, zfe->meta->disknrstart);
	_zip_write2(fp, zfe->meta->int_attr);
	_zip_write4(fp, zfe->meta->ext_attr);
	_zip_write4(fp, zfe->meta->local_offset);
    }
    
    _zip_writestr(fp, zfe->fn, strlen(zfe->fn));
    if (localp) {
	if (zfe->meta->lef_len)
	    _zip_writestr(fp, zfe->meta->lef, zfe->meta->lef_len);
    }
    else {
	if (zfe->meta->ef_len)
	    _zip_writestr(fp, zfe->meta->ef, zfe->meta->ef_len);
	if (zfe->meta->fc_len)
	    _zip_writestr(fp, zfe->meta->fc, zfe->meta->fc_len);
    }

    if (ferror(fp))
	return -1;
    
    return 0;
}



static int
_zip_create_entry(struct zip *dest, struct zip_entry *se,
		  char *name, struct zip_meta *meta)
{
    struct zip_entry *de;

    if (!dest)
	return -1;
    
    de = _zip_new_entry(dest);

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
	/* copy values from original zf_entry */
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
	return -1;
    }

    return _zip_merge_meta(de->meta, meta);
}



static void
_zip_u2d_time(time_t time, int *ddate, int *dtime)
{
    struct tm *tm;

    tm = localtime(&time);
    *dtime = ((tm->tm_year+1900-1980)<<9)+ ((tm->tm_mon+1)<<5)
	+ tm->tm_mday;
    *ddate = ((tm->tm_hour)<<11)+ ((tm->tm_min)<<5)
	+ ((tm->tm_sec)>>1);

    return;
}



int
_zip_local_header_read(struct zip *zf, int idx)
{
    struct zip_entry *ze;
    
    if (zf->entry[idx].meta->lef_len != (unsigned short)-1)
	return 0;

    if ((ze=_zip_new_entry(NULL)) == NULL)
	return -1;

    if (fseek(zf->zp, zf->entry[idx].meta->local_offset, SEEK_SET) < 0) {
	zip_err = ZERR_SEEK;
	return -1;
    }

    if (_zip_readcdentry(zf->zp, ze, NULL, 0, 1, 1) < 0)
	return -1;

    zf->entry[idx].meta->lef_len = ze->meta->lef_len;
    zf->entry[idx].meta->lef = ze->meta->lef;
    ze->meta->lef = NULL;
    _zip_free_entry(ze);

    return 0;
}
