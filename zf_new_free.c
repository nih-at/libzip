#include <stdlib.h>

#include "zip.h"
#include "zipint.h"



/* _zip_zf_new:
   creates a new zipfile struct, and sets the contents to zero; returns
   the new struct. */

struct zf *
_zip_zf_new(void)
{
    struct zf *zf;

    zf = (struct zf *)xmalloc(sizeof(struct zf));

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



/* _zip_zf_free:
   frees the space allocated to a zipfile struct, and closes the
   corresponding file. Returns 0 if successful, the error returned
   by fclose if not. */

int
_zip_zf_free(struct zf *zf)
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
	    free(zf->entry[i].fn);
	    free(zf->entry[i].ef);
	    free(zf->entry[i].fcom);
	    free(zf->entry[i].fn_old);
	    if (zf->entry[i].ch_data_fp)
		(void)fclose(zf->entry[i].ch_data_fp);
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
