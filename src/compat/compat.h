#ifndef _HAD_COMPAT_COMPAT_H
#define _HAD_COMPAT_COMPAT_H

#include "compat.h"

#ifdef __cplusplus
extern "C" {
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
