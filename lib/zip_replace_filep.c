/*
  $NiH$

  zip_replace_filep.c -- replace file from FILE*
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



#include <stdlib.h>

#include "zip.h"
#include "zipint.h"

struct read_file {
    FILE *f;
    int off, len;
};

static int read_file(void *state, void *data, int len, enum zip_cmd cmd);



int
zip_replace_filep(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		 FILE *file, int start, int len)
{
    struct read_file *f;

    if (idx < -1 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }
    
    if ((f=(struct read_file *)malloc(sizeof(struct read_file))) == NULL) {
	zip_err = ZERR_MEMORY;
	return -1;
    }

    f->f = file;
    f->off = start;
    f->len = (len ? len : -1);
    
    return zip_replace(zf, idx, name, meta, read_file, f, 0);
}



static int
read_file(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_file *z;
    char *buf;
    int i, n;

    z = (struct read_file *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	if (fseek(z->f, z->off, SEEK_SET) < 0) {
	    zip_err = ZERR_SEEK;
	    return -1;
	}
	return 0;
	
    case ZIP_CMD_READ:
	if (z->len != -1)
	    n = len > z->len ? z->len : len;
	else
	    n = len;
	
	if ((i=fread(buf, 1, n, z->f)) < 0) {
	    zip_err = ZERR_READ;
	    return -1;
	}

	if (z->len != -1)
	    z->len -= i;

	return i;
	
    case ZIP_CMD_META:
	return 0;

    case ZIP_CMD_CLOSE:
	if (z->f) {
	    fclose(z->f);
	    z->f = NULL;
	}
	return 0;
    }

    return -1;
}
