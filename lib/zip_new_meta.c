#include "zip.h"

struct zip_meta *
zip_new_meta(void)
{
    struct zip_meta *meta;
    
    if ((meta=(struct zip_meta *)malloc(sizeof(struct zip_meta)))==NULL) {
	zip_err = ZERR_MEMORY;
	return NULL;
    }

    meta->version_made = meta->version_need = meta->bitflags = -1;
    meta->comp_method = meta->disknrstart = meta->int_attr = -1;
    meta->crc = meta->comp_size = meta->uncomp_size = -1;
    meta->ext_attr = meta->local_offset = -1;
    meta->ef_len = meta->lef_len = meta->fc_len = -1;

    meta->last_mod = 0;
    meta->ef = meta->lef = meta->fc = NULL;

    return meta;
}
