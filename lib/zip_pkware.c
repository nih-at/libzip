/*
  zip_pkware.c -- Traditional PKWARE de/encryption backend routines
  Copyright (C) 2009-2020 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

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


#include <stdlib.h>
#include <string.h>

#include "zipint.h"
#include "zip_crypto.h"


#define PKWARE_KEY0 305419896
#define PKWARE_KEY1 591751049
#define PKWARE_KEY2 878082192


struct _zip_trad_pkware {
    zip_uint32_t key[3];
};


void
update_keys(zip_trad_pkware_t *ctx, Bytef b) {
    ctx->key[0] = (zip_uint32_t) crc32(ctx->key[0] ^ 0xffffffffUL, &b, 1) ^ 0xffffffffUL;
    ctx->key[1] = (ctx->key[1] + (ctx->key[0] & 0xff)) * 134775813 + 1;
    b = (Bytef) (ctx->key[1] >> 24);
    ctx->key[2] = (zip_uint32_t) crc32(ctx->key[2] ^ 0xffffffffUL, &b, 1) ^ 0xffffffffUL;
}


Bytef
decrypt_byte(zip_trad_pkware_t *ctx) {
    zip_uint16_t tmp;
    tmp = (zip_uint16_t) (ctx->key[2] | 2);
    tmp = (zip_uint16_t) (((zip_uint32_t) tmp * (tmp ^ 1)) >> 8);
    return (Bytef)tmp;
}


zip_trad_pkware_t *
_zip_pkware_new(zip_error_t *error) {
    zip_trad_pkware_t *ctx;

    if ((ctx = (zip_trad_pkware_t *) malloc(sizeof(*ctx))) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }
    ctx->key[0] = PKWARE_KEY0;
    ctx->key[1] = PKWARE_KEY1;
    ctx->key[2] = PKWARE_KEY2;

    return ctx;
}


void
encrypt(zip_trad_pkware_t *ctx, zip_uint8_t *out, const zip_uint8_t *in, zip_uint64_t len, int update_only) {
    zip_uint64_t i;
    Bytef b;
    Bytef tmp;

    for (i = 0; i < len; i++) {
        b = in[i];

        if (update_only) {
            update_keys(ctx, b);
        } else {
            /* encrypt next byte */
            tmp = decrypt_byte(ctx);
            update_keys(ctx, b);
            b ^= tmp;
        }

        /* store cleartext */
        if (out) {
            out[i] = b;
        }
    }
}


void
decrypt(zip_trad_pkware_t *ctx, zip_uint8_t *out, const zip_uint8_t *in, zip_uint64_t len, int update_only) {
    zip_uint64_t i;
    Bytef b;
    Bytef tmp;

    for (i = 0; i < len; i++) {
        b = in[i];

        if (!update_only) {
            /* decrypt next byte */
            tmp = decrypt_byte(ctx);
            b ^= tmp;
        }

        /* store cleartext */
        if (out) {
            out[i] = b;
        }

        update_keys(ctx, b);
    }
}


void
_zip_pkware_free(zip_trad_pkware_t *ctx) {
    if (ctx == NULL) {
        return;
    }

    free(ctx);
}
