/*
  $NiH: zip_open.c,v 1.19.4.1 2004/03/20 09:54:07 dillo Exp $

  zip_open.c -- open zip archive
  Copyright (C) 1999, 2003 Dieter Baron and Thomas Klausner

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

static void set_error(int *errp, int err);
static struct zip *_zip_readcdir(FILE *fp, unsigned char *buf,
			   unsigned char *eocd, int buflen, int *errp);
static int _zip_checkcons(FILE *fp, struct zip *zf);
static int _zip_headercomp(struct zip_entry *h1, int local1p,
			   struct zip_entry *h2, int local2p);
static unsigned char *_zip_memmem(const unsigned char *big, int biglen,
				  const unsigned char *little, int littlelen);



/* zip_open:
   Tries to open the file 'fn' as a zipfile. If flags & ZIP_CHECKCONS,
   also does some consistency checks (comparing local headers to
   central directory entries). If flags & ZIP_CREATE, make a new file
   (if flags & ZIP_EXCL, error if already existing).  Returns a
   zipfile struct, or NULL if unsuccessful, setting zip_err. */

struct zip *
zip_open(const char *fn, int flags, int *errp)
{
    FILE *fp;
    unsigned char *buf, *match;
    int a, i, buflen, best;
    struct zip *cdir, *cdirnew;
    long len;
    struct stat st;

    if (fn == NULL) {
	set_error(errp, ZERR_INVAL);
	return NULL;
    }
    
    if (stat(fn, &st) != 0) {
	if (flags & ZIP_CREATE) {
	    if ((cdir=_zip_new(errp)) == NULL)
		return NULL;
	    
	    cdir->zn = strdup(fn);
	    if (!cdir->zn) {
		_zip_free(cdir);
		set_error(errp, ZERR_MEMORY);
		return NULL;
	    }
	    return cdir;
	} else {
	    set_error(errp, ZERR_NOENT);
	    return NULL;
	}
    } else if ((flags & ZIP_EXCL)) {
	set_error(errp, ZERR_EXISTS);
	return NULL;
    }
    /* ZIP_CREATE gets ignored if file exists and not ZIP_EXCL,
       just like open() */
    
    if ((fp=fopen(fn, "rb"))==NULL) {
	set_error(errp, ZERR_OPEN);
	return NULL;
    }
    
    clearerr(fp);
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    i = fseek(fp, -(len < BUFSIZE ? len : BUFSIZE), SEEK_END);
    if (i == -1 && errno != EFBIG) {
	/* seek before start of file on my machine */
	set_error(errp, ZERR_SEEK);
	fclose(fp);
	return NULL;
    }

    /* XXX: why not allocate statically? */
    buf = (unsigned char *)malloc(BUFSIZE);
    if (!buf) {
	set_error(errp, ZERR_MEMORY);
	fclose(fp);
	return NULL;
    }

    clearerr(fp);
    buflen = fread(buf, 1, BUFSIZE, fp);

    if (ferror(fp)) {
	set_error(errp, ZERR_READ);
	free(buf);
	fclose(fp);
	return NULL;
    }
    
    best = -1;
    cdir = NULL;
    match = buf;
    while ((match=_zip_memmem(match, buflen-(match-buf)-18,
			      EOCD_MAGIC, 4))!=NULL) {
	/* found match -- check, if good */
	/* to avoid finding the same match all over again */
	match++;
	if ((cdirnew=_zip_readcdir(fp, buf, match-1, buflen, errp)) == NULL)
	    continue;	    

	if (cdir) {
	    if (best <= 0)
		best = _zip_checkcons(fp, cdir);
	    a = _zip_checkcons(fp, cdirnew);
	    if (best < a) {
		_zip_free(cdir);
		cdir = cdirnew;
		best = a;
	    }
	    else
		_zip_free(cdirnew);
	}
	else {
	    cdir = cdirnew;
	    if (flags & ZIP_CHECKCONS)
		best = _zip_checkcons(fp, cdir);
	    else
		best = 0;
	}
	cdirnew = NULL;
    }

    if (best < 0) {
	/* no consistent eocd found */
	set_error(errp, ZERR_NOZIP);
	free(buf);
	_zip_free(cdir);
	fclose(fp);
	return NULL;
    }

    free(buf);

    cdir->zn = strdup(fn);
    if (!cdir->zn) {
	set_error(errp, ZERR_MEMORY);
	_zip_free(cdir);
	fclose(fp);
	return NULL;
    }

    cdir->zp = fp;
    
    return cdir;
}



static void
set_error(int *errp, int err)
{
    if (errp)
	*errp = err;
}



/* _zip_readcdir:
   tries to find a valid end-of-central-directory at the beginning of
   buf, and then the corresponding central directory entries.
   Returns a struct zip which contains the central directory 
   entries, or NULL if unsuccessful. */

static struct zip *
_zip_readcdir(FILE *fp, unsigned char *buf, unsigned char *eocd, int buflen,
	      int *errp)
{
    struct zip *zf;
    unsigned char *cdp;
    int i, comlen, readp;
    int entries;

    comlen = buf + buflen - eocd - EOCDLEN;
    if (comlen < 0) {
	/* not enough bytes left for comment */
	set_error(errp, ZERR_NOZIP);
	return NULL;
    }

    /* check for end-of-central-dir magic */
    if (memcmp(eocd, EOCD_MAGIC, 4) != 0) {
	set_error(errp, ZERR_NOZIP);
	return NULL;
    }

    if (memcmp(eocd+4, "\0\0\0\0", 4) != 0) {
	set_error(errp, ZERR_MULTIDISK);
	return NULL;
    }

    if ((zf=_zip_new(errp)) == NULL)
	return NULL;

    cdp = eocd + 8;
    /* number of cdir-entries on this disk */
    i = _zip_read2(&cdp);
    /* number of cdir-entries */
    entries = _zip_read2(&cdp);
    zf->cd_size = _zip_read4(&cdp);
    zf->cd_offset = _zip_read4(&cdp);
    zf->comlen = _zip_read2(&cdp);
    zf->entry = NULL;

    /* XXX: some zip files are broken; their internal comment length
       says 0, but they have 1 or 2 comment bytes */
    if (((zf->comlen != comlen) && (zf->comlen != comlen-1) &&
	 (zf->comlen != comlen-2)) || (entries != i)) {
	/* comment size wrong -- too few or too many left after central dir */
	/* or number of cdir-entries on this disk != number of cdir-entries */
	set_error(errp, ZERR_NOZIP);
	_zip_free(zf);
	return NULL;
    }

    zf->com = (unsigned char *)_zip_memdup(eocd+EOCDLEN, zf->comlen);

    cdp = eocd;
    if (zf->cd_size < eocd-buf) {
	/* if buffer already read in, use it */
	readp = 0;
	cdp = eocd - zf->cd_size;
    }
    else {
	/* go to start of cdir and read it entry by entry */
	readp = 1;
	clearerr(fp);
	fseek(fp, -(zf->cd_size+zf->comlen+EOCDLEN), SEEK_END);
	if (ferror(fp) || (ftell(fp) != zf->cd_offset)) {
	    /* seek error or offset of cdir wrong */
	    if (ferror(fp))
		set_error(errp, ZERR_SEEK);
	    else
		set_error(errp, ZERR_NOZIP);
	    _zip_free(zf);
	    return NULL;
	}
    }

    zf->entry = (struct zip_entry *)malloc(sizeof(struct zip_entry)
					   *entries);
    if (!zf->entry) {
	set_error(errp, ZERR_MEMORY);
	_zip_free(zf);
	return NULL;
    }

    zf->nentry_alloc = entries;

    for (i=0; i<entries; i++) {
	if (_zip_new_entry(zf) == NULL) {
	    /* shouldn't happen, since space already has been malloc'd */
	    _zip_error_get(&zf->error, errp, NULL);
	    _zip_free(zf);
	    return NULL;
	}
    }
    
    for (i=0; i<zf->nentry; i++) {
	if ((_zip_readcdentry(fp, zf->entry+i, &cdp, eocd-cdp,
			      readp, 0)) < 0) {
	    _zip_free(zf);
	    return NULL;
	}
    }
    
    return zf;
}



/* _zip_checkcons:
   Checks the consistency of the central directory by comparing central
   directory entries with local headers and checking for plausible
   file and header offsets. Returns -1 if not plausible, else the
   difference between the lowest and the highest fileposition reached */

static int
_zip_checkcons(FILE *fp, struct zip *zf)
{
    int min, max, i, j;
    struct zip_entry *temp;
    unsigned char *buf;

    buf = NULL;
    if (zf->nentry) {
	max = zf->entry[0].meta->local_offset;
	min = zf->entry[0].meta->local_offset;
    }

    if ((temp=_zip_new_entry(NULL))==NULL)
	return -1;
    
    for (i=0; i<zf->nentry; i++) {
	if (zf->entry[i].meta->local_offset < min)
	    min = zf->entry[i].meta->local_offset;
	if (min < 0) {
	    _zip_error_set(&zf->error, ZERR_NOZIP, 0);
	    _zip_free_entry(temp);
	    return -1;
	}
	
	j = zf->entry[i].meta->local_offset + zf->entry[i].meta->comp_size
	    + zf->entry[i].file_fnlen + zf->entry[i].meta->ef_len
	    + zf->entry[i].meta->fc_len + LENTRYSIZE;
	if (j > max)
	    max = j;
	if (max > zf->cd_offset) {
	    _zip_error_set(&zf->error, ZERR_NOZIP, 0);
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (fseek(fp, zf->entry[i].meta->local_offset, SEEK_SET) != 0) {
	    _zip_error_set(&zf->error, ZERR_SEEK, 0);
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (_zip_readcdentry(fp, temp, &buf, 0, 1, 1) == -1) {
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (_zip_headercomp(zf->entry+i, 0, temp, 1) != 0) {
	    _zip_error_set(&zf->error, ZERR_NOZIP, 0);
	    _zip_free_entry(temp);
	    return -1;
	}
    }

    _zip_free_entry(temp);
    return max - min;
}



/* _zip_headercomp:
   compares two headers h1 and h2; if they are local headers, set
   local1p or local2p respectively to 1, else 0. Return 0 if they
   are identical, -1 if not. */

static int
_zip_headercomp(struct zip_entry *h1, int local1p, struct zip_entry *h2,
	   int local2p)
{
    if ((h1->meta->version_need != h2->meta->version_need)
#if 0
	/* some zip-files have different values in local
	   and global headers for the bitflags */
	|| (h1->meta->bitflags != h2->meta->bitflags)
#endif
	|| (h1->meta->comp_method != h2->meta->comp_method)
	|| (h1->meta->last_mod != h2->meta->last_mod)
	|| (h1->meta->crc != h2->meta->crc)
	|| (h1->meta->comp_size != h2->meta->comp_size)
	|| (h1->meta->uncomp_size != h2->meta->uncomp_size)
	|| !h1->fn
	|| !h2->fn
	|| strcmp(h1->fn, h2->fn))
	return -1;

    /* if they are different type, nothing more to check */
    if (local1p != local2p)
	return 0;

    if ((h1->meta->version_made != h2->meta->version_made)
	|| (h1->meta->disknrstart != h2->meta->disknrstart)
	|| (h1->meta->int_attr != h2->meta->int_attr)
	|| (h1->meta->ext_attr != h2->meta->ext_attr)
	|| (h1->meta->local_offset != h2->meta->local_offset)
	|| (h1->meta->ef_len != h2->meta->ef_len)
	|| (h1->meta->ef_len && memcmp(h1->meta->ef, h2->meta->ef,
				       h1->meta->ef_len))
	|| (h1->meta->fc_len != h2->meta->fc_len)
	|| (h1->meta->fc_len && memcmp(h1->meta->fc, h2->meta->fc,
				       h1->meta->fc_len))) {
	return -1;
    }

    return 0;
}



static unsigned char *
_zip_memmem(const unsigned char *big, int biglen, const unsigned char *little, 
       int littlelen)
{
    const unsigned char *p;
    
    if ((biglen < littlelen) || (littlelen == 0))
	return NULL;
    p = big-1;
    while ((p=memchr(p+1, little[0], big-(p+1)+biglen-littlelen+1))!=NULL) {
	if (memcmp(p+1, little+1, littlelen-1)==0)
	    return (unsigned char *)p;
    }

    return NULL;
}



void *
_zip_memdup(const void *mem, int len)
{
    void *ret;

    ret = malloc(len);
    if (!ret) {
	/* XXX: zip_err = ZERR_MEMORY; */
	return NULL;
    }

    memcpy(ret, mem, len);

    return ret;
}
