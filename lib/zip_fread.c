#include "zip.h"
#include "zipint.h"



int
zip_fread(struct zip_file *zff, char *outbuf, int toread)
{
    int len, out_before, ret;

    if (!zff)
	return -1;

    if (zff->flags != 0)
	return -1;

    if (toread == 0)
	return 0;

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
	    /* XXX: Z_STREAM_END probably won't happen, since we didn't
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
		if (len <= 0) {
		    /* XXX: error
		       myerror (ERRSTR, "read error");
		    */
		    return -1;
		}
		zff->zstr->next_in = zff->buffer;
		zff->zstr->avail_in = len;
		continue;
	    }
	    /* XXX: set error */
	    zip_err = ZERR_ZLIB;
	    return -1;
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_STREAM_ERROR:
	case Z_MEM_ERROR:
	    /* XXX: set error */
	    zip_err = ZERR_ZLIB;
	    return -1;
	}
    }
}
