#ifndef _HAD_ZIPINT_H
#define _HAD_ZIPINT_H

/*
  $NiH: zipint.h,v 1.16 2003/10/01 09:51:01 dillo Exp $

  zipint.h -- internal declarations.
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



#define MAXCOMLEN        65536
#define EOCDLEN             22
#define BUFSIZE       (MAXCOMLEN+EOCDLEN)
#define LOCAL_MAGIC   "PK\3\4"
#define CENTRAL_MAGIC "PK\1\2"
#define EOCD_MAGIC    "PK\5\6"
#define DATADES_MAGIC "PK\7\8"
#define CDENTRYSIZE         46
#define LENTRYSIZE          30



void _zip_entry_init(struct zip *, int);
int _zip_file_fillbuf(char *, int, struct zip_file *);
int _zip_free(struct zip *);
int _zip_free_entry(struct zip_entry *);
int _zip_local_header_read(struct zip *, int);
void *_zip_memdup(const void *, int);
int _zip_merge_meta(struct zip_meta *, struct zip_meta *);
int _zip_merge_meta_fix(struct zip_meta *, struct zip_meta *);
struct zip *_zip_new(void);
struct zip_entry *_zip_new_entry(struct zip *);
int _zip_readcdentry(FILE *, struct zip_entry *, unsigned char **, 
		     int, int, int);
int _zip_set_name(struct zip *, int, const char *);
int _zip_unchange(struct zip_entry *);
int _zip_unchange_data(struct zip_entry *);

#endif /* zipint.h */

