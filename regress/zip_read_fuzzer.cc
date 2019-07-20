#include "../lib/zip.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_source_t *src;
    zip_t *za;
    zip_error_t error;

    zip_error_init(&error);

    if ((src = zip_source_buffer_create(data, size, 0, &error)) == NULL) {
	zip_error_fini(&error);
	return 0;
    }

    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
	zip_source_free(src);
	zip_error_fini(&error);
	return 0;
    }

    zip_error_fini(&error);

    char buf[32768];
    unsigned long n = zip_get_num_entries(za, 0);

    zip_file_t *f;
    unsigned long i;
    for (i = 0; i < n; i++) {
	f = zip_fopen_index(za, i, 0);
	if (f == NULL)
	    continue;

	while (zip_fread(f, buf, 32768) > 0)
	    ;

	zip_fclose(f);
    }

    zip_close(za);

    return 0;
}
