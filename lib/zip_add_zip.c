#include "zip.h"
#include "zipint.h"



int
zip_add_zip(struct zip *zf, char *name, struct zip_meta *meta,
	    struct zip *srczf, int srcidx, int start, int len)
{
    return zip_replace_zip(zf, -1, name, meta, srczf, srcidx, start, len);
}
