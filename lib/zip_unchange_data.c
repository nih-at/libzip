#include "zip.h"
#include "zipint.h"

int
_zip_unchange_data(struct zip_entry *ze)
{
    int ret;
    
    if (ze->ch_func) {
	ret = ze->ch_func(ze->ch_data, NULL, 0, ZIP_CMD_CLOSE);
	ze->ch_func = NULL;
    }
    
    free(ze->ch_data);
    ze->ch_data = NULL;
    
    ze->ch_comp = 0;

    ze->state = (ze->fn_old || ze->ch_meta) ? ZIP_ST_RENAMED
	: ZIP_ST_UNCHANGED;

    return ret;
}

