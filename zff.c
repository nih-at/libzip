#include <stdlib.h>
#include "error.h"
#include "xmalloc.h"
#include "ziplow.h"
#include "zip.h"

struct zf_file *
zff_new(struct zf *zf)
{
    struct zf_file *zff;

    if (zf->nfile >= zf->nfile_alloc-1) {
	zf->nfile_alloc += 10;
	zf->file = (struct zf_file **)xrealloc(zf->file, zf->nfile_alloc
					      *sizeof(struct zf_file *));
    }

    zff = (struct zf_file *)xmalloc(sizeof(struct zf_file));
    zf->file[zf->nfile++] = zff;

    zff->zf = zf;
    zff->flags = 0;
    zff->crc = crc32(0L, Z_NULL, 0);
    zff->crc_orig = 0;
    zff->method = -1;
    zff->bytes_left = zff->cbytes_left = 0;
    zff->fpos = 0;
    zff->buffer = NULL;
    zff->zstr = NULL;

    return zff;
}



int
zff_fillbuf(char *buf, int buflen, struct zf_file *zff)
{
    int i, j;

    if (zff->flags != 0)
	return -1;
    if (zff->cbytes_left <= 0 || buflen <= 0)
	return 0;
    
    fseek(zff->zf->zp, zff->fpos, SEEK_SET);
    if (buflen < zff->cbytes_left)
	i = buflen;
    else
	i = zff->cbytes_left;

    j = fread(buf, 1, i, zff->zf->zp);
    if (j == 0) 
	zff->flags = ZERR_SEEK;
    else if (j < 0)
	zff->flags = ZERR_READ;
    else {
	zff->fpos += j;
	zff->cbytes_left -= j;
    }

    return j;	
}



int
zff_close(struct zf_file *zff)
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

    /* EOF is ok */
    ret = (zff->flags == -1 ? 0 : zff->flags);
    if (!ret)
	ret = (zff->crc_orig == zff->crc);

    free(zff);
    return ret;
}




int
zff_read(struct zf_file *zff, char *outbuf, int toread)
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
	ret = zff_fillbuf(outbuf, toread, zff);
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
	    /* XXX: STREAM_END probably won't happen, since we didn't
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
		len = zff_fillbuf(zff->buffer, BUFSIZE, zff);
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
	    myerror(ERRFILE, "zlib error: buf_err: %s", zff->zstr->msg);
	    return -1;
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_STREAM_ERROR:
	case Z_MEM_ERROR:
	    /* XXX: set error */
	    myerror(ERRFILE, "zlib error: %s", zff->zstr->msg);
	    return -1;
	}
    }
}

