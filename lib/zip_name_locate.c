#include <string.h>

#include "zip.h"
#include "zipint.h"



int
zip_name_locate(struct zip *zf, char *fname, int case_sens)
{
    int i;

    if (case_sens == 0)
	case_sens = 1 /* XXX: os default */;

    for (i=0; i<zf->nentry; i++)
	if ((case_sens ? strcmp : strcasecmp)(fname, zf->entry[i].fn) == 0)
	    return i;

    return -1;
}
