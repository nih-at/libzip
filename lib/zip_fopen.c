#include "zip.h"
#include "zipint.h"



struct zip_file *
zip_fopen(struct zip *zf, char *fname, int case_sens)
{
    int idx;

    if ((idx=zip_name_locate(zf, fname, case_sens)) < 0) {
	zip_err = ZERR_NOENT;
	return NULL;
    }

    return zip_fopen_index(zf, idx);
}
