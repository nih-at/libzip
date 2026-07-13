/*
  exact_read_integrity.c -- verify close reports deferred integrity failures
  Copyright (C) 2026

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "zip.h"

static int
check_case(const char *archive, const char *password, const char *name, int expected_close_err) {
    int err;
    int close_err;
    int fail;
    char *buf;
    zip_file_t *zf;
    zip_int64_t n;
    zip_stat_t st;
    zip_t *za;

    fail = 0;
    err = 0;
    buf = NULL;
    za = zip_open(archive, 0, &err);
    if (za == NULL) {
        zip_error_t zip_error;

        zip_error_init_with_code(&zip_error, err);
        fprintf(stderr, "can't open '%s': %s\n", archive, zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return 1;
    }

    if (password != NULL && zip_set_default_password(za, password) < 0) {
        fprintf(stderr, "can't set password for '%s': %s\n", archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    zip_stat_init(&st);
    if (zip_stat(za, name, 0, &st) < 0) {
        fprintf(stderr, "can't stat '%s' in '%s': %s\n", name, archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    if ((zf = zip_fopen(za, name, 0)) == NULL) {
        fprintf(stderr, "can't open '%s' in '%s': %s\n", name, archive, zip_strerror(za));
        zip_close(za);
        return 1;
    }

    if ((buf = (char *)malloc((size_t)st.size)) == NULL) {
        fprintf(stderr, "malloc failed for '%s'\n", name);
        zip_fclose(zf);
        zip_close(za);
        return 1;
    }

    n = zip_fread(zf, buf, st.size);
    if (n != (zip_int64_t)st.size) {
        if (n < 0) {
            fprintf(stderr, "zip_fread unexpectedly failed for '%s' in '%s': %s\n", name, archive, zip_file_strerror(zf));
        }
        else {
            fprintf(stderr, "zip_fread unexpectedly returned %lld instead of %llu for '%s' in '%s'\n", (long long)n, (unsigned long long)st.size, name, archive);
        }
        fail = 1;
    }

    close_err = zip_fclose(zf);
    if (close_err != expected_close_err) {
        fprintf(stderr, "zip_fclose returned %d instead of %d for '%s' in '%s'\n", close_err, expected_close_err, name, archive);
        fail = 1;
    }

    free(buf);
    zip_close(za);

    return fail;
}


int
main(void) {
    int fail;

    fail = 0;
    fail += check_case("broken.zip", NULL, "storedok", 0);
    fail += check_case("broken.zip", NULL, "storedcrcerror", ZIP_ER_CRC);
    fail += check_case("broken.zip", NULL, "deflatecrcerror", ZIP_ER_CRC);
    fail += check_case("hmac-error.zip", "1234", "test.txt", ZIP_ER_CRC);

    return fail ? 1 : 0;
}
