#include "zip.h"
#include "zipint.h"



int
zip_add_zip(struct zip *zf, char *name,
	    struct zip *srczf, int srcidx, int start, int len)
{
    return zip_replace_zip(zf, -1, name, srczf, srcidx, start, len);
}
