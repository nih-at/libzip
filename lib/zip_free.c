/*
  $NiH: zip_free.c,v 1.4 2002/06/06 09:27:10 dillo Exp $

  zip_free.c -- free struct zip
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



#include <stdlib.h>
#include "zip.h"
#include "zipint.h"



/* _zip_free:
   frees the space allocated to a zipfile struct, and closes the
   corresponding file. Returns 0 if successful, the error returned
   by fclose if not. */

int
_zip_free(struct zip *zf)
{
    int i, ret;

    ret = 0;

    if (zf == NULL)
	return 0;

    if (zf->zn)
	free(zf->zn);

    if (zf->zp)
	ret = fclose(zf->zp);

    if (zf->com)
	free(zf->com);

    if (zf->entry) {
	for (i=0; i<zf->nentry; i++) {
	    _zip_free_entry(zf->entry+i);
	}
	free (zf->entry);
    }

    for (i=0; i<zf->nfile; i++) {
	zf->file[i]->flags = ZERR_ZIPCLOSED;
	zf->file[i]->zf = NULL;
	zf->file[i]->name = NULL;
    }

    free(zf->file);
    
    free(zf);

    if (ret)
	zip_err = ZERR_CLOSE;
    
    return ret;
}
