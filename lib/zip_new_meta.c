/*
  zip_new_meta.c -- create and init struct zip_meta
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

struct zip_meta *
zip_new_meta(void)
{
    struct zip_meta *meta;
    
    if ((meta=(struct zip_meta *)malloc(sizeof(struct zip_meta)))==NULL) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    meta->version_made = meta->version_need = meta->bitflags = -1;
    meta->comp_method = meta->disknrstart = meta->int_attr = -1;
    meta->crc = meta->comp_size = meta->uncomp_size = -1;
    meta->ext_attr = meta->local_offset = -1;
    meta->ef_len = meta->lef_len = meta->fc_len = -1;

    meta->last_mod = 0;
    meta->ef = meta->lef = meta->fc = NULL;

    return meta;
}
