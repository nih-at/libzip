#include <stdlib.h>
#include "zip.h"
#include "zipint.h"

int
_zip_merge_meta(struct zip_meta *dest, struct zip_meta *src)
{
    if (src->version_made != (unsigned short)-1)
	dest->version_made = src->version_made;
    if (src->version_need != (unsigned short)-1)
	dest->version_need = src->version_need;
    if (src->bitflags != (unsigned short)-1)
	dest->bitflags = src->bitflags;
    if (src->comp_method != (unsigned short)-1)
	dest->comp_method = src->comp_method;
    if (src->disknrstart != (unsigned short)-1)
	dest->disknrstart = src->disknrstart;
    if (src->int_attr != (unsigned short)-1)
	dest->int_attr = src->int_attr;
    
    if (src->crc != (unsigned int)-1)
	dest->crc = src->crc;
    if (src->uncomp_size != (unsigned int)-1)
	dest->uncomp_size = src->uncomp_size;
    if (src->comp_size != (unsigned int)-1)
	dest->comp_size = src->comp_size;
    if (src->ext_attr != (unsigned int)-1)
	dest->ext_attr = src->ext_attr;
    if (src->local_offset != (unsigned int)-1)
	dest->local_offset = src->local_offset;

    if (src->last_mod != 0)
	dest->last_mod = src->last_mod;
    
    if ((src->ef_len != (unsigned short)-1) && src->ef) {
	free(dest->ef);
	dest->ef = _zip_memdup(src->ef, src->ef_len);
	if (!dest->ef) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    if ((src->lef_len != (unsigned short)-1) && src->lef) {
	free(dest->lef);
	dest->lef = _zip_memdup(src->lef, src->lef_len);
	if (!dest->lef) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    if ((src->fc_len != (unsigned short)-1) && src->fc) {
	free(dest->fc);
	dest->fc = _zip_memdup(src->fc, src->fc_len);
	if (!dest->fc) {
	    zip_err = ZERR_MEMORY;
	    return -1;
	}
    }

    return 0;
}
