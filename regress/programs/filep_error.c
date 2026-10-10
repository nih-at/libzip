/*
  filep_error.c -- check that a failed zip_source_filep_create leaves the file open
  Copyright (C) 2026

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include <stdio.h>

#include "zip.h"

int main(int argc, char *argv[]) {
    FILE *fp;
    long size;
    zip_error_t error;
    zip_source_t *zs;

    if (argc != 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        return 1;
    }

    if ((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "%s: can't open '%s'\n", argv[0], argv[1]);
        return 1;
    }
    if (fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0) {
        fprintf(stderr, "%s: can't determine size of '%s'\n", argv[0], argv[1]);
        fclose(fp);
        return 1;
    }

    /* The requested range ends one byte past the end of the file. */
    zip_error_init(&error);
    if ((zs = zip_source_filep_create(fp, 0, size + 1, &error)) != NULL) {
        fprintf(stderr, "%s: zip_source_filep_create unexpectedly succeeded\n", argv[0]);
        zip_source_free(zs);
        return 1;
    }
    printf("%s\n", zip_error_strerror(&error));
    zip_error_fini(&error);
    fflush(stdout);

    /* No source was created, so the file is still ours to close. */
    if (fclose(fp) != 0) {
        fprintf(stderr, "%s: can't close '%s'\n", argv[0], argv[1]);
        return 1;
    }
    printf("closed\n");

    return 0;
}
