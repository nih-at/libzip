#include <stdlib.h>

#include "zip.h"
#include "xmalloc.h"

static int zip_unchange_data(struct zf *zf, int idx);
static int zip_set_name(struct zf *zf, int idx, char *name);

/* XXX: place in internal header file. */
void zip_new_entry(struct zf *zf);
void zip_entry_init(struct zf *zf, int idx);



static int
zip_unchange_data(struct zf *zf, int idx)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    if (zf->entry[idx].ch_data_fp) {
	fclose(zf->entry[idx].ch_data_fp);
	zf->entry[idx].ch_data_fp = NULL;
    }
    if (zf->entry[idx].ch_data_buf) {
	free(zf->entry[idx].ch_data_buf);
	zf->entry[idx].ch_data_buf = NULL;
    }

    zf->entry[idx].ch_data_zf = NULL;
    zf->entry[idx].ch_data_zf_fileno = 0;
    zf->entry[idx].ch_data_offset = 0;
    zf->entry[idx].ch_data_len = 0;

    zf->entry[idx].state = zf->entry[idx].fn_old ? Z_RENAMED : Z_UNCHANGED;

    return idx;
}






void
zip_entry_init(struct zf *zf, int idx)
{
    zf->entry[idx].fn = zf->entry[idx].ef = zf->entry[idx].fcom =
	zf->entry[idx].fn_old = NULL;
    zf->entry[idx].state = Z_UNCHANGED;
    zf->entry[idx].ch_data_zf = NULL;
    zf->entry[idx].ch_data_buf = NULL;
    zf->entry[idx].ch_data_fp = NULL;
    zf->entry[idx].ch_data_offset = zf->entry[idx].ch_data_len = 0;
    zf->entry[idx].ch_data_zf_fileno = 0;
}



void
zip_new_entry(struct zf *zf)
{
    if (zf->nentry >= zf->nentry_alloc-1) {
	zf->nentry_alloc += 16;
	zf->entry = (struct zf_entry *)xrealloc(zf->entry,
						 sizeof(struct zf_entry)
						 * zf->nentry_alloc);
    }
    zip_entry_init(zf, zf->nentry);

    zf->entry[zf->nentry].fn_old = NULL;
    zf->entry[zf->nentry].state = Z_ADDED;

    zf->nentry++;
}
