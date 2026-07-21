#include "zipint.h"

ZIP_EXTERN int zip_source_at_eof(zip_source_t *src) {
    zip_uint8_t d[1];
    zip_int64_t n;

    if (!ZIP_SOURCE_IS_OPEN_READING(src)) {
        zip_error_set(&src->error, ZIP_ER_INVAL, 0);
        return -1;
    }

    if (!src->eof) {
        if (src->supports & ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_AT_EOF)) {
            n = _zip_source_call(src, NULL, 0, ZIP_SOURCE_AT_EOF);
            if (n < 0) {
                return -1;
            }
            if (n != 0) {
                src->eof = true;
            }
        }
        else {
            n = zip_source_read(src, d, sizeof(d));
            if (n < 0) {
                return -1;
            }
            if (n > 0) {
                if (src->supports & ZIP_SOURCE_MAKE_COMMAND_BITMASK(ZIP_SOURCE_SEEK)) {
                    if (zip_source_seek(src, -1, SEEK_CUR) < 0) {
                        return -1;
                    }
                }
                else {
                    src->have_next_byte = true;
                    src->next_byte = d[0];
                    src->bytes_read -= 1;
                }
            }
        }
    }

    return src->eof ? 1 : 0;
}
