#include <stdlib.h>

#include "zip.h"
#include "zipint.h"



int
zip_fclose(struct zip_file *zff)
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

    /* if EOF, compare CRC */
    if (zff->flags == -1)
	ret = (zff->crc_orig == zff->crc);
    else
	ret = zff->flags;

    free(zff);
    return ret;
}
