#include "zip.h"
#include "zipint.h"



int
zip_add(struct zip *zf, char *name, zip_read_func *fn, void *state, int comp)
{
    int idx;
    
    /* XXX: create new entry */

    return zip_replace(zf, idx, name, fn, state, comp);
}
