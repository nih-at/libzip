#ifndef HAD_DIFF_OUTPUT_H
#define HAD_DIFF_OUTPUT_H

#include <zip.h>

typedef struct {
    const char *archive_names[2];
    const char *file_name;
    zip_uint64_t file_size;
    zip_uint32_t file_crc;
    int verbose;
} diff_output_t;

void diff_output_init(diff_output_t *output, int verbose, char *const archive_names[]);
void diff_output_start_file(diff_output_t *output, const char *name, zip_uint64_t size, zip_uint32_t crc);
void diff_output_end_file(diff_output_t *output);

void diff_output(diff_output_t *output, int side, const char *fmt, ...) __attribute__((__format__(__printf__, 3, 4)));
void diff_output_data(diff_output_t *output, int side, const zip_uint8_t *data, zip_uint64_t data_length, const char *fmt, ...) __attribute__((__format__(__printf__, 5, 6)));
void diff_output_file(diff_output_t *output, char side, const char *name, zip_uint64_t size, zip_uint32_t crc);


#endif /* HAD_DIFF_OUTPUT_H */
