#include "zip.h"
#include "zipint.h"



int
zip_add(struct zip *zf, char *name, zip_read_func *fn, void *state, int comp)
{
    _zip_new_entry(zf);

    return zip_replace(zf, zf->nentry-1, name, fn, state, comp);
}
