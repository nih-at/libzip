/*
  $NiH: zip_fread.c,v 1.8 2003/10/02 14:13:29 dillo Exp $

  zip_fread.c -- read from file
  Copyright (C) 1999 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "zip.h"
#include "zipint.h"



ssize_t
zip_fread(struct zip_file *zff, void *outbuf, size_t toread)
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
