#include <stdlib.h>

#include "zip.h"
#include "zipint.h"



int
zip_replace_file(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		 char *fname, int start, int len)
{
    FILE *fp;

    if (idx < -1 || idx >= zf->nentry) {
	zip_err = ZERR_INVAL;
	return -1;
    }
    
    if ((fp=fopen(fname, "rb")) == NULL) {
	zip_err = ZERR_OPEN;
	return -1;
    }

    return zip_replace_filep(zf, idx, (name ? name : fname), meta,
			     fp, start, len);
}
