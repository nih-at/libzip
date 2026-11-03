/*
  pkware_keys_clear.c -- verify PKWARE keystream state is scrubbed
  Copyright (C) 2026

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


static int keys_are_zero(const zip_pkware_keys_t *keys) {
    return keys->key[0] == 0 && keys->key[1] == 0 && keys->key[2] == 0;
}


int main(void) {
    zip_pkware_keys_t keys;
    static const char password[] = "libzip-secret-password";

    _zip_pkware_keys_reset(&keys);
    _zip_pkware_encrypt(&keys, NULL, (const zip_uint8_t *)password, strlen(password));

    if (keys_are_zero(&keys)) {
        fprintf(stderr, "expected non-zero password-derived PKWARE keys\n");
        return 1;
    }

    /* Mimic the hardening added to trad_pkware_free(). */
    _zip_crypto_clear(&keys, sizeof(keys));

    if (!keys_are_zero(&keys)) {
        fprintf(stderr, "PKWARE keys were not fully cleared\n");
        return 1;
    }

    return 0;
}
