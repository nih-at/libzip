/*
  zip_open.c -- open zip file
  Copyright (C) 1999 Dieter Baron and Thomas Klaunser

  This file is part of libzip, a library to manipulate ZIP files.
  The authors can be contacted at <nih@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

static struct zip *_zip_readcdir(FILE *fp, unsigned char *buf,
			   unsigned char *eocd, int buflen);
static int _zip_read2(unsigned char **a);
static int _zip_read4(unsigned char **a);
static char *_zip_readstr(unsigned char **buf, int len, int nulp);
static char *_zip_readfpstr(FILE *fp, int len, int nulp);
static int _zip_checkcons(FILE *fp, struct zip *zf);
static int _zip_headercomp(struct zip_entry *h1, int local1p,
			   struct zip_entry *h2, int local2p);
static unsigned char *_zip_memmem(const unsigned char *big, int biglen,
				  const unsigned char *little, int littlelen);
static time_t _zip_d2u_time(int dtime, int ddate);



/* zip_open:
   Tries to open the file 'fn' as a zipfile. If flags & ZIP_CHECKCONS,
   also does some consistency checks (comparing local headers to
   central directory entries). If flags & ZIP_CREATE, make a new file
   (if flags & ZIP_EXCL, error if already existing).  Returns a
   zipfile struct, or NULL if unsuccessful, setting zip_err. */

struct zip *
zip_open(char *fn, int flags)
{
    FILE *fp;
    unsigned char *buf, *match;
    int a, i, buflen, best;
    struct zip *cdir, *cdirnew;
    long len;
    struct stat st;

    if (fn == NULL) {
	zip_err = ZERR_INVAL;
	return NULL;
    }
    
    if (stat(fn, &st) != 0) {
	if (flags & ZIP_CREATE) {
	    cdir = _zip_new();
	    cdir->zn = strdup(fn);
	    if (!cdir->zn) {
		zip_err = ZERR_MEMORY;
		return NULL;
	    }
	    return cdir;
	} else {
	    zip_err = ZERR_NOENT;
	    return NULL;
	}
    } else if ((flags & ZIP_EXCL)) {
	zip_err = ZERR_EXISTS;
	return NULL;
    }
    /* ZIP_CREATE gets ignored if file exists and not ZIP_EXCL,
       just like open() */
    
    if ((fp=fopen(fn, "rb"))==NULL) {
	zip_err = ZERR_OPEN;
	return NULL;
    }
    
    clearerr(fp);
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    i = fseek(fp, -(len < BUFSIZE ? len : BUFSIZE), SEEK_END);
    if (i == -1 && errno != EFBIG) {
	/* seek before start of file on my machine */
	zip_err = ZERR_SEEK;
	fclose(fp);
	return NULL;
    }

    buf = (unsigned char *)malloc(BUFSIZE);
    if (!buf) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    clearerr(fp);
    buflen = fread(buf, 1, BUFSIZE, fp);

    if (ferror(fp)) {
	zip_err = ZERR_READ;
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
	if ((cdirnew=_zip_readcdir(fp, buf, match-1, buflen)) == NULL)
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
	zip_err = ZERR_NOZIP;
	free(buf);
	_zip_free(cdir);
	fclose(fp);
	return NULL;
    }

    free(buf);

    cdir->zn = strdup(fn);
    if (!cdir->zn) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    cdir->zp = fp;
    
    return cdir;
}



/* _zip_readcdir:
   tries to find a valid end-of-central-directory at the beginning of
   buf, and then the corresponding central directory entries.
   Returns a zipfile struct which contains the central directory 
   entries, or NULL if unsuccessful. */

static struct zip *
_zip_readcdir(FILE *fp, unsigned char *buf, unsigned char *eocd, int buflen)
{
    struct zip *zf;
    unsigned char *cdp;
    int i, comlen, readp;
    int entries;

    comlen = buf + buflen - eocd - EOCDLEN;
    if (comlen < 0) {
	/* not enough bytes left for comment */
	zip_err = ZERR_NOZIP;
	return NULL;
    }

    /* check for end-of-central-dir magic */
    if (memcmp(eocd, EOCD_MAGIC, 4) != 0) {
	zip_err = ZERR_NOZIP;
	return NULL;
    }

    if (memcmp(eocd+4, "\0\0\0\0", 4) != 0) {
	zip_err = ZERR_MULTIDISK;
	return NULL;
    }

    if ((zf=_zip_new()) == NULL) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    cdp = eocd + 8;
    /* number of cdir-entries on this disk */
    i = _zip_read2(&cdp);
    /* number of cdir-entries */
    entries = _zip_read2(&cdp);
    zf->cd_size = _zip_read4(&cdp);
    zf->cd_offset = _zip_read4(&cdp);
    zf->comlen = _zip_read2(&cdp);
    zf->entry = NULL;

    if ((zf->comlen != comlen) || (entries != i)) {
	/* comment size wrong -- too few or too many left after central dir */
	/* or number of cdir-entries on this disk != number of cdir-entries */
	zip_err = ZERR_NOZIP;
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
		zip_err = ZERR_SEEK;
	    else
		zip_err = ZERR_NOZIP;
	    _zip_free(zf);
	    return NULL;
	}
    }

    zf->entry = (struct zip_entry *)malloc(sizeof(struct zip_entry)
					   *entries);
    if (!zf->entry) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    zf->nentry_alloc = entries;

    for (i=0; i<entries; i++) {
	if (_zip_new_entry(zf) == NULL) {
	    /* shouldn't happen, since space already has been malloc'd */
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



/* _zip_readcdentry:
   fills the zipfile entry zfe with data from the buffer *cdpp, not reading
   more than 'left' bytes from it; if readp != 0, it also reads more data
   from fp, if necessary. If localp != 0, it reads a local header instead
   of a central directory entry. Returns 0 if successful, -1 if not,
   advancing *cdpp for each byte read. */

int
_zip_readcdentry(FILE *fp, struct zip_entry *zfe, unsigned char **cdpp, 
		 int left, int readp, int localp)
{
    unsigned char buf[CDENTRYSIZE];
    unsigned char *cur;
    unsigned short dostime, dosdate;
    int size;

    if (localp)
	size = LENTRYSIZE;
    else
	size = CDENTRYSIZE;
    
    if (readp) {
	/* read entry from disk */
	if ((fread(buf, 1, size, fp)<size)) {
	    zip_err = ZERR_READ;
	    return -1;
	}
	left = size;
	cur = buf;
    }
    else {
	cur = *cdpp;
	if (left < size) {
	    zip_err = ZERR_NOZIP;
	    return -1;
	}
    }

    if (localp) {
	if (memcmp(cur, LOCAL_MAGIC, 4)!=0) {
	    zip_err = ZERR_NOZIP;
	    return -1;
	}
    }
    else
	if (memcmp(cur, CENTRAL_MAGIC, 4)!=0) {
	    zip_err = ZERR_NOZIP;
	    return -1;
	}

    cur += 4;

    /* convert buffercontents to zf_entry */
    if (!localp)
	zfe->meta->version_made = _zip_read2(&cur);
    else
	zfe->meta->version_made = 0;
    zfe->meta->version_need = _zip_read2(&cur);
    zfe->meta->bitflags = _zip_read2(&cur);
    zfe->meta->comp_method = _zip_read2(&cur);
    /* convert to time_t */
    dostime = _zip_read2(&cur);
    dosdate = _zip_read2(&cur);

    zfe->meta->last_mod = _zip_d2u_time(dostime, dosdate);
    
    zfe->meta->crc = _zip_read4(&cur);
    zfe->meta->comp_size = _zip_read4(&cur);
    zfe->meta->uncomp_size = _zip_read4(&cur);
    
    zfe->file_fnlen = _zip_read2(&cur);
    if (localp) {
	zfe->meta->ef_len = 0;
	zfe->meta->lef_len = _zip_read2(&cur);
	zfe->meta->fc_len = zfe->meta->disknrstart = zfe->meta->int_attr = 0;
	zfe->meta->ext_attr = zfe->meta->local_offset = 0;
    } else {
	zfe->meta->ef_len = _zip_read2(&cur);
	zfe->meta->lef_len = 0;
	zfe->meta->fc_len = _zip_read2(&cur);
	zfe->meta->disknrstart = _zip_read2(&cur);
	zfe->meta->int_attr = _zip_read2(&cur);

	zfe->meta->ext_attr = _zip_read4(&cur);
	zfe->meta->local_offset = _zip_read4(&cur);
    }

    if (left < CDENTRYSIZE+zfe->file_fnlen+zfe->meta->ef_len
	+zfe->meta->lef_len+zfe->meta->fc_len) {
	if (readp) {
	    if (zfe->file_fnlen)
		zfe->fn = _zip_readfpstr(fp, zfe->file_fnlen, 1);
	    else
		zfe->fn = strdup("");

	    if (!zfe->fn) {
		zip_err = ZERR_MEMORY;
		return -1;
	    }

	    /* only set for local headers */
	    if (zfe->meta->lef_len) {
		zfe->meta->lef = _zip_readfpstr(fp, zfe->meta->lef_len, 0);
		if (!zfe->meta->lef)
		    return -1;
	    }

	    /* only set for central directory entries */
	    if (zfe->meta->ef_len) {
		zfe->meta->ef = _zip_readfpstr(fp, zfe->meta->ef_len, 0);
		if (!zfe->meta->ef)
		    return -1;
	    }

	    /* only set for central directory entries */
	    if (zfe->meta->fc_len) {
		zfe->meta->fc = _zip_readfpstr(fp, zfe->meta->fc_len, 0);
		if (!zfe->meta->fc)
		    return -1;
	    }
	}
	else {
	    /* can't get more bytes if not allowed to read */
	    zip_err = ZERR_NOZIP;
	    return -1;
	}
    }
    else {
        if (zfe->file_fnlen) {
	    zfe->fn = _zip_readstr(&cur, zfe->file_fnlen, 1);
	    if (!zfe->fn)
		return -1;
	}

	/* only set for local headers */
	if (zfe->meta->lef_len) {
	    zfe->meta->lef = _zip_readstr(&cur, zfe->meta->lef_len, 0);
	    if (!zfe->meta->lef)
		return -1;
	}

	/* only set for central directory entries */
	if (zfe->meta->ef_len) {
	    zfe->meta->ef = _zip_readstr(&cur, zfe->meta->ef_len, 0);
	    if (!zfe->meta->ef)
		return -1;
	}

	/* only set for central directory entries */
        if (zfe->meta->fc_len) {
	    zfe->meta->fc = _zip_readstr(&cur, zfe->meta->fc_len, 0);
	    if (!zfe->meta->fc)
		return -1;
	}
    }
    if (!readp)
      *cdpp = cur;

    return 0;
}



static int
_zip_read2(unsigned char **a)
{
    int ret;

    ret = (*a)[0]+((*a)[1]<<8);
    *a += 2;

    return ret;
}



static int
_zip_read4(unsigned char **a)
{
    int ret;

    ret = ((((((*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;

    return ret;
}



static char *
_zip_readstr(unsigned char **buf, int len, int nulp)
{
    char *r, *o;

    r = (char *)malloc(nulp?len+1:len);
    if (!r) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }
    
    memcpy(r, *buf, len);
    *buf += len;

    if (nulp) {
	/* elephant */
	r[len] = 0;
	o = r-1;
	while (((o=memchr(o+1, 0, r+len-(o+1))) < r+len) && o)
	       *o = ' ';
    }

    return r;
}



static char *
_zip_readfpstr(FILE *fp, int len, int nulp)
{
    char *r, *o;

    r = (char *)malloc(nulp?len+1:len);
    if (!r) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    if (fread(r, 1, len, fp)<len) {
	free(r);
	return NULL;
    }

    if (nulp) {
	/* elephant */
	r[len] = 0;
	o = r-1;
	while (((o=memchr(o+1, 0, r+len-(o+1))) < r+len) && o)
	       *o = ' ';
    }
    
    return r;
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
	    zip_err = ZERR_NOZIP;
	    _zip_free_entry(temp);
	    return -1;
	}
	
	j = zf->entry[i].meta->local_offset + zf->entry[i].meta->comp_size
	    + zf->entry[i].file_fnlen + zf->entry[i].meta->ef_len
	    + zf->entry[i].meta->fc_len + LENTRYSIZE;
	if (j > max)
	    max = j;
	if (max > zf->cd_offset) {
	    zip_err = ZERR_NOZIP;
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (fseek(fp, zf->entry[i].meta->local_offset, SEEK_SET) != 0) {
	    zip_err = ZERR_SEEK;
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (_zip_readcdentry(fp, temp, &buf, 0, 1, 1) == -1) {
	    _zip_free_entry(temp);
	    return -1;
	}
	
	if (_zip_headercomp(zf->entry+i, 0, temp, 1) != 0) {
	    zip_err = ZERR_NOZIP;
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
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    memcpy(ret, mem, len);

    return ret;
}



static time_t
_zip_d2u_time(int dtime, int ddate)
{
    struct tm *tm;
    time_t now;

    now = time(NULL);
    tm = localtime(&now);
    
    tm->tm_year = ((ddate>>9)&127) + 1980 - 1900;
    tm->tm_mon = ((ddate>>5)&15) - 1;
    tm->tm_mday = ddate&31;

    tm->tm_hour = (dtime>>11)&31;
    tm->tm_min = (dtime>>5)&63;
    tm->tm_sec = (dtime<<1)&62;

    return mktime(tm);
}
