#ifndef _HAD_COMPAT_COMPAT_H
#define _HAD_COMPAT_COMPAT_H

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_ASPRINTF
int asprintf(char **strp, const char *fmt, ...);
#endif

#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#else
#include "fnmatch.h"
#endif

#ifndef HAVE_GETLINE
#include <stdio.h>
#include <stdlib.h>
zip_int64_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
extern char *optarg;
extern int optind;
extern int opterr;

extern int getopt(int, char *const *, const char *);
#endif
extern int optopt;

#ifndef HAVE_ISATTY
#ifdef HAVE__ISATTY
#define isatty(fd) _isatty(fd)
#endif
#endif

#ifndef HAVE_VASPRINTF
#include <stdarg.h>
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HAD_COMPAT_COMPAT_H */
