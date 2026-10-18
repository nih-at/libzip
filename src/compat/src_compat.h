#ifndef _HAD_COMPAT_COMPAT_H
#define _HAD_COMPAT_COMPAT_H

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_ASPRINTF
int asprintf(char **strp, const char *fmt, ...);
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

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};
#define no_argument 0
#define required_argument 1
#define optional_argument 2

int getopt_long(int argc, char * const argv[], const char *optstring, const struct option *longopts, int *longindex);
#endif

#ifndef HAVE_VASPRINTF
#include <stdarg.h>
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HAD_COMPAT_COMPAT_H */
