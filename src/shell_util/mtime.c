#include "shell_util.h"

#include <string.h>

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
