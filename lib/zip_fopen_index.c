#include "zip.h"
#include "zipint.h"

static int _zip_file_fillbuf(char *buf, int buflen, struct zip_file *zff);



struct zip_file *
zip_fopen_index(struct zip *zf, int fileno)
{
    unsigned char buf[4], *c;
    int len;
    struct zip_file *zff;

    if ((fileno < 0) || (fileno >= zf->nentry)) {
	zip_err = ZERR_NOENT;
	return NULL;
    }

    if (zf->entry[fileno].state != Z_UNCHANGED
	&& zf->entry[fileno].state != Z_RENAMED) {
	zip_err = ZERR_CHANGED;
	return NULL;
    }

    if ((zf->entry[fileno].comp_meth != 0)
	&& (zf->entry[fileno].comp_meth != 8)) {
	zip_err = ZERR_COMPNOTSUPP;
	return NULL;
    }

    zff = _zip_file_new(zf);

    zff->name = zf->entry[fileno].fn;
    zff->method = zf->entry[fileno].comp_meth;
    zff->bytes_left = zf->entry[fileno].uncomp_size;
    zff->cbytes_left = zf->entry[fileno].comp_size;
    zff->crc_orig = zf->entry[fileno].crc;

    /* go to start of actual data */
    fseek (zf->zp, zf->entry[fileno].local_offset+LENTRYSIZE-4, SEEK_SET);
    /* XXX: check fseek error */
    len = fread(buf, 1, 4, zf->zp);
    if (len != 4) {
	zip_err = ZIP_READ;
	zip_fclose(zff);
	return NULL;
    }
    
    c = buf;
    zff->fpos = zf->entry[fileno].local_offset+LENTRYSIZE;
    zff->fpos += read2(&c);
    zff->fpos += read2(&c);
	
    if (zf->entry[fileno].comp_meth == 0)
	return zff;
    
    zff->buffer = (char *)xmalloc(BUFSIZE);

    len = _zip_file_fillbuf (zff->buffer, BUFSIZE, zff);
    if (len <= 0) {
	/* XXX: error handling */
	zip_fclose(zff);
	return NULL;
    }

    if ((zff->zstr = (z_stream *)malloc(sizeof(z_stream))) == NULL) {
	zip_err = ZERR_MEMORY;
	zip_fclose(zff);
	return NULL;
    }
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
	zip_fclose(zff);
	return NULL;
    }
    
    return zff;
}



static int
_zip_file_fillbuf(char *buf, int buflen, struct zip_file *zff)
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



struct zip_file *
_zip_file_new(struct zip *zf)
{
    struct zip_file *zff;

    if (zf->nfile >= zf->nfile_alloc-1) {
	zf->nfile_alloc += 10;
	zf->file = (struct zip_file **)realloc(zf->file, zf->nfile_alloc
					       *sizeof(struct zip_file *));
	if (zf->file == NULL) {
	    zip_err = ZERR_MEMEORY;
	    return NULL;
	}
    }

    zff = (struct zf_file *)malloc(sizeof(struct zip_file));
    if (zff = NULL) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }
    
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
