/*
  zip_winzip_aes.c -- Winzip AES de/encryption backend routines
  Copyright (C) 2017 Dieter Baron and Thomas Klausner

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

#include "zipint.h"

#include <limits.h>
#include <string.h>

#ifdef HAVE_OPENSSL
#include <openssl/aes.h>
#include <openssl/hmac.h>
#define HMAC_Init_SHA1(ctx, secret, secret_length)  (HMAC_Init((ctx), (secret), (secret_length), EVP_sha1()))
#define HMAC_MAX_LENGTH INT_MAX
#elif defined(HAVE_GNUTLS)
#define AES_set_encrypt_key(key, size, ctx) XXX
#define HMAC_MAX_LENGTH SIZE_MAX
#define HMAC_CTX_init(ctx) /* nothing */
#define HMAC_Init_SHA1(ctx, secret, secret_length)  XXX
#define HMAC_CTX XXX
#define AES_KEY XXX
#define PKCS5_PBKDF2_HMAC_SHA1(password, password_length, salt, salt_length, iterations, out_size, out) XXX
#else
#error
#endif

#if HMAC_MAX_LENGTH < ZIP_UINT64_MAX
static void
hmac(HMAC_CTX *ctx, const zip_uint8_t *data, zip_uint64_t length)
{
    zip_uint64_t n;
    
    while (length > 0) {
	n = ZIP_MIN(length, HMAC_MAX_LENGTH);
	
	HMAC_Update(ctx, data, n);
	length -= n;
    }
}
#else
#define hmac(ctx, data, length) (HMAC_Update((ctx), (data), (length)))
#endif

#define MAX_KEY_LENGTH 256
#define PBKDF2_ITERATIONS 1000

struct _zip_winzip_aes {
    AES_KEY aes_key;
    HMAC_CTX hmac_ctx;
    zip_uint8_t counter[AES_BLOCK_SIZE];
    zip_uint8_t pad[AES_BLOCK_SIZE];
    int pad_offset;
};

static void
aes_crypt(zip_winzip_aes_t *ctx, zip_uint8_t *data, zip_uint64_t length)
{
    zip_uint64_t i, j;

    for (i=0; i < length; i++) {
	if (ctx->pad_offset == AES_BLOCK_SIZE) {
	    for (j = 0; j < 8; j++) {
		ctx->counter[j]++;
		if (ctx->counter[j] != 0) {
		    break;
		}
	    }
	    AES_encrypt(ctx->counter, ctx->pad, &ctx->aes_key);
	    ctx->pad_offset = 0;
	}
	data[i] ^= ctx->pad[ctx->pad_offset++];
    }
    
}


zip_winzip_aes_t *
_zip_winzip_aes_new(const zip_uint8_t *password, zip_uint64_t password_length, const zip_uint8_t *salt, zip_uint16_t encryption_method, zip_uint8_t *password_verify, zip_error_t *error)
{
    zip_winzip_aes_t *ctx;
    zip_uint8_t buffer[2 * (MAX_KEY_LENGTH / 8) + WINZIP_AES_PASSWORD_VERIFY_LENGTH];
    zip_uint16_t key_size = 0;

    switch (encryption_method) {
    case ZIP_EM_AES_128:
	key_size = 128;
	break;
    case ZIP_EM_AES_192:
	key_size = 192;
	break;
    case ZIP_EM_AES_256:
	key_size = 256;
	break;
    }

    if (key_size == 0 || salt == NULL || password == NULL || password_length == 0) {
	zip_error_set(error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    if ((ctx = (zip_winzip_aes_t *)malloc(sizeof(*ctx))) == NULL) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    memset(ctx->counter, 0, AES_BLOCK_SIZE);
    ctx->pad_offset = AES_BLOCK_SIZE;
    
    /* TODO: error checks */
    PKCS5_PBKDF2_HMAC_SHA1(password, password_length, salt, key_size / 8 / 2, PBKDF2_ITERATIONS, 2 * (key_size / 8) + WINZIP_AES_PASSWORD_VERIFY_LENGTH, buffer);
    AES_set_encrypt_key(buffer, key_size, &ctx->aes_key);
    HMAC_CTX_init(&ctx->hmac_ctx);
    HMAC_Init_SHA1(&ctx->hmac_ctx, buffer + (key_size / 8), (key_size / 8));

    if (password_verify) {
	memcpy(password_verify, buffer + (2 * key_size / 8), WINZIP_AES_PASSWORD_VERIFY_LENGTH);
    }

    return ctx;
}


void
_zip_winzip_aes_encrypt(zip_winzip_aes_t *ctx, zip_uint8_t *data, zip_uint64_t length)
{
    aes_crypt(ctx, data, length);
    hmac(&ctx->hmac_ctx, data, length);
}


void
_zip_winzip_aes_decrypt(zip_winzip_aes_t *ctx, zip_uint8_t *data, zip_uint64_t length)
{
    hmac(&ctx->hmac_ctx, data, length);
    aes_crypt(ctx, data, length);
}


void
_zip_winzip_aes_finish(zip_winzip_aes_t *ctx, zip_uint8_t *hmac)
{
    unsigned int len;
    HMAC_Final(&ctx->hmac_ctx, hmac, &len);
}


void
_zip_winzip_aes_free(zip_winzip_aes_t *ctx)
{
    if (ctx == NULL) {
	return;
    }
    HMAC_CTX_cleanup(&ctx->hmac_ctx);
    _zip_crypto_clear(ctx, sizeof(*ctx));
    free(ctx);
}
