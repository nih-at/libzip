#include <stdlib.h>
#include "zip.h"
#include "zipint.h"



/* _zip_new:
   creates a new zipfile struct, and sets the contents to zero; returns
   the new struct. */

struct zip *
_zip_new(void)
{
    struct zip *zf;

    zf = (struct zip *)malloc(sizeof(struct zip));
    if (!zf) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    zf->zn = NULL;
    zf->zp = NULL;
    zf->comlen = zf->changes = 0;
    zf->nentry = zf->nentry_alloc = zf->cd_size = zf->cd_offset = 0;
    zf->nfile = zf->nfile_alloc = 0;
    zf->com = NULL;
    zf->entry = NULL;
    zf->file = NULL;
    
    return zf;
}
