#include "compat/compat.h"

int vasprintf(char **strp, const char *fmt, va_list ap) {
    int size = vsnprintf(NULL, 0, fmt, ap);
    if (size < 0) {
        return -1;
    }
    *strp = (char *)malloc(size + 1);
    if (!*strp) {
        return -1;
    }
    return vsnprintf(*strp, size + 1, fmt, ap);
}
