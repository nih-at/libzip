#include <stdlib.h>
#include "zip.h"

void
zip_free_meta(struct zip_meta *meta)
{
    if (!meta)
	return;
    
    free(meta->ef);
    free(meta->fc);

    free(meta);
    
    return;
}
