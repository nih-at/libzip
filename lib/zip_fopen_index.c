/*
  zip_fopen_index.c -- open file in zip file for reading by index
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

#include "zip.h"
#include "zipint.h"

static struct zip_file *_zip_file_new(struct zip *zf);



struct zip_file *
zip_fopen_index(struct zip *zf, int fileno)
{
    unsigned char buf[4];
    int len;
    struct zip_file *zff;

    if ((fileno < 0) || (fileno >= zf->nentry)) {
	zip_err = ZERR_INVAL;
	return NULL;
    }

#if 0
    if (zf->entry[fileno].state != ZIP_ST_UNCHANGED
	&& zf->entry[fileno].state != ZIP_ST_RENAMED) {
	zip_err = ZERR_CHANGED;
	return NULL;
    }
#endif

    if ((zf->entry[fileno].meta->comp_method != 0)
	&& (zf->entry[fileno].meta->comp_method != 8)) {
	zip_err = ZERR_COMPNOTSUPP;
	return NULL;
    }

    zff = _zip_file_new(zf);

    zff->name = zf->entry[fileno].fn;
    zff->method = zf->entry[fileno].meta->comp_method;
    zff->bytes_left = zf->entry[fileno].meta->uncomp_size;
    zff->cbytes_left = zf->entry[fileno].meta->comp_size;
    zff->crc_orig = zf->entry[fileno].meta->crc;

    /* go to start of actual data */
    if (fseek(zf->zp, zf->entry[fileno].meta->local_offset+LENTRYSIZE-4,
	      SEEK_SET) < 0) {
	zip_err = ZERR_SEEK;
	zip_fclose(zff);
	return NULL;
    }
    len = fread(buf, 1, 4, zf->zp);
    if (len != 4) {
	zip_err = ZERR_READ;
	zip_fclose(zff);
	return NULL;
    }
    
    zff->fpos = zf->entry[fileno].meta->local_offset+LENTRYSIZE;
    zff->fpos += (buf[3]<<8)+buf[2]+(buf[1]<<8)+buf[0];
	
    if ((zf->entry[fileno].meta->comp_method == 0)
	|| (zff->bytes_left == 0))
	return zff;
    
    if ((zff->buffer=(char *)malloc(BUFSIZE)) == NULL) {
	zip_err = ZERR_MEMORY;
	zip_fclose(zff);
	return NULL;
    }

    len = _zip_file_fillbuf (zff->buffer, BUFSIZE, zff);
    if (len <= 0) {
	zip_fclose(zff);
	return NULL;
    }

    if ((zff->zstr = (z_stream *)malloc(sizeof(z_stream))) == NULL) {
	zip_err = ZERR_MEMORY;
	zip_fclose(zff);
	return NULL;
    }
    zff->zstr->zalloc = Z_NULL;
    zff->zstr->zfree = Z_NULL;
    zff->zstr->opaque = NULL;
    zff->zstr->next_in = zff->buffer;
    zff->zstr->avail_in = len;

    /* negative value to tell zlib that there is no header */
    if (inflateInit2(zff->zstr, -MAX_WBITS) != Z_OK) {
	zip_err = ZERR_ZLIB;
	zip_fclose(zff);
	return NULL;
    }
    
    return zff;
}



int
_zip_file_fillbuf(char *buf, int buflen, struct zip_file *zff)
{
    int i, j;

    if (zff->flags != 0)
	return -1;
    if (zff->cbytes_left <= 0 || buflen <= 0)
	return 0;
    
    if (fseek(zff->zf->zp, zff->fpos, SEEK_SET) < 0) {
	zff->flags = ZERR_SEEK;
	return -1;
    }
    if (buflen < zff->cbytes_left)
	i = buflen;
    else
	i = zff->cbytes_left;

    j = fread(buf, 1, i, zff->zf->zp);
    if (j == 0) {
	zff->flags = ZERR_EOF;
	j = -1;
    }
    else if (j < 0)
	zff->flags = ZERR_READ;
    else {
	zff->fpos += j;
	zff->cbytes_left -= j;
    }

    return j;	
}



static struct zip_file *
_zip_file_new(struct zip *zf)
{
    struct zip_file *zff;

    if (zf->nfile >= zf->nfile_alloc-1) {
	zf->nfile_alloc += 10;
	zf->file = (struct zip_file **)realloc(zf->file, zf->nfile_alloc
					       *sizeof(struct zip_file *));
	if (zf->file == NULL) {
	    zip_err = ZERR_MEMORY;
	    return NULL;
	}
    }

    if ((zff=(struct zip_file *)malloc(sizeof(struct zip_file))) == NULL) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }
    
    zf->file[zf->nfile++] = zff;

    zff->zf = zf;
    zff->flags = 0;
    zff->crc = crc32(0L, Z_NULL, 0);
    zff->crc_orig = 0;
    zff->method = -1;
    zff->bytes_left = zff->cbytes_left = 0;
    zff->fpos = 0;
    zff->buffer = NULL;
    zff->zstr = NULL;

    return zff;
}
