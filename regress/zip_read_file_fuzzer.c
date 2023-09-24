#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zip.h>

void
randomize(char *buf, int count) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int i;
    srand(time(NULL));
    for (i = 0; i < count; i++) {
        buf[i] = charset[rand() % sizeof(charset)];
    }
}

/**
   This fuzzing target takes input data, creates a ZIP archive from it, checks the archive's consistency,
   and iterates over the entries in the archive, reading data from each entry.
**/

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_t *za;
    char buf[32768];
    zip_int64_t i, n;
    zip_file_t *f;
    char name[20 + 4 + 1];
    FILE *fp;

    snprintf(name, sizeof(name), "XXXXXXXXXXXXXXXXXXXX.zip");
    randomize(name, 20);

    if ((fp = fopen(name, "w+bx")) == NULL) {
        fprintf(stderr, "can't create file '%s': %s\n", name, strerror(errno));
        return 1;
    }
    if (fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        fprintf(stderr, "can't write data to file '%s': %s\n", name, strerror(errno));
        return 1;
    }
    if (fclose(fp) < 0) {
        fprintf(stderr, "can't close file '%s': %s\n", name, strerror(errno));
        return 1;
    }

    za = zip_open(name, 0, NULL);
    if (za == NULL) {
        (void)remove(name);
        fprintf(stderr, "Error opening archive '%s'\n", name);
        return 1;
    }

    n = zip_get_num_entries(za, 0);
    for (i = 0; i < n; i++) {
        f = zip_fopen_index(za, i, 0);
        if (f == NULL) {
            fprintf(stderr, "Unable to open file %d.\n", (int)i);
            zip_close(za);
            (void)remove(name);
            return 1;
        }

        while (zip_fread(f, buf, sizeof(buf)) > 0) {
            ;
        }
        if (zip_fclose(f) < 0) {
            fprintf(stderr, "Error closing file %d\n", (int)i);
            zip_close(za);
            (void)remove(name);
            return 1;
        }
    }
    if (zip_close(za) < 0) {
        zip_discard(za);
        (void)remove(name);
        fprintf(stderr, "Error closing archiv '%s'\n", name);
        return 1;
    }
    (void)remove(name);
    return 0;
}
