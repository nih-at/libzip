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

    if (fn == NULL)
	return NULL;
    
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
    /* ZIP_CREAT gets ignored if file exists and not ZIP_EXCL,
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
	/* read error */
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

    comlen = buf + buflen - eocd - EOCDLEN;
    if (comlen < 0) {
	/* not enough bytes left for comment */
	return NULL;
    }

    /* check for end-of-central-dir magic */
    if (memcmp(eocd, EOCD_MAGIC, 4) != 0)
	return NULL;

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
    zf->nentry = zf->nentry_alloc = _zip_read2(&cdp);
    zf->cd_size = _zip_read4(&cdp);
    zf->cd_offset = _zip_read4(&cdp);
    zf->comlen = _zip_read2(&cdp);
    zf->entry = NULL;

    if ((zf->comlen != comlen) || (zf->nentry != i)) {
	/* comment size wrong -- too few or too many left after central dir */
	/* or number of cdir-entries on this disk != number of cdir-entries */
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
	    _zip_free(zf);
	    return NULL;
	}
    }

    zf->entry = (struct zip_entry *)malloc(sizeof(struct zip_entry)
					  *zf->nentry);
    if (!zf->entry) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }
    
    for (i=0; i<zf->nentry; i++) {
	zf->entry[i].fn = NULL;
	zf->entry[i].ef = NULL;
	zf->entry[i].fcom = NULL;
	_zip_entry_init(zf, i);
    }
    
    for (i=0; i<zf->nentry; i++) {
	if ((_zip_readcdentry(fp, zf->entry+i, &cdp, eocd-cdp, readp, 0)) < 0) {
	    /* i entries have already been filled, tell _zip_free
	       how many to free */
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
    int size;

    if (localp)
	size = LENTRYSIZE;
    else
	size = CDENTRYSIZE;
    
    if (readp) {
	/* read entry from disk */
	if ((fread(buf, 1, size, fp)<size))
	    return -1;
	left = size;
	cur = buf;
    }
    else {
	cur = *cdpp;
	if (left < size)
	    return -1;
    }

    if (localp) {
	if (memcmp(cur, LOCAL_MAGIC, 4)!=0)
	    return -1;
    }
    else
	if (memcmp(cur, CENTRAL_MAGIC, 4)!=0)
	    return -1;

    cur += 4;

    /* convert buffercontents to zf_entry */
    if (!localp)
	zfe->version_made = _zip_read2(&cur);
    else
	zfe->version_made = 0;
    zfe->version_need = _zip_read2(&cur);
    zfe->bitflags = _zip_read2(&cur);
    zfe->comp_meth = _zip_read2(&cur);
    zfe->lmtime = _zip_read2(&cur);
    zfe->lmdate = _zip_read2(&cur);

    zfe->crc = _zip_read4(&cur);
    zfe->comp_size = _zip_read4(&cur);
    zfe->uncomp_size = _zip_read4(&cur);
    
    zfe->fnlen = _zip_read2(&cur);
    zfe->eflen = _zip_read2(&cur);
    if (!localp) {
	zfe->fcomlen = _zip_read2(&cur);
	zfe->disknrstart = _zip_read2(&cur);
	zfe->intatt = _zip_read2(&cur);

	zfe->extatt = _zip_read4(&cur);
	zfe->local_offset = _zip_read4(&cur);
    }
    else {
	zfe->fcomlen = zfe->disknrstart = zfe->intatt = 0;
	zfe->extatt = zfe->local_offset = 0;
    }
    
    if (left < CDENTRYSIZE+zfe->fnlen+zfe->eflen+zfe->fcomlen) {
	if (readp) {
	    if (zfe->fnlen)
		zfe->fn = _zip_readfpstr(fp, zfe->fnlen, 1);
	    else {
		zfe->fn = strdup("");
		if (!zfe->fn) {
		    zip_err = ZERR_MEMORY;
		    return -1;
		}
	    }
	    if (zfe->eflen)
		zfe->ef = _zip_readfpstr(fp, zfe->eflen, 0);
	    /* XXX: really null-terminate comment? */
	    if (zfe->fcomlen)
		zfe->fcom = _zip_readfpstr(fp, zfe->fcomlen, 1);
	}
	else {
	    /* can't get more bytes if not allowed to read */
	    return -1;
	}
    }
    else {
        if (zfe->fnlen)
	    zfe->fn = _zip_readstr(&cur, zfe->fnlen, 1);
        if (zfe->eflen)
	    zfe->ef = _zip_readstr(&cur, zfe->eflen, 0);
        if (zfe->fcomlen)
	    zfe->fcom = _zip_readstr(&cur, zfe->fcomlen, 1);
    }
    if (!readp)
      *cdpp = cur;

    /* XXX */
    zfe->ch_data_fp = NULL;
    zfe->ch_data_buf = NULL;
    zfe->ch_data_offset = 0;
    zfe->ch_data_len = 0;

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
    char *r;

    r = (char *)malloc(nulp?len+1:len);
    if (!r) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }
    
    memcpy(r, *buf, len);
    *buf += len;

    if (nulp)
	r[len] = 0;

    return r;
}



static char *
_zip_readfpstr(FILE *fp, int len, int nullp)
{
    char *r;

    r = (char *)malloc(nullp?len+1:len);
    if (!r) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    if (fread(r, 1, len, fp)<len) {
	free(r);
	return NULL;
    }

    if (nullp)
	r[len] = 0;
    
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
    struct zip_entry temp;
    unsigned char *buf;

    buf = NULL;

    if (zf->nentry) {
	max = zf->entry[0].local_offset;
	min = zf->entry[0].local_offset;
    }

    for (i=0; i<zf->nentry; i++) {
	if (zf->entry[i].local_offset < min)
	    min = zf->entry[i].local_offset;
	if (min < 0)
	    return -1;

	j = zf->entry[i].local_offset + zf->entry[i].comp_size
	    + zf->entry[i].fnlen + zf->entry[i].eflen
	    + zf->entry[i].fcomlen + LENTRYSIZE;
	if (j > max)
	    max = j;
	if (max > zf->cd_offset)
	    return -1;

	if (fseek(fp, zf->entry[i].local_offset, SEEK_SET) != 0) {
	    zip_err = ZERR_SEEK;
	    return -1;
	}
	_zip_readcdentry(fp, &temp, &buf, 0, 1, 1);
	if (_zip_headercomp(zf->entry+i, 0, &temp, 1) != 0)
	    return -1;
    }

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
    if ((h1->version_need != h2->version_need)
	|| (h1->bitflags != h2->bitflags)
	|| (h1->comp_meth != h2->comp_meth)
	|| (h1->lmtime != h2->lmtime)
	|| (h1->lmdate != h2->lmdate)
	|| (h1->fnlen != h2->fnlen)
	|| (h1->crc != h2->crc)
	|| (h1->comp_size != h2->comp_size)
	|| (h1->uncomp_size != h2->uncomp_size)
	|| (h1->fnlen && memcmp(h1->fn, h2->fn, h1->fnlen)))
	return -1;

    /* if they are different type, nothing more to check */
    if (local1p != local2p)
	return 0;

    if ((h1->version_made != h2->version_made)
	|| (h1->disknrstart != h2->disknrstart)
	|| (h1->intatt != h2->intatt)
	|| (h1->extatt != h2->extatt)
	|| (h1->local_offset != h2->local_offset)
	|| (h1->eflen != h2->eflen)
	|| (h1->eflen && memcmp(h1->fn, h2->fn, h1->fnlen))
	|| (h1->fcomlen != h2->fcomlen)
	|| (h1->fcomlen && memcmp(h1->fcom, h2->fcom, h1->fcomlen))) {
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
