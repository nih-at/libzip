#include <stdlib.h>

#include "zip.h"
#include "xmalloc.h"

static int zip_unchange_data(struct zf *zf, int idx);
static int zip_set_name(struct zf *zf, int idx, char *name);

/* XXX: place in internal header file. */
void zip_new_entry(struct zf *zf);
void zip_entry_init(struct zf *zf, int idx);



int
zip_rename(struct zf *zf, int idx, char *name)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    if (zf->entry[idx].state == Z_UNCHANGED) 
	zf->entry[idx].state = Z_RENAMED;
    zf->changes = 1;

    zip_set_name(zf, idx, name);

    return idx;
}



int
zip_add_file(struct zf *zf, char *name, FILE *file,
	int start, int len)
{
    zip_new_entry(zf);

    zf->changes = 1;
    zip_set_name(zf, zf->nentry-1, name ? name : "-");
    zf->entry[zf->nentry-1].ch_data_fp = file;
    zf->entry[zf->nentry-1].ch_data_offset = start;
    zf->entry[zf->nentry-1].ch_data_len = len;

    return zf->nentry-1;
}



int
zip_add_data(struct zf *zf, char *name, char *buf,
	int start, int len)
{
    zip_new_entry(zf);

    zf->changes = 1;
    zip_set_name(zf, zf->nentry-1, name ? name : "-");
    zf->entry[zf->nentry-1].ch_data_buf = buf;
    zf->entry[zf->nentry-1].ch_data_offset = start;
    zf->entry[zf->nentry-1].ch_data_len = len;

    return zf->nentry-1;
}



int
zip_add_zip(struct zf *zf, char *name, struct zf *srczf, int srcidx,
	    int start, int len)
{
    if (srcidx >= srczf->nentry || srcidx < 0)
	return -1;

    zip_new_entry(zf);

    zf->changes = 1;
    zip_set_name(zf, zf->nentry-1, name ? name : srczf->entry[srcidx].fn);
    zf->entry[zf->nentry-1].ch_data_zf = srczf;
    zf->entry[zf->nentry-1].ch_data_zf_fileno = srcidx;
    zf->entry[zf->nentry-1].ch_data_offset = start;
    zf->entry[zf->nentry-1].ch_data_len = len;

    return zf->nentry-1;
}



int
zip_replace_file(struct zf *zf, int idx, char *name, FILE *file,
	    int start, int len)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    zip_unchange_data(zf, idx);

    zf->changes = 1;
    zf->entry[idx].state = Z_REPLACED;
    zip_set_name(zf, idx, name);
    
    zf->entry[idx].ch_data_fp = file;
    zf->entry[idx].ch_data_offset = start;
    zf->entry[idx].ch_data_len = len;

    return idx;
}



int
zip_replace_data(struct zf *zf, int idx, char *name, char *buf,
	    int start, int len)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    zip_unchange_data(zf, idx);

    zf->changes = 1;
    zf->entry[idx].state = Z_REPLACED;
    zip_set_name(zf, idx, name);
    
    zf->entry[idx].ch_data_buf = buf;
    zf->entry[idx].ch_data_offset = start;
    zf->entry[idx].ch_data_len = len;

    return idx;
}



int
zip_replace_zip(struct zf *zf, int idx, char *name, struct zf *srczf,
		int srcidx, int start, int len)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    if (srcidx >= srczf->nentry || srcidx < 0)
	return -1;

    zip_unchange_data(zf, idx);

    zf->changes = 1;
    zf->entry[idx].state = Z_REPLACED;
    zip_set_name(zf, idx, name);
    
    zf->entry[idx].ch_data_zf = srczf;
    zf->entry[idx].ch_data_zf_fileno = srcidx;
    zf->entry[idx].ch_data_offset = start;
    zf->entry[idx].ch_data_len = len;

    return idx;
}



int
zip_delete(struct zf *zf, int idx)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    zip_unchange(zf, idx);

    zf->changes = 1;
    zf->entry[idx].state = Z_DELETED;

    return idx;
}



int
zip_unchange(struct zf *zf, int idx)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    if (zf->entry[idx].fn_old) {
	free(zf->entry[idx].fn);
	zf->entry[idx].fn = zf->entry[idx].fn_old;
	zf->entry[idx].fn_old = NULL;
	zf->entry[idx].fnlen = strlen(zf->entry[idx].fn);
    }

    return zip_unchange_data(zf, idx);
}



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



static int
zip_set_name(struct zf *zf, int idx, char *name)
{
    if (idx >= zf->nentry || idx < 0)
	return -1;

    if (name != NULL) {
	if (zf->entry[idx].fn_old == NULL)
	    zf->entry[idx].fn_old = zf->entry[idx].fn;
	else
	    free(zf->entry[idx].fn);
	zf->entry[idx].fn = xstrdup(name);
	zf->entry[idx].fnlen = strlen(name);
    }

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
