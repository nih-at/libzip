/*
  $NiH: zip_add_zip.c,v 1.5 2003/03/16 10:21:38 wiz Exp $

  zip_add_zip.c -- add file from zip file
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



int
zip_add_zip(struct zip *zf, const char *name, struct zip_meta *meta,
	    struct zip *srczf, int srcidx, int start, int len)
{
    return zip_replace_zip(zf, -1, name, meta, srczf, srcidx, start, len);
}
