#include "compat/src_compat.h"

int getopt_long(int argc, char *const argv[], const char *optstring, const struct option *longopts, int *longindex) {
    /* TODO: Implement long option parsing */
    return getopt(argc, argv, optstring);
}
