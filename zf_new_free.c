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



/* _zip_free:
   frees the space allocated to a zipfile struct, and closes the
   corresponding file. Returns 0 if successful, the error returned
   by fclose if not. */

int
_zip_free(struct zip *zf)
{
    int i, ret;

    if (zf == NULL)
	return 0;

    if (zf->zn)
	free(zf->zn);

    if (zf->zp)
	ret = fclose(zf->zp);

    if (zf->com)
	free(zf->com);

    if (zf->entry) {
	for (i=0; i<zf->nentry; i++) {
	    _zip_free_entry(zf->entry+i);
	}
	free (zf->entry);
    }

    for (i=0; i<zf->nfile; i++) {
	zf->file[i]->flags = ZERR_ZIPCLOSED;
	zf->file[i]->zf = NULL;
	zf->file[i]->name = NULL;
    }

    free(zf->file);
    
    free(zf);

    if (ret)
	zip_err = ZERR_CLOSE;
    
    return ret;
}
