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

    /* EOF is ok */
    ret = (zff->flags == -1 ? 0 : zff->flags);
    if (!ret)
	ret = (zff->crc_orig == zff->crc);

    free(zff);
    return ret;
}
