/*
 crypto_clear.c -- verify internal crypto buffer clearing
 Copyright (C) 2026 Dieter Baron and Thomas Klausner

 This file is part of libzip, a library to manipulate ZIP archives.
 The authors can be contacted at <info@libzip.org>

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the
 distribution.
 3. The names of the authors may not be used to endorse or promote
 products derived from this software without specific prior
 written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "zipint.h"


static int check_range(size_t start, size_t length) {
    static const unsigned char expected[] = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a};
    unsigned char buffer[sizeof(expected)];
    size_t i;
    int ret = 0;

    if (start > sizeof(buffer) || length > sizeof(buffer) - start) {
        fprintf(stderr, "invalid crypto clear test range\n");
        return -1;
    }
    memcpy(buffer, expected, sizeof(buffer));
    _zip_crypto_clear(buffer + start, length);

    for (i = 0; i < sizeof(buffer); i++) {
        unsigned char wanted = i >= start && i - start < length ? 0 : expected[i];
        if (buffer[i] != wanted) {
            fprintf(stderr, "crypto clear (%zu, %zu) byte %zu: expected %u, got %u\n", start, length, i, wanted, buffer[i]);
            ret = -1;
        }
    }

    return ret;
}


int main(void) {
    int ret = 0;
    ret |= check_range(1, 4);
    ret |= check_range(2, 0);
    ret |= check_range(0, 6);
    return ret == 0 ? 0 : 1;
}
