#include <stdlib.h>
#include "zip.h"

int
_zip_free_entry(struct zip_entry *ze)
{
    int ret;

    ret = 0;
    
    free(ze->fn);
    free(ze->fn_old);
    
    zip_free_meta(ze->meta);
    zip_free_meta(ze->ch_meta);
    if (ze->ch_func)
	ret = ze->ch_func(ze->ch_data, NULL, 0, ZIP_CMD_CLOSE);
    
    free(ze->ch_data);

    return ret;
}
