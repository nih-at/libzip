#include <stdlib.h>
#include "zip.h"

struct zip_entry *
_zip_new_entry(struct zip *zf)
{
    struct zip_entry *ze;
    if (!zf) {
	ze = (struct zip_entry *)malloc(sizeof(struct zip_entry));
	if (!ze) {
	    zip_err = ZERR_MEMORY;
	    return NULL;
	}
    } else {
	if (zf->nentry >= zf->nentry_alloc-1) {
	    zf->nentry_alloc += 16;
	    zf->entry = (struct zip_entry *)realloc(zf->entry,
						    sizeof(struct zip_entry)
						    * zf->nentry_alloc);
	    if (!zf->entry) {
		zip_err = ZERR_MEMORY;
		return NULL;
	    }
	}
	ze = zf->entry+zf->nentry;
    }
    if ((ze->meta = zip_new_meta()) == NULL)
	return NULL;

    ze->fn = ze->fn_old = NULL;
    ze->state = ZIP_ST_UNCHANGED;

    ze->ch_func = NULL;
    ze->ch_data = NULL;
    ze->ch_comp = -1;
    ze->ch_meta = NULL;

    if (zf)
	zf->nentry++;

    return ze;
}
