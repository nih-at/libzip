/*
  $NiH: zip_replace_filep.c,v 1.8 2004/04/14 14:01:27 dillo Exp $

  zip_replace_filep.c -- replace file from FILE*
  Copyright (C) 1999, 2003, 2004 Dieter Baron and Thomas Klausner

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

struct read_file {
    FILE *f;
    off_t off, len;
};

static int read_file(void *state, void *data, size_t len, enum zip_cmd cmd);



int
zip_replace_filep(struct zip *zf, int idx,
		  FILE *file, off_t start, off_t len)
{
    if (idx < 0 || idx >= zf->nentry) {
	_zip_error_set(&zf->error, ZERR_INVAL, 0);
	return -1;
    }
    
    return _zip_replace_filep(zf, idx, NULL, file, start, len);
}



int
_zip_replace_filep(struct zip *zf, int idx, const char *name,
		  FILE *file, off_t start, off_t len)
{
    struct read_file *f;

    if ((f=(struct read_file *)malloc(sizeof(struct read_file))) == NULL) {
	_zip_error_set(&zf->error, ZERR_MEMORY, 0);
	return -1;
    }

    f->f = file;
    f->off = start;
    f->len = (len ? len : -1);
    
    return _zip_replace(zf, idx, name, read_file, f, 0);
}



static int
read_file(void *state, void *data, size_t len, enum zip_cmd cmd)
{
    struct read_file *z;
    char *buf;
    int i, n;

    z = (struct read_file *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	if (fseeko(z->f, z->off, SEEK_SET) < 0) {
	    /* XXX: zip_err = ZERR_SEEK; */
	    return -1;
	}
	return 0;
	
    case ZIP_CMD_READ:
	if (z->len != -1)
	    n = len > z->len ? z->len : len;
	else
	    n = len;
	
	if ((i=fread(buf, 1, n, z->f)) < 0) {
	    /* XXX: zip_err = ZERR_READ; */
	    return -1;
	}

	if (z->len != -1)
	    z->len -= i;

	return i;
	
    case ZIP_CMD_CLOSE:
	if (z->f) {
	    fclose(z->f);
	    z->f = NULL;
	}
	free(z);
	return 0;

    default:
	;
    }

    return -1;
}
