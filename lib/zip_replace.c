#include "zip.h"
#include "zipint.h"



int
zip_replace(struct zip *zf, int idx, char *name, struct zip_meta *meta,
	    zip_read_func *fn, void *state, int comp)
{
    if (idx == -1) {
	if (_zip_new_entry(zf) == NULL)
	    return -1;

	idx = zf->nentry - 1;
    }
    
    if (idx < 0 || idx >= zf->nentry) {
	zip_err = ZERR_NOENT;
	return -1;
    }

    if (_zip_unchange_data(zf, idx) != 0)
	return -1;

    if (_zip_set_name(zf, idx, name) != 0)
	return -1;
    
    zf->changes = 1;
    zf->entry[idx].state = Z_REPLACED;
    zf->entry[idx].ch_meta = meta;
    zf->entry[idx].ch_func = fn;
    zf->entry[idx].ch_data = state;
    zf->entry[idx].ch_comp = comp;

    return 0;
}
