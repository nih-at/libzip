#include "zip.h"

int
_zip_entry_free(struct zip_entry *ze)
{
    int ret;
    
    free(ze->fn);
    free(ze->fn_old);
    
    zip_meta_free(ze->meta);
    zip_meta_free(ze->ch_meta);
    if (ze->ch_func)
	ret = ze->ch_func(ze->ch_data, NULL, 0, ZIP_CMD_CLOSE);
    
    free(ch_data);

    return ret;
}
