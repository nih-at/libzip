#include "zip.h"

void
zip_meta_free(struct zip_meta *meta)
{
    if (!meta)
	return;
    
    free(meta->ef);
    free(meta->fc);

    free(meta);
    
    return;
}
