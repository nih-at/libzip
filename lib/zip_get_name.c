/*
  $NiH: zip_get_name.c,v 1.3 2003/03/16 10:21:39 wiz Exp $

  zip_get_name.c -- get filename for a file in zip file
  Copyright (C) 1999, 2003 Dieter Baron and Thomas Klausner

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



const char *
zip_get_name(struct zip *zf, int idx)
{
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return NULL;
    }

    return zf->entry[idx].fn;
}
