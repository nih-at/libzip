#include "zip.h"
#include "zipint.h"



struct zip_meta *
zip_get_meta(struct zip *zf, int idx)
{
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return NULL;
    }

    _zip_local_header_read(zf, idx);

    return zf->entry[idx].meta;
}
