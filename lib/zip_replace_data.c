/*
  $NiH$

  zip_replace_data.c -- replace file from buffer
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

struct read_data {
    char *buf, *data;
    int len;
    int freep;
};

static int read_data(void *state, void *data, int len, enum zip_cmd cmd);




int
zip_replace_data(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		 char *data, int len, int freep)
{
    struct read_data *f;

    if (idx < -1 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }
    
    if ((f=(struct read_data *)malloc(sizeof(struct read_data))) == NULL) {
	zip_err = ZERR_MEMORY;
	return -1;
    }

    f->data = data;
    f->len = len;
    f->freep = freep;
    
    return zip_replace(zf, idx, name, meta, read_data, f, 0);
}



static int
read_data(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_data *z;
    char *buf;
    int n;

    z = (struct read_data *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	z->buf = z->data;
	return 0;
	
    case ZIP_CMD_READ:
	n = len > z->len ? z->len : len;
	if (n < 0)
	    n = 0;

	if (n) {
	    memcpy(buf, z->buf, n);
	    z->buf += n;
	    z->len -= n;
	}

	return n;
	
    case ZIP_CMD_META:
	return 0;

    case ZIP_CMD_CLOSE:
	if (z->freep) {
	    free(z->data);
	    z->data = NULL;
	}
	return 0;
    }

    return -1;
}
