/*
  local_header_mismatch.c -- verify default reads reject local/central header mismatches
  Copyright (C) 2026

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

#define ARCHIVE "local-header-mismatch.zip"

#define CENTRAL_CRC_OFFSET 16
#define CENTRAL_COMP_SIZE_OFFSET 20
#define CENTRAL_UNCOMP_SIZE_OFFSET 24
#define CENTRAL_LOCAL_HEADER_OFFSET 42

static const zip_uint8_t central_magic[] = {0x50, 0x4b, 0x01, 0x02};
static const zip_uint8_t local_magic[] = {0x50, 0x4b, 0x03, 0x04};

static int add_stored_file(zip_t *za, const char *name, const zip_uint8_t *data, zip_uint64_t length);
static int check_checkcons_rejects_archive(void);
static int check_default_read_rejects_archive(void);
static int create_confused_archive(void);
static int find_nth_signature(const zip_uint8_t *data, size_t length, const zip_uint8_t *signature, unsigned int nth, size_t *offsetp);
static int patch_central_directory(void);
static zip_uint32_t read_le32(const zip_uint8_t *data);
static void write_le32(zip_uint8_t *data, zip_uint32_t value);

static int
add_stored_file(zip_t *za, const char *name, const zip_uint8_t *data, zip_uint64_t length) {
    zip_int64_t index;
    zip_source_t *source;

    if ((source = zip_source_buffer(za, data, length, 0)) == NULL) {
        fprintf(stderr, "can't create source for '%s': %s\n", name, zip_strerror(za));
        return -1;
    }

    if ((index = zip_file_add(za, name, source, 0)) < 0) {
        fprintf(stderr, "can't add '%s': %s\n", name, zip_strerror(za));
        zip_source_free(source);
        return -1;
    }

    if (zip_set_file_compression(za, (zip_uint64_t)index, ZIP_CM_STORE, 0) < 0) {
        fprintf(stderr, "can't force stored compression for '%s': %s\n", name, zip_strerror(za));
        return -1;
    }

    return 0;
}


static int
check_checkcons_rejects_archive(void) {
    int err;
    zip_t *za;

    err = 0;
    za = zip_open(ARCHIVE, ZIP_CHECKCONS, &err);
    if (za != NULL) {
        fprintf(stderr, "ZIP_CHECKCONS unexpectedly accepted crafted archive\n");
        zip_close(za);
        return -1;
    }
    if (err != ZIP_ER_INCONS) {
        zip_error_t zip_error;

        zip_error_init_with_code(&zip_error, err);
        fprintf(stderr, "ZIP_CHECKCONS failed with %s instead of inconsistency\n", zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return -1;
    }

    return 0;
}


static int
check_default_read_rejects_archive(void) {
    int err;
    zip_error_t *zip_error;
    zip_file_t *zf;
    zip_t *za;

    err = 0;
    za = zip_open(ARCHIVE, 0, &err);
    if (za == NULL) {
        zip_error_t open_error;

        zip_error_init_with_code(&open_error, err);
        fprintf(stderr, "can't open crafted archive without ZIP_CHECKCONS: %s\n", zip_error_strerror(&open_error));
        zip_error_fini(&open_error);
        return -1;
    }

    zf = zip_fopen(za, "safe.txt", 0);
    if (zf != NULL) {
        char buffer[32];
        zip_int64_t n;

        n = zip_fread(zf, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            fprintf(stderr, "zip_fopen unexpectedly succeeded and zip_fread then failed: %s\n", zip_file_strerror(zf));
        }
        else {
            buffer[n] = '\0';
            fprintf(stderr, "zip_fopen unexpectedly succeeded and returned '%s'\n", buffer);
        }
        zip_fclose(zf);
        zip_close(za);
        return -1;
    }

    zip_error = zip_get_error(za);
    if (zip_error_code_zip(zip_error) != ZIP_ER_INCONS) {
        fprintf(stderr, "zip_fopen failed with %s instead of inconsistency\n", zip_strerror(za));
        zip_close(za);
        return -1;
    }

    zip_close(za);
    return 0;
}


static int
create_confused_archive(void) {
    static const zip_uint8_t evil_payload[] = "EVIL_PAYLOAD!!";
    static const zip_uint8_t safe_payload[] = "SAFE";
    int err;
    zip_t *za;

    err = 0;
    za = zip_open(ARCHIVE, ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (za == NULL) {
        zip_error_t zip_error;

        zip_error_init_with_code(&zip_error, err);
        fprintf(stderr, "can't create '%s': %s\n", ARCHIVE, zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return -1;
    }

    if (add_stored_file(za, "safe.txt", safe_payload, sizeof(safe_payload) - 1) < 0
        || add_stored_file(za, "evil.txt", evil_payload, sizeof(evil_payload) - 1) < 0) {
        zip_discard(za);
        return -1;
    }

    if (zip_close(za) < 0) {
        fprintf(stderr, "can't finalize '%s': %s\n", ARCHIVE, zip_strerror(za));
        zip_discard(za);
        return -1;
    }

    return patch_central_directory();
}


static int
find_nth_signature(const zip_uint8_t *data, size_t length, const zip_uint8_t *signature, unsigned int nth, size_t *offsetp) {
    size_t i;
    unsigned int seen;

    seen = 0;
    for (i = 0; i + 4 <= length; i++) {
        if (memcmp(data + i, signature, 4) == 0) {
            if (seen == nth) {
                *offsetp = i;
                return 0;
            }
            seen++;
        }
    }

    return -1;
}


static int
patch_central_directory(void) {
    zip_uint8_t *data;
    size_t evil_central_offset, evil_local_offset, safe_central_offset, safe_local_offset;
    size_t length;
    FILE *fp;

    if ((fp = fopen(ARCHIVE, "rb")) == NULL) {
        fprintf(stderr, "can't reopen '%s' for patching\n", ARCHIVE);
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) < 0) {
        fprintf(stderr, "can't seek to end of '%s'\n", ARCHIVE);
        fclose(fp);
        return -1;
    }
    length = (size_t)ftell(fp);
    rewind(fp);

    if ((data = (zip_uint8_t *)malloc(length)) == NULL) {
        fprintf(stderr, "malloc failed while patching '%s'\n", ARCHIVE);
        fclose(fp);
        return -1;
    }
    if (fread(data, 1, length, fp) != length) {
        fprintf(stderr, "can't read '%s'\n", ARCHIVE);
        free(data);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    if (find_nth_signature(data, length, local_magic, 0, &safe_local_offset) < 0
        || find_nth_signature(data, length, local_magic, 1, &evil_local_offset) < 0
        || find_nth_signature(data, length, central_magic, 0, &safe_central_offset) < 0
        || find_nth_signature(data, length, central_magic, 1, &evil_central_offset) < 0) {
        fprintf(stderr, "can't locate expected ZIP headers in '%s'\n", ARCHIVE);
        free(data);
        return -1;
    }

    if (read_le32(data + safe_central_offset + CENTRAL_LOCAL_HEADER_OFFSET) != (zip_uint32_t)safe_local_offset
        || read_le32(data + evil_central_offset + CENTRAL_LOCAL_HEADER_OFFSET) != (zip_uint32_t)evil_local_offset) {
        fprintf(stderr, "unexpected local header offsets while patching '%s'\n", ARCHIVE);
        free(data);
        return -1;
    }

    write_le32(data + safe_central_offset + CENTRAL_CRC_OFFSET, read_le32(data + evil_central_offset + CENTRAL_CRC_OFFSET));
    write_le32(data + safe_central_offset + CENTRAL_COMP_SIZE_OFFSET, read_le32(data + evil_central_offset + CENTRAL_COMP_SIZE_OFFSET));
    write_le32(data + safe_central_offset + CENTRAL_UNCOMP_SIZE_OFFSET, read_le32(data + evil_central_offset + CENTRAL_UNCOMP_SIZE_OFFSET));
    write_le32(data + safe_central_offset + CENTRAL_LOCAL_HEADER_OFFSET, read_le32(data + evil_central_offset + CENTRAL_LOCAL_HEADER_OFFSET));

    if ((fp = fopen(ARCHIVE, "wb")) == NULL) {
        fprintf(stderr, "can't reopen '%s' for writing\n", ARCHIVE);
        free(data);
        return -1;
    }
    if (fwrite(data, 1, length, fp) != length) {
        fprintf(stderr, "can't rewrite '%s'\n", ARCHIVE);
        free(data);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    free(data);

    return 0;
}


static zip_uint32_t
read_le32(const zip_uint8_t *data) {
    return (zip_uint32_t)data[0]
           | ((zip_uint32_t)data[1] << 8)
           | ((zip_uint32_t)data[2] << 16)
           | ((zip_uint32_t)data[3] << 24);
}


static void
write_le32(zip_uint8_t *data, zip_uint32_t value) {
    data[0] = (zip_uint8_t)(value & 0xff);
    data[1] = (zip_uint8_t)((value >> 8) & 0xff);
    data[2] = (zip_uint8_t)((value >> 16) & 0xff);
    data[3] = (zip_uint8_t)((value >> 24) & 0xff);
}


int
main(void) {
    if (create_confused_archive() < 0) {
        return 1;
    }
    if (check_checkcons_rejects_archive() < 0) {
        return 1;
    }
    if (check_default_read_rejects_archive() < 0) {
        return 1;
    }

    return 0;
}
