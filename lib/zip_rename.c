#include "zip.h"
#include "zipint.h"



int
zip_rename(struct zip *zf, int idx, char *name)
{
    if (idx >= zf->nentry || idx < 0) {
	zip_err = ZERR_INVAL;
	return -1;
    }

    if (zf->entry[idx].state == ZIP_ST_UNCHANGED) 
	zf->entry[idx].state = ZIP_ST_RENAMED;
    
    zf->changes = 1;

    _zip_set_name(zf, idx, name);

    return 0;
}
