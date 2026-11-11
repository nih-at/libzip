#include "zipsh.h" /* Include first so the necessary feature defines are set for the standard library headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define decode_hex_digit(c) ((c) >= '0' && (c) <= '9' ? (c) - '0' : (c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 : (c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : -1)

static int decode_hex_byte(const char *hex, error_message_t **error_message) {
    int high = decode_hex_digit(hex[0]);
    int low = decode_hex_digit(hex[1]);
    if (high < 0 || low < 0) {
        *error_message = error_message_new_format("invalid hex character '%c'", hex[high < 0 ? 0 : 1]);
        return -1;
    }
    return (high << 4) | low;
}

zip_uint8_t *decode_hex(const char *hex, error_message_t **error_message) {
    if (!hex) {
        *error_message = error_message_new("hex string is NULL");
        return NULL;
    }
    size_t len = strlen(hex);
    if (len % 2 != 0) {
        *error_message = error_message_new("hex string has odd length");
        return NULL;
    }
    size_t decoded_len = len / 2;
    zip_uint8_t *decoded = malloc(decoded_len);
    if (!decoded) {
        *error_message = error_message_new("memory allocation failed");
        return NULL;
    }
    for (size_t i = 0; i < decoded_len; i++) {
        int byte = decode_hex_byte(hex + 2 * i, error_message);
        if (byte < 0) {
            free(decoded);
            return NULL;
        }
        decoded[i] = (zip_uint8_t)byte;
    }
    return decoded;
}

void print_hex(const zip_uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", (unsigned char)data[i]);
    }
    printf("\n");
}


const char *format_mtime(time_t mtime) {
    static char buf[256];

    struct tm *tpm;
    struct tm tm;

    tpm = zip_localtime(&mtime, &tm);
    if (tpm == NULL) {
        memcpy(buf, "<not valid>", 12);
    }
    else {
        strftime(buf, sizeof(buf), "%a %b %d %Y %H:%M:%S", tpm);
    }
    return buf;
}

int get_archive_flags(invocation_t *invocation) {
    int flags = ZIP_CREATE;
    if (invocation_get_option(invocation, OPTION_READ_ONLY) != NULL) {
        flags |= ZIP_RDONLY;
    }
    if (invocation_get_option(invocation, OPTION_CHECK_CONSISTENCY) != NULL) {
        flags |= ZIP_CHECKCONS;
    }
    return flags;
}

bool open_archive(invocation_t *invocation, error_message_t **error_message) {
    int err;

    archive = zip_open(archive_name, get_archive_flags(invocation), &err);
    if (archive == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        *error_message = error_message_new_format("can't open zip archive '%s': %s", archive_name, zip_error_strerror(&error));
        zip_error_fini(&error);
        return false;
    }

    return true;
}

bool close_archive(error_message_t **error_message) {
    if (archive != NULL) {
        if (zip_close(archive) < 0) {
            *error_message = error_message_new_format("can't close zip archive '%s': %s", archive_name, zip_strerror(archive));
            zip_discard(archive);
            return false;
        }
    }
    return true;
}
