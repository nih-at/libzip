#include "zip.h"



int
zip_unchange(struct zip *zf, int idx)
{
    if (idx >= zf->nentry || idx < 0) {
	zip_err = ZERR_NOENT;
	return -1;
    }

    if (zf->entry[idx].fn_old) {
	free(zf->entry[idx].fn);
	zf->entry[idx].fn = zf->entry[idx].fn_old;
	zf->entry[idx].fn_old = NULL;
    }

    return _zip_unchange_data(zf, idx);
}
