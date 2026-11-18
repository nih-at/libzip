#include "zipsh.h" /* Include first so the necessary feature defines are set for the standard library headers. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
