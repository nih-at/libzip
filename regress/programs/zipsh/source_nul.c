#include "zipsh_regress.h"

#include <stdlib.h>
#include <string.h>

static zip_int64_t source_nul_cb(void *ud, void *data, zip_uint64_t length, zip_source_cmd_t command) {
    source_nul_t *ctx = (source_nul_t *)ud;

    switch (command) {
    case ZIP_SOURCE_CLOSE:
        return 0;

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&ctx->error, data, length);

    case ZIP_SOURCE_FREE:
        free(ctx);
        return 0;

    case ZIP_SOURCE_OPEN:
        ctx->offset = 0;
        return 0;

    case ZIP_SOURCE_READ:
        if (length > ZIP_INT64_MAX) {
            zip_error_set(&ctx->error, ZIP_ER_INVAL, 0);
            return -1;
        }

        if (length > ctx->length - ctx->offset) {
            length = ctx->length - ctx->offset;
        }

        memset(data, 0, length);
        ctx->offset += length;
        return (zip_int64_t)length;

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st = ZIP_SOURCE_GET_ARGS(zip_stat_t, data, length, &ctx->error);

        if (st == NULL) {
            return -1;
        }

        st->valid |= ZIP_STAT_SIZE;
        st->size = ctx->length;

        return 0;
    }

    case ZIP_SOURCE_SUPPORTS:
        return zip_source_make_command_bitmap(ZIP_SOURCE_CLOSE, ZIP_SOURCE_ERROR, ZIP_SOURCE_FREE, ZIP_SOURCE_OPEN, ZIP_SOURCE_READ, ZIP_SOURCE_STAT, -1);

    default:
        zip_error_set(&ctx->error, ZIP_ER_OPNOTSUPP, 0);
        return -1;
    }
}

zip_source_t *source_nul(zip_t *zs, zip_uint64_t length) {
    source_nul_t *ctx;
    zip_source_t *src;

    if ((ctx = (source_nul_t *)malloc(sizeof(*ctx))) == NULL) {
        zip_error_set(zip_get_error(zs), ZIP_ER_MEMORY, 0);
        return NULL;
    }

    zip_error_init(&ctx->error);
    ctx->length = length;
    ctx->offset = 0;

    if ((src = zip_source_function(zs, source_nul_cb, ctx)) == NULL) {
        free(ctx);
        return NULL;
    }

    return src;
}
