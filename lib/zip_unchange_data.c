/*
  $NiH: zip_unchange_data.c,v 1.7 2002/06/06 09:27:16 dillo Exp $

  zip_unchange_data.c -- undo helper function
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

int
_zip_unchange_data(struct zip_entry *ze)
{
    int ret;

    ret = 0;
    
    if (ze->ch_func) {
	ret = ze->ch_func(ze->ch_data, NULL, 0, ZIP_CMD_CLOSE);
	ze->ch_func = NULL;
    }
    
    free(ze->ch_data);
    ze->ch_data = NULL;
    
    ze->ch_comp = 0;

    ze->state = (ze->fn_old || ze->ch_meta) ? ZIP_ST_RENAMED
	: ZIP_ST_UNCHANGED;

    return ret;
}

