/*
  $NiH: zip_replace_zip.c,v 1.17.4.5 2004/04/06 20:30:06 dillo Exp $

  zip_replace_zip.c -- replace file from zip file
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



#include <stdlib.h>
#include "zip.h"
#include "zipint.h"

struct read_zip {
    struct zip *zf;
    int idx;
    long fpos;
    long avail;
    /* XXX: ... */
};

struct read_part {
    struct zip_file *zf;
    struct zip_stat st;
    off_t off, len;
};

static ssize_t read_zip(void *st, void *data, size_t len, enum zip_cmd cmd);
static ssize_t read_part(void *st, void *data, size_t len, enum zip_cmd cmd);



int
zip_replace_zip(struct zip *zf, int idx,
		struct zip *srczf, int srcidx, off_t start, off_t len)
{
    if (srcidx < 0 || srcidx >= srczf->nentry) {
	_zip_error_set(&zf->error, ZERR_INVAL, 0);
	return -1;
    }

    return _zip_replace_zip(zf, idx, NULL, srczf, srcidx, start, len);
}



int
_zip_replace_zip(struct zip *zf, int idx, const char *name,
		 struct zip *srczf, int srcidx, off_t start, off_t len)
{
    int ze, se;
    struct read_zip *z;
    struct read_part *p;

    if (srczf->entry[srcidx].state != ZIP_ST_UNCHANGED
	&& srczf->entry[srcidx].state != ZIP_ST_RENAMED) {
	_zip_error_set(&zf->error, ZERR_CHANGED, 0);
	return NULL;
    }
    
    if ((srczf->cdir->entry[srcidx].comp_method != ZIP_CM_STORE
	 && start == 0 && (len == 0 || len == -1))) {
	if ((z=(struct read_zip *)malloc(sizeof(struct read_zip))) == NULL) {
	    _zip_error_set(&zf->error, ZERR_MEMORY, 0);
	    return -1;
	}
	z->zf = srczf;
	z->idx = srcidx;
	return _zip_replace(zf, idx, name, read_zip, z, ZIP_CH_ISCOMP);
    }
    else {
	if ((p=malloc(sizeof(*p))) == NULL) {
	    _zip_error_set(&zf->error, ZERR_MEMORY, 0);
	    return -1;
	}
	
	ze = srczf->error.zip_err;
	se = srczf->error.sys_err;
	
	if (zip_stat_index(srczf, idx, &p->st) < 0
	    || (p->zf=zip_fopen_index(srczf, idx)) == NULL) {
	    free(p);
	    _zip_error_set(&zf->error,
			   srczf->error.zip_err, srczf->error.sys_err);
	    srczf->error.zip_err = ze;
	    srczf->error.sys_err = se;

	    return -1;
	}
	p->off = start;
	p->len = (len ? len : -1);

	return _zip_replace(zf, idx, name, read_part, p, 0);
    }
}



static ssize_t
read_zip(void *state, void *data, size_t len, enum zip_cmd cmd)
{
    struct read_zip *z;
    int ret;
    
    z = (struct read_zip *)state;

    switch (cmd) {
    case ZIP_CMD_INIT:
	/* XXX: sets zf->error */
	if ((z->fpos=_zip_file_get_offset(z->zf, z->idx)) == 0)
	    return -1;
	z->avail = z->zf->cdir->entry[z->idx].comp_size;
	return 0;
	
    case ZIP_CMD_READ:
	if (fseek(z->zf->zp, z->fpos, SEEK_SET) < 0) {
	    /* XXX: zip_err = ZERR_SEEK; */
	    return -1;
	}
	if ((ret=fread(data, 1, len < z->avail ? len : z->avail,
		       z->zf->zp)) < 0) {
	    /* XXX: zip_err = ZERR_READ; */
	    return -1;
	}
	z->fpos += ret;
	z->avail -= ret;
	return ret;
	
    case ZIP_CMD_CLOSE:
	free(z);
	return 0;

    default:
	;
    }

    return -1;
}



static ssize_t
read_part(void *state, void *data, size_t len, enum zip_cmd cmd)
{
    struct read_part *z;
    char b[8192], *buf;
    int i, n;

    z = (struct read_part *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	for (n=0; n<z->off; n+= i) {
	    i = (z->off-n > 8192 ? 8192 : z->off-n);
	    if ((i=zip_fread(z->zf, b, i)) < 0) {
		zip_fclose(z->zf);
		z->zf = NULL;
		return -1;
	    }
	}
	return 0;
	
    case ZIP_CMD_READ:
	if (z->len != -1)
	    n = len > z->len ? z->len : len;
	else
	    n = len;
	

	if ((i=zip_fread(z->zf, buf, n)) < 0) {
	    /* XXX: copy error from z->zff */
	    return -1;
	}

	if (z->len != -1)
	    z->len -= i;

	return i;
	
    case ZIP_CMD_CLOSE:
	zip_fclose(z->zf);
	free(z);
	return 0;

    case ZIP_CMD_STAT:
	if (len < sizeof(z->st))
	    return -1;

	memcpy(data, &z->st, sizeof(z->st));
	return 0;

    default:
	;
    }

    return -1;
}
