#include <stdlib.h>

#include "zip.h"
#include "zipint.h"

struct read_data {
    char *buf, *data;
    int len;
    int freep;
};

static int read_data(void *state, void *data, int len, enum zip_cmd cmd);




int
zip_replace_data(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		 char *data, int len, int freep)
{
    struct read_data *f;

    if (idx < -1 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }
    
    if ((f=(struct read_data *)malloc(sizeof(struct read_data))) == NULL) {
	zip_err = ZERR_MEMORY;
	return -1;
    }

    f->data = data;
    f->len = len;
    f->freep = freep;
    
    return zip_replace(zf, idx, name, meta, read_data, f, 0);
}



static int
read_data(void *state, void *data, int len, enum zip_cmd cmd)
{
    struct read_data *z;
    char *buf;
    int n;

    z = (struct read_data *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_CMD_INIT:
	z->buf = z->data;
	return 0;
	
    case ZIP_CMD_READ:
	n = len > z->len ? z->len : len;
	if (n < 0)
	    n = 0;

	if (n) {
	    memcpy(buf, z->buf, n);
	    z->buf += n;
	    z->len -= n;
	}

	return n;
	
    case ZIP_CMD_META:
	return 0;

    case ZIP_CMD_CLOSE:
	if (z->freep) {
	    free(z->data);
	    z->data = NULL;
	}
	return 0;
    }

    return -1;
}
