#include "zip.h"
#include "zipint.h"



int
zip_add_data(struct zip *zf, char *name, struct zip_meta *meta,
	    char *data, int len, int freep)
{
    return zip_replace_data(zf, -1, name, meta, data, len, freep);
}
