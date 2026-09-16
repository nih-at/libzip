/*
  crypto_clear.c -- verify internal crypto buffer clearing
  SPDX-License-Identifier: BSD-3-Clause

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "config.h"

#include <stdio.h>

#include "zipint.h"


static int check_range(void) {
    unsigned char buffer[] = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a};
    size_t i;

    _zip_crypto_clear(buffer + 1, 4);

    if (buffer[0] != 0xa5 || buffer[5] != 0x5a) {
        fprintf(stderr, "crypto clear modified a guard byte\n");
        return 1;
    }
    for (i = 1; i < 5; i++) {
        if (buffer[i] != 0) {
            fprintf(stderr, "crypto clear did not clear byte %zu\n", i);
            return 1;
        }
    }

    return 0;
}


static int check_zero_length(void) {
    unsigned char buffer[] = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a};
    static const unsigned char expected[] = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a};
    size_t i;

    _zip_crypto_clear(buffer + 2, 0);

    for (i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != expected[i]) {
            fprintf(stderr, "zero-length crypto clear modified byte %zu\n", i);
            return 1;
        }
    }

    return 0;
}


int main(void) {
    return check_range() || check_zero_length();
}
