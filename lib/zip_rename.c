#include "zip.h"
#include "zipint.h"



int
zip_rename(struct zip *zf, int idx, char *name)
{
    if (idx >= zf->nentry || idx < 0) {
	zip_err = ZIP_NOENT;
	return -1;
    }

    if (zf->entry[idx].state == Z_UNCHANGED) 
	zf->entry[idx].state = Z_RENAMED;
    
    zf->changes = 1;

    _zip_set_name(zf, idx, name);

    return 0;
}
