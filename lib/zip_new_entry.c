/*
  zip_new_entry.c -- create and init struct zip_entry
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

struct zip_entry *
_zip_new_entry(struct zip *zf)
{
    struct zip_entry *ze;
    if (!zf) {
	ze = (struct zip_entry *)malloc(sizeof(struct zip_entry));
	if (!ze) {
	    zip_err = ZERR_MEMORY;
	    return NULL;
	}
    } else {
	if (zf->nentry >= zf->nentry_alloc-1) {
	    zf->nentry_alloc += 16;
	    zf->entry = (struct zip_entry *)realloc(zf->entry,
						    sizeof(struct zip_entry)
						    * zf->nentry_alloc);
	    if (!zf->entry) {
		zip_err = ZERR_MEMORY;
		return NULL;
	    }
	}
	ze = zf->entry+zf->nentry;
    }
    if ((ze->meta = zip_new_meta()) == NULL)
	return NULL;

    ze->fn = ze->fn_old = NULL;
    ze->state = ZIP_ST_UNCHANGED;

    ze->ch_func = NULL;
    ze->ch_data = NULL;
    ze->ch_comp = -1;
    ze->ch_meta = NULL;

    if (zf)
	zf->nentry++;

    return ze;
}
