/*
  crypto_equal.c -- tests for _zip_crypto_equal

  Copyright (C) 2026 the libzip developers

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

#include <zipint.h>

#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

static void check(const char *name, bool got, bool expected) {
    if (got != expected) {
        fprintf(stderr, "FAIL: %s: got %d, expected %d\n", name, (int)got, (int)expected);
        failures++;
    }
}

int main(void) {
    static const zip_uint8_t hmac_a[HMAC_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    static const zip_uint8_t hmac_b[HMAC_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    static const zip_uint8_t hmac_first[HMAC_LENGTH] = {0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    static const zip_uint8_t hmac_last[HMAC_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0xff};
    static const zip_uint8_t hmac_middle[HMAC_LENGTH] = {0x00, 0x11, 0x22, 0x33, 0xff, 0x55, 0x66, 0x77, 0x88, 0x99};
    static const zip_uint8_t verify_a[WINZIP_AES_PASSWORD_VERIFY_LENGTH] = {0xaa, 0xbb};
    static const zip_uint8_t verify_b[WINZIP_AES_PASSWORD_VERIFY_LENGTH] = {0xaa, 0xba};

    check("equal values", _zip_crypto_equal(hmac_a, hmac_b, HMAC_LENGTH), true);
    check("differ at first byte", _zip_crypto_equal(hmac_a, hmac_first, HMAC_LENGTH), false);
    check("differ at last byte", _zip_crypto_equal(hmac_a, hmac_last, HMAC_LENGTH), false);
    check("differ in the middle", _zip_crypto_equal(hmac_a, hmac_middle, HMAC_LENGTH), false);
    check("single bit differs", _zip_crypto_equal(verify_a, verify_b, WINZIP_AES_PASSWORD_VERIFY_LENGTH), false);
    check("same value, other direction", _zip_crypto_equal(hmac_b, hmac_a, HMAC_LENGTH), true);
    check("zero length", _zip_crypto_equal(hmac_a, hmac_first, 0), true);

    if (failures > 0) {
        return 1;
    }

    printf("OK\n");
    return 0;
}
