/*
  $NiH$

  zip_delete.c -- delete file from zip file
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



#include "zip.h"
#include "zipint.h"



int
zip_delete(struct zip *zf, int idx)
{
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if (zip_unchange(zf, idx) != 0)
	return -1;

    zf->changes = 1;
    zf->entry[idx].state = ZIP_ST_DELETED;

    return 0;
}


