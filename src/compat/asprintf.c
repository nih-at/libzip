#include "compat/src_compat.h"

#include <stdarg.h>
#include <stdio.h>

int asprintf(char **strp, const char *fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}
