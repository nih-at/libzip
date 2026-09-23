/*
  zip_source_zip_fuzzer.c -- fuzz the zip_source_zip windowing/extraction paths
  Copyright (C) 2026 The libzip Authors

  SPDX-License-Identifier: BSD-3-Clause

  Coverage gap addressed:
    zip_read_fuzzer.c and zip_read_file_fuzzer.c read entries through
    zip_fopen_index(), which always calls zip_source_zip_file_create()
    with the fixed arguments (flags = 0, start = 0, len = -1).  As a
    result the partial-window and raw-extraction logic in
    zip_source_zip_new.c is never exercised by any harness:

      - start > 0 / len >= 0   (reading a sub-range of an entry; the
                                window source offset arithmetic and the
                                overflow / past-end-of-file checks)
      - ZIP_FL_COMPRESSED      (returning the raw compressed bytes wrapped
                                in a window instead of decompressing)
      - ZIP_FL_UNCHANGED       (stat/dirent selection for the original
                                on-disk entry)
      - the public zip_source_zip_create() entry point itself

    These paths combine attacker-controlled start/len with sizes parsed
    from the central directory and then layer window -> decrypt ->
    decompress -> crc sources on top, so they are worth fuzzing directly.

  Strategy:
    Open the fuzz input as a read-only archive and, for every entry,
    create a zip_source_zip for several flag / start / len combinations
    derived from the entry's stated size (in-range partial ranges as well
    as boundary and past-the-end ranges).  Each resulting source is then
    opened and drained through the source interface so the window,
    decrypt, decompress and crc layers actually run.
*/

#include <stdint.h>
#include <stddef.h>

#include <zip.h>

static const char *PASSWORD = "fuzzpw";

/* Create a zip_source_zip for the given parameters and, if it succeeds,
   drive it through the source interface so every layer is exercised. */
static void
exercise(zip_t *za, zip_uint64_t idx, zip_flags_t flags, zip_uint64_t start, zip_int64_t len) {
    zip_error_t error;
    zip_source_t *src;

    zip_error_init(&error);
    src = zip_source_zip_create(za, idx, flags, start, len, &error);
    zip_error_fini(&error);
    if (src == NULL) {
        return;
    }

    if (zip_source_open(src) == 0) {
        char buf[4096];
        while (zip_source_read(src, buf, sizeof(buf)) > 0) {
            ;
        }
        zip_source_close(src);
    }

    zip_source_free(src);
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_error_t error;
    zip_source_t *src;
    zip_t *za;
    zip_int64_t n, i;

    zip_error_init(&error);

    src = zip_source_buffer_create(data, size, 0, &error);
    if (src == NULL) {
        zip_error_fini(&error);
        return 0;
    }

    za = zip_open_from_source(src, 0, &error);
    if (za == NULL) {
        zip_source_free(src);
        zip_error_fini(&error);
        return 0;
    }
    zip_error_fini(&error);

    zip_set_default_password(za, PASSWORD);

    n = zip_get_num_entries(za, 0);
    for (i = 0; i < n; i++) {
        zip_uint64_t idx = (zip_uint64_t)i;
        zip_stat_t st;
        zip_uint64_t entry_size = 0;

        if (zip_stat_index(za, idx, 0, &st) == 0 && (st.valid & ZIP_STAT_SIZE)) {
            entry_size = st.size;
        }

        /* Whole-entry variants: decode, raw compressed window, original entry. */
        exercise(za, idx, 0, 0, -1);
        exercise(za, idx, ZIP_FL_COMPRESSED, 0, -1);
        exercise(za, idx, ZIP_FL_UNCHANGED, 0, -1);

        /* In-range partial windows: these drive the offset arithmetic. */
        if (entry_size >= 2) {
            zip_uint64_t half = entry_size / 2;
            exercise(za, idx, 0, 1, (zip_int64_t)half);
            exercise(za, idx, 0, half, (zip_int64_t)(entry_size - half));
            exercise(za, idx, 0, 0, (zip_int64_t)(entry_size - 1));
        }

        /* Boundary and past-the-end ranges exercise the rejection paths. */
        exercise(za, idx, 0, entry_size, 1);
        exercise(za, idx, 0, entry_size + 1, -1);
    }

    zip_discard(za);
    return 0;
}
