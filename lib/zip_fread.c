/*
  zip_fread.c -- read from file
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
zip_fread(struct zip_file *zff, char *outbuf, int toread)
{
    int len, out_before, ret;

    if (!zff)
	return -1;

    if ((zff->flags == -1) || (toread == 0))
	return 0;

    if (zff->flags != 0)
	return -1;

    if (zff->bytes_left == 0) {
	zff->flags = -1;
	if (zff->crc != zff->crc_orig) {
	    zff->flags = ZERR_CRC;
	    return -1;
	}
	return 0;
    }
    
    if (zff->method == 0) {
	ret = _zip_file_fillbuf(outbuf, toread, zff);
	if (ret > 0) {
	    zff->crc = crc32(zff->crc, outbuf, ret);
	    zff->bytes_left -= ret;
	}
	return ret;
    }
    
    zff->zstr->next_out = outbuf;
    zff->zstr->avail_out = toread;
    out_before = zff->zstr->total_out;
    
    /* endless loop until something has been accomplished */
    for (;;) {
	ret = inflate(zff->zstr, Z_SYNC_FLUSH);

	switch (ret) {
	case Z_OK:
	case Z_STREAM_END:
	    /* all ok */
	    /* Z_STREAM_END probably won't happen, since we didn't
	       have a header */
	    len = zff->zstr->total_out - out_before;
	    if (len >= zff->bytes_left || len >= toread) {
		zff->crc = crc32(zff->crc, outbuf, len);
		zff->bytes_left -= len;
	        return(len);
	    }
	    break;

	case Z_BUF_ERROR:
	    if (zff->zstr->avail_in == 0) {
		/* read some more bytes */
		len = _zip_file_fillbuf(zff->buffer, BUFSIZE, zff);
		if (len == 0) {
		    zff->flags = ZERR_INCONS;
		    return -1;
		}
		else if (len < 0)
		    return -1;
		zff->zstr->next_in = zff->buffer;
		zff->zstr->avail_in = len;
		continue;
	    }
	    zff->flags = ZERR_ZLIB;
	    return -1;
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_STREAM_ERROR:
	case Z_MEM_ERROR:
	    zff->flags = ZERR_ZLIB;
	    return -1;
	}
    }
}
