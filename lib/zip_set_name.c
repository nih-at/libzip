#include <stdlib.h>
#include <string.h>
#include "zip.h"
#include "zipint.h"



int
_zip_set_name(struct zip *zf, int idx, char *name)
{
    if (idx >= zf->nentry || idx < 0) {
	zip_err = ZERR_NOENT;
	return -1;
    }

    if (name != NULL) {
	if (zf->entry[idx].fn_old == NULL)
	    zf->entry[idx].fn_old = zf->entry[idx].fn;
	else
	    free(zf->entry[idx].fn);
	zf->entry[idx].fn = strdup(name);
	if (zf->entry[idx].fn) {
	    zf->entry[idx].fn = zf->entry[idx].fn_old;
	    zf->entry[idx].fn_old = NULL;
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    return 0;
}
