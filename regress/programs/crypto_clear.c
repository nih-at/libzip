/*
  crypto_clear.c -- verify internal crypto buffer clearing
  SPDX-License-Identifier: BSD-3-Clause

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "zipint.h"


static int check_range(size_t start, size_t length) {
    static const unsigned char expected[] = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a};
    unsigned char buffer[sizeof(expected)];
    size_t i;

    if (start > sizeof(buffer) || length > sizeof(buffer) - start) {
        fprintf(stderr, "invalid crypto clear test range\n");
        return 1;
    }
    memcpy(buffer, expected, sizeof(buffer));
    _zip_crypto_clear(buffer + start, length);

    for (i = 0; i < sizeof(buffer); i++) {
        unsigned char wanted = i >= start && i - start < length ? 0 : expected[i];
        if (buffer[i] != wanted) {
            fprintf(stderr, "crypto clear byte %zu: expected %u, got %u\n", i, wanted, buffer[i]);
            return 1;
        }
    }

    return 0;
}


int main(void) {
    return check_range(1, 4) || check_range(2, 0) || check_range(0, 6);
}
