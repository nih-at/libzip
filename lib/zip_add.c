#include "zip.h"
#include "zipint.h"



int
zip_add(struct zip *zf, char *name, struct zip_meta *meta,
	zip_read_func *fn, void *state, int comp)
{
    return zip_replace(zf, -1, name, meta, fn, state, comp);
}
