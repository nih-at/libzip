#include <stdlib.h>
#include "zip.h"
#include "zipint.h"

struct read_zip {
    struct zip *zf;
    int idx;
    /* ... */
};

struct read_part {
    struct zip *zf;
    int idx;
    struct zip_file *zff;
    int off, len;
    /* ... */
};

static int read_zip(void *state, void *data, int len, enum zip_cmd cmd);
static int read_part(void *state, void *data, int len, enum zip_cmd cmd);



int
zip_replace_zip(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		struct zip *srczf, int srcidx, int start, int len)
{
    struct read_zip *z;
    struct read_part *p;

    if (srcidx < -1 || srcidx >= srczf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if (start == 0 && (len == 0 || len == -1)) {
	if ((z=(struct read_zip *)malloc(sizeof(struct read_zip))) == NULL) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
	z->zf = srczf;
	z->idx = srcidx;
	return zip_replace(zf, idx, name, meta, read_zip, z, 1);
    }
    else {
	if ((p=(struct read_part *)malloc(sizeof(struct read_part))) == NULL) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
	p->zf = srczf;
	p->idx = srcidx;
	p->off = start;
	p->len = len;
	p->zff = NULL;
	return zip_replace(zf, idx, name, meta, read_part, z, 0);
    }
}



static int
read_zip(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_zip *z;

    z = (struct read_zip *)state;

    switch (cmd) {
    case ZIP_CMD_INIT:
	/* XXX */
	break;
	
    case ZIP_CMD_READ:
	/* XXX */
	break;
	
    case ZIP_CMD_META:
	/* XXX */
	break;

    case ZIP_CMD_CLOSE:
	return 0;
    }

    return 0;
}



static int
read_part(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_part *z;
    char b[8192], *buf;
    int i, n;

    z = (struct read_part *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	if ((z->zff=zip_fopen_index(z->zf, z->idx) ) == NULL)
	    return -1;

	for (n=0; n<z->off; n+= i) {
	    i = (z->off-n > 8192 ? 8192 : z->off-n);
	    if ((i=zip_fread(z->zff, b, n)) < 0) {
		zip_fclose(z->zff);
		z->zff = NULL;
		return -1;
	    }
	}
	return 0;
	
    case ZIP_CMD_READ:
	if (z->len != -1)
	    n = len > z->len ? z->len : len;
	else
	    n = len;
	
	if ((i=zip_fread(z->zff, buf, n)) < 0)
	    return -1;

	if (z->len != -1)
	    z->len -= i;

	return i;
	
    case ZIP_CMD_META:
	return 0;

    case ZIP_CMD_CLOSE:
	if (z->zff) {
	    zip_fclose(z->zff);
	    z->zff = NULL;
	}
	return 0;
    }

    return -1;
}
