/*
  zip_new.c -- create and init struct zip
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



/* _zip_new:
   creates a new zipfile struct, and sets the contents to zero; returns
   the new struct. */

struct zip *
_zip_new(void)
{
    struct zip *zf;

    zf = (struct zip *)malloc(sizeof(struct zip));
    if (!zf) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    zf->zn = NULL;
    zf->zp = NULL;
    zf->comlen = zf->changes = 0;
    zf->nentry = zf->nentry_alloc = zf->cd_size = zf->cd_offset = 0;
    zf->nfile = zf->nfile_alloc = 0;
    zf->com = NULL;
    zf->entry = NULL;
    zf->file = NULL;
    
    return zf;
}
