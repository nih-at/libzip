/*
  zip_fclose.c -- close file in zip file
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
zip_fclose(struct zip_file *zff)
{
    int i, ret;
    
    if (zff->zstr)
	inflateEnd(zff->zstr);
    free(zff->buffer);
    free(zff->zstr);

    for (i=0; i<zff->zf->nfile; i++) {
	if (zff->zf->file[i] == zff) {
	    zff->zf->file[i] = zff->zf->file[zff->zf->nfile-1];
	    zff->zf->nfile--;
	    break;
	}
    }

    /* if EOF, compare CRC */
    if (zff->flags == -1)
	ret = (zff->crc_orig == zff->crc);
    else
	ret = zff->flags;

    free(zff);
    return ret;
}
