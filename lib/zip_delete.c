#include "zip.h"
#include "zipint.h"



int
zip_delete(struct zip *zf, int idx)
{
    if (idx >= zf->nentry || idx < 0) {
	zip_err = ZIP_NOENT;
	return -1;
    }

    if (_zip_unchange_data(zf, idx) != 0)
	return -1;

    zf->changes = 1;
    zf->entry[idx].state = Z_DELETED;

    return 0;
}


