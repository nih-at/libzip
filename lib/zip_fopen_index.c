#include "zip.h"
#include "zipint.h"



struct zip_file *
zip_fopen_index(struct zip *zf, int fileno)
{
    unsigned char buf[4], *c;
    int len;
    struct zip_file *zff;

    if ((fileno < 0) || (fileno >= zf->nentry))
	return NULL;

    if (zf->entry[fileno].state != Z_UNCHANGED
	&& zf->entry[fileno].state != Z_RENAMED)
	return NULL;

    if ((zf->entry[fileno].comp_meth != 0)
	&& (zf->entry[fileno].comp_meth != 8)) {
	myerror(ERRFILE, "unsupported compression method %d",
		zf->entry[fileno].comp_meth);
	return NULL;
    }

    zff = zff_new(zf);

    zff->name = zf->entry[fileno].fn;
    zff->method = zf->entry[fileno].comp_meth;
    zff->bytes_left = zf->entry[fileno].uncomp_size;
    zff->cbytes_left = zf->entry[fileno].comp_size;
    zff->crc_orig = zf->entry[fileno].crc;

    /* go to start of actual data */
    fseek (zf->zp, zf->entry[fileno].local_offset+LENTRYSIZE-4, SEEK_SET);
    len = fread(buf, 1, 4, zf->zp);
    if (len != 4) {
	myerror (ERRSTR, "can't read local header");
	return NULL;
    }
    c = buf;
    zff->fpos = zf->entry[fileno].local_offset+LENTRYSIZE;
    zff->fpos += read2(&c);
    zff->fpos += read2(&c);
	
    if (zf->entry[fileno].comp_meth == 0)
	return zff;
    
    zff->buffer = (char *)xmalloc(BUFSIZE);

    len = zff_fillbuf (zff->buffer, BUFSIZE, zff);
    if (len <= 0) {
	/* XXX: error handling */
	zff_close(zff);
	return NULL;
    }

    zff->zstr = (z_stream *)xmalloc(sizeof(z_stream));
    zff->zstr->zalloc = Z_NULL;
    zff->zstr->zfree = Z_NULL;
    zff->zstr->opaque = NULL;
    zff->zstr->next_in = zff->buffer;
    zff->zstr->avail_in = len;

    /* negative value to tell zlib that there is no header */
    if (inflateInit2(zff->zstr, -MAX_WBITS) != Z_OK) {
	/* XXX: error here 
	   myerror(ERRFILE, zff->zstr->msg);
	*/
	zff_close(zff);
	return NULL;
    }
    
    return zff;
}
