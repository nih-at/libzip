/*
  zip_replace_file.c -- replace file from file system
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



int
zip_replace_file(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		 char *fname, int start, int len)
{
    FILE *fp;

    if (idx < -1 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }
    
    if ((fp=fopen(fname, "rb")) == NULL) {
	zip_err = ZERR_OPEN;
	return -1;
    }

    return zip_replace_filep(zf, idx, (name ? name : fname), meta,
			     fp, start, len);
}
