/*
  $NiH: zip_replace_zip.c,v 1.16 2003/10/01 09:51:01 dillo Exp $

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
    struct zip *zf;
    int idx;
    struct zip_file *zff;
    int off, len;
    /* XXX: ... */
};

static int read_zip(void *state, void *data, int len, enum zip_cmd cmd);
static int read_part(void *state, void *data, int len, enum zip_cmd cmd);



int
zip_replace_zip(struct zip *zf, int idx, const char *name,
		struct zip_meta *meta,
		struct zip *srczf, int srcidx, int start, int len)
{
    struct read_zip *z;
    struct read_part *p;

    if (srcidx < -1 || srcidx >= srczf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if ((srczf->entry[srcidx].meta->comp_method != 0
	 && start == 0 && (len == 0 || len == -1))) {
	if ((z=(struct read_zip *)malloc(sizeof(struct read_zip))) == NULL) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
	z->zf = srczf;
	z->idx = srcidx;
	return zip_replace(zf, idx, (name ? name : srczf->entry[srcidx].fn),
			   meta, read_zip, z, 1);
    }
    else {
	if ((p=(struct read_part *)malloc(sizeof(struct read_part))) == NULL) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
	p->zf = srczf;
	p->idx = srcidx;
	p->off = start;
	p->len = (len ? len : -1);
	p->zff = NULL;
	return zip_replace(zf, idx, (name ? name : srczf->entry[srcidx].fn),
			   meta, read_part, p, 0);
    }
}



static int
read_zip(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_zip *z;
    int ret;
    
    z = (struct read_zip *)state;

    switch (cmd) {
    case ZIP_CMD_INIT:
	_zip_local_header_read(z->zf, z->idx);
	z->fpos = z->zf->entry[z->idx].meta->local_offset + LENTRYSIZE
	    + z->zf->entry[z->idx].meta->lef_len
	    + z->zf->entry[z->idx].meta->fc_len
	    + z->zf->entry[z->idx].file_fnlen;
	z->avail = z->zf->entry[z->idx].meta->comp_size;
	return 0;
	
    case ZIP_CMD_READ:
	if (fseek(z->zf->zp, z->fpos, SEEK_SET) < 0) {
	    zip_err = ZERR_SEEK;
	    return -1;
	}
	if ((ret=fread(data, 1, len < z->avail ? len : z->avail,
		       z->zf->zp)) < 0) {
	    zip_err = ZERR_READ;
	    return -1;
	}
	z->fpos += ret;
	z->avail -= ret;
	return ret;
	
    case ZIP_CMD_META:
	return _zip_merge_meta_fix((struct zip_meta *)data,
				   z->zf->entry[z->idx].meta);
    case ZIP_CMD_CLOSE:
	return 0;
    }

    return -1;
}



static int
read_part(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_part *z;
    char b[8192], *buf;
    int i, n;

    z = (struct read_part *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	if ((z->zff=zip_fopen_index(z->zf, z->idx) ) == NULL)
	    return -1;

	for (n=0; n<z->off; n+= i) {
	    i = (z->off-n > 8192 ? 8192 : z->off-n);
	    if ((i=zip_fread(z->zff, b, i)) < 0) {
		zip_fclose(z->zff);
		z->zff = NULL;
		return -1;
	    }
	}
	return 0;
	
    case ZIP_CMD_READ:
	if (z->len != -1)
	    n = len > z->len ? z->len : len;
	else
	    n = len;
	
	if ((i=zip_fread(z->zff, buf, n)) < 0)
	    return -1;

	if (z->len != -1)
	    z->len -= i;

	return i;
	
    case ZIP_CMD_META:
	return 0;

    case ZIP_CMD_CLOSE:
	if (z->zff) {
	    zip_fclose(z->zff);
	    z->zff = NULL;
	}
	return 0;
    }

    return -1;
}
