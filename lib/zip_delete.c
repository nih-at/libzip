#include "zip.h"
#include "zipint.h"



int
zip_delete(struct zip *zf, int idx)
{
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if (zip_unchange(zf, idx) != 0)
	return -1;

    zf->changes = 1;
    zf->entry[idx].state = ZIP_ST_DELETED;

    return 0;
}


