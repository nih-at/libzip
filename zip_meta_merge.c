#include "zip.h"
#include "zipint.h"

int
_zip_merge_meta(struct meta *dest, struct meta *src)
{
    if (src->version_made != -1)
	dest->version_made = src->version_made;
    if (src->version_need != -1)
	dest->version_need = src->version_need;
    if (src->bitflags != -1)
	dest->bitflags = src->bitflags;
    if (src->comp_method != -1)
	dest->comp_method = src->comp_method;
    if (src->disknrstart != -1)
	dest->disknrstart = src->disknrstart;
    if (src->int_attr != -1)
	dest->int_attr = src->int_attr;
    
    if (src->crc != -1)
	dest->crc = src->crc;
    if (src->uncomp_size != -1)
	dest->uncomp_size = src->uncomp_size;
    if (src->comp_size != -1)
	dest->comp_size = src->comp_size;
    if (src->ext_attr != -1)
	dest->ext_attr = src->ext_attr;
    if (src->local_offset != -1)
	dest->local_offset = src->local_offset;

    if (src->last_mod != 0)
	dest->last_mod = src->last_mod;
    
    if ((src->ef_len != -1) && src->ef) {
	free(dest->ef);
	dest->ef = _zip_memdup(dest->ef, src->ef, src->ef_len);
	if (!dest->ef) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    if ((src->lef_len != -1) && src->lef) {
	free(dest->lef);
	dest->lef = _zip_memdup(dest->lef, src->lef, src->lef_len);
	if (!dest->lef) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    if ((src->fc_len != -1) && src->fc) {
	free(dest->fc);
	dest->fc = _zip_memdup(dest->fc, src->fc, src->fc_len);
	if (!dest->fc) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    return 0;
}
