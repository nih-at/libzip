/*
  $NiH: zip_replace.c,v 1.8 2002/06/06 09:27:14 dillo Exp $

  zip_replace.c -- replace file via callback function
  Copyright (C) 1999 Dieter Baron and Thomas Klausner

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



#include "zip.h"
#include "zipint.h"



int
zip_replace(struct zip *zf, int idx, char *name, struct zip_meta *meta,
	    zip_read_func fn, void *state, int comp)
{
    if (idx == -1) {
	if (_zip_new_entry(zf) == NULL)
	    return -1;

	idx = zf->nentry - 1;
    }
    
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if (_zip_unchange_data(zf->entry+idx) != 0)
	return -1;

    if (_zip_set_name(zf, idx, name) != 0)
	return -1;
    
    zf->changes = 1;
    zf->entry[idx].state = ZIP_ST_REPLACED;
    if (zf->entry[idx].ch_meta)
	zip_free_meta(zf->entry[idx].ch_meta);
    zf->entry[idx].ch_meta = meta;
    zf->entry[idx].ch_func = fn;
    zf->entry[idx].ch_data = state;
    zf->entry[idx].ch_comp = comp;

    return 0;
}
