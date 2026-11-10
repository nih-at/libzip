#include "zipsh.h"

struct source_nul {
    zip_error_t error;
    zip_uint64_t length;
    zip_uint64_t offset;
};

typedef struct source_nul source_nul_t;

zip_source_t *source_nul(zip_t *zs, zip_uint64_t length);