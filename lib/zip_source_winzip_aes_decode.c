/*
  zip_source_winzip_aes_decode.c -- Winzip AES decryption routines
  Copyright (C) 2009-2024 Dieter Baron and Thomas Klausner

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


#include <stdlib.h>
#include <string.h>

#include "zipint.h"

#include "zip_crypto.h"

struct winzip_aes {
    char *password;
    zip_uint16_t encryption_method;

    zip_uint64_t header_length; /* length of salt + password verification value, precedes the ciphertext */
    zip_uint64_t data_length;
    zip_uint64_t current_position;

    /* The HMAC can only be computed over the ciphertext in order, starting
       from the beginning. hmac_position tracks how much of it has been fed
       to the HMAC so far, contiguously from 0; if a seek ever creates a gap
       that never gets filled in, verification is silently skipped instead
       of failing, exactly as zip_source_crc.c does for the CRC. */
    zip_uint64_t hmac_position;

    zip_winzip_aes_t *aes_ctx;
    bool hmac_verify_failed;
    bool hmac_verified;
    zip_error_t error;
};


static int decrypt_header(zip_source_t *src, struct winzip_aes *ctx);
static void winzip_aes_free(struct winzip_aes *);
static zip_int64_t winzip_aes_decrypt(zip_source_t *src, void *ud, void *data, zip_uint64_t len, zip_source_cmd_t cmd);
static struct winzip_aes *winzip_aes_new(zip_uint16_t encryption_method, const char *password, zip_error_t *error);


zip_source_t *zip_source_winzip_aes_decode(zip_t *za, zip_source_t *src, zip_uint16_t encryption_method, int flags, const char *password) {
    zip_source_t *s2;
    zip_stat_t st;
    zip_uint64_t aux_length;
    struct winzip_aes *ctx;

    if ((encryption_method != ZIP_EM_AES_128 && encryption_method != ZIP_EM_AES_192 && encryption_method != ZIP_EM_AES_256) || password == NULL || src == NULL) {
        zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return NULL;
    }
    if (flags & ZIP_CODEC_ENCODE) {
        zip_error_set(&za->error, ZIP_ER_ENCRNOTSUPP, 0);
        return NULL;
    }

    if (zip_source_stat(src, &st) != 0) {
        zip_error_set_from_source(&za->error, src);
        return NULL;
    }

    aux_length = WINZIP_AES_PASSWORD_VERIFY_LENGTH + SALT_LENGTH(encryption_method) + HMAC_LENGTH;

    if ((st.valid & ZIP_STAT_COMP_SIZE) == 0 || st.comp_size < aux_length) {
        zip_error_set(&za->error, ZIP_ER_OPNOTSUPP, 0);
        return NULL;
    }

    if ((ctx = winzip_aes_new(encryption_method, password, &za->error)) == NULL) {
        return NULL;
    }

    ctx->data_length = st.comp_size - aux_length;

    if ((s2 = zip_source_layered(za, src, winzip_aes_decrypt, ctx)) == NULL) {
        winzip_aes_free(ctx);
        return NULL;
    }

    return s2;
}


static int decrypt_header(zip_source_t *src, struct winzip_aes *ctx) {
    zip_uint8_t header[WINZIP_AES_MAX_HEADER_LENGTH];
    zip_uint8_t password_verification[WINZIP_AES_PASSWORD_VERIFY_LENGTH];
    unsigned int headerlen;
    zip_int64_t n;

    headerlen = WINZIP_AES_PASSWORD_VERIFY_LENGTH + SALT_LENGTH(ctx->encryption_method);
    if ((n = zip_source_read(src, header, headerlen)) < 0) {
        zip_error_set_from_source(&ctx->error, src);
        return -1;
    }

    if (n != headerlen) {
        zip_error_set(&ctx->error, ZIP_ER_EOF, 0);
        return -1;
    }

    /* On ZIP_SOURCE_SUPPORTS_REOPEN, this runs again on a context that may
       still hold a crypto context from the previous open (it's no longer
       freed right after a successful verify, so seeking after that still
       works); free it first to avoid leaking it. */
    _zip_winzip_aes_free(ctx->aes_ctx);

    if ((ctx->aes_ctx = _zip_winzip_aes_new((zip_uint8_t *)ctx->password, strlen(ctx->password), header, ctx->encryption_method, password_verification, &ctx->error)) == NULL) {
        return -1;
    }
    if (memcmp(password_verification, header + SALT_LENGTH(ctx->encryption_method), WINZIP_AES_PASSWORD_VERIFY_LENGTH) != 0) {
        _zip_winzip_aes_free(ctx->aes_ctx);
        ctx->aes_ctx = NULL;
        zip_error_set(&ctx->error, ZIP_ER_WRONGPASSWD, 0);
        return -1;
    }
    ctx->header_length = headerlen;
    return 0;
}


static void verify_hmac(zip_source_t *src, struct winzip_aes *ctx) {
    unsigned char computed[ZIP_CRYPTO_SHA1_LENGTH], from_file[HMAC_LENGTH];

    if (ctx->hmac_verified) {
        return;
    }
    ctx->hmac_verified = true;

    if (zip_source_read(src, from_file, HMAC_LENGTH) < HMAC_LENGTH) {
        zip_error_set_from_source(&ctx->error, src);
        ctx->hmac_verify_failed = true;
        return;
    }

    if (ctx->hmac_position != ctx->data_length) {
        /* The ciphertext wasn't read contiguously from the start (the caller
           seeked around), so there's no way to compute the HMAC over the
           whole file. Silently skip verification rather than fail, exactly
           as zip_source_crc.c does for the CRC in the same situation. */
        return;
    }

    if (!_zip_winzip_aes_finish(ctx->aes_ctx, computed)) {
        zip_error_set(&ctx->error, ZIP_ER_INTERNAL, 0);
        ctx->hmac_verify_failed = true;
        return;
    }

    /* ctx->aes_ctx is kept alive (freed only in winzip_aes_free()), since the
       source may still be seeked and read from after this point; only the
       HMAC inside it is now spent and must never be fed more data, which
       ctx->hmac_verified guards against in the ZIP_SOURCE_READ case. */

    if (memcmp(from_file, computed, HMAC_LENGTH) != 0) {
        zip_error_set(&ctx->error, ZIP_ER_CRC, 0);
        ctx->hmac_verify_failed = true;
        return;
    }

    return;
}


static zip_int64_t winzip_aes_decrypt(zip_source_t *src, void *ud, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    struct winzip_aes *ctx;
    zip_int64_t n;

    ctx = (struct winzip_aes *)ud;

    switch (cmd) {
    case ZIP_SOURCE_AT_EOF:
        return ctx->current_position == ctx->data_length;

    case ZIP_SOURCE_OPEN:
        ctx->hmac_verify_failed = false;
        ctx->hmac_verified = false;
        ctx->hmac_position = 0;
        if (decrypt_header(src, ctx) < 0) {
            return -1;
        }
        ctx->current_position = 0;
        return 0;

    case ZIP_SOURCE_READ:
        len = ZIP_MIN(len, ctx->data_length - ctx->current_position);

        if (len > 0) {
            zip_uint64_t hmac_skip;

            if ((n = zip_source_read(src, data, len)) < 0) {
                zip_error_set_from_source(&ctx->error, src);
                return -1;
            }

            if (n == 0) {
                zip_error_set(&ctx->error, ZIP_ER_EOF, 0);
                return -1;
            }

            /* Feed only the part of this read that's contiguous with what's
               already been authenticated, exactly as zip_source_crc.c does
               for the CRC; the rest is still decrypted, just not verified.
               Once verify_hmac() has run once, the HMAC is spent (finalized
               or given up on) and must never be fed again, regardless of
               position - hence the hmac_verified check. */
            if (!ctx->hmac_verified && ctx->current_position <= ctx->hmac_position && ctx->hmac_position < ctx->current_position + (zip_uint64_t)n) {
                hmac_skip = ctx->hmac_position - ctx->current_position;
                ctx->hmac_position = ctx->current_position + (zip_uint64_t)n;
            }
            else {
                hmac_skip = (zip_uint64_t)n;
            }

            if (!_zip_winzip_aes_decrypt(ctx->aes_ctx, (zip_uint8_t *)data, (zip_uint64_t)n, hmac_skip)) {
                zip_error_set(&ctx->error, ZIP_ER_INTERNAL, 0);
                return -1;
            }

            ctx->current_position += (zip_uint64_t)n;
        }
        else {
            n = 0;
        }

        if (ctx->current_position == ctx->data_length) {
            verify_hmac(src, ctx);
        }

        if (n > 0) {
            return n;
        }
        else {
            return ctx->hmac_verify_failed ? -1 : 0;
        }

    case ZIP_SOURCE_CLOSE:
        /* For empty files, we should verify the HMAC even if no data has been read. */
        if (ctx->current_position == ctx->data_length) {
            verify_hmac(src, ctx);
        }
        return ctx->hmac_verify_failed ? -1 : 0;

    case ZIP_SOURCE_STAT: {
        zip_stat_t *st;

        st = (zip_stat_t *)data;

        st->encryption_method = ZIP_EM_NONE;
        st->valid |= ZIP_STAT_ENCRYPTION_METHOD;
        if (st->valid & ZIP_STAT_COMP_SIZE) {
            if (st->comp_size < WINZIP_AES_PASSWORD_VERIFY_LENGTH + SALT_LENGTH(ctx->encryption_method) + HMAC_LENGTH) {
                zip_error_set(&ctx->error, ZIP_ER_DATA_LENGTH, 0);
                return -1;
            }
            st->comp_size -= WINZIP_AES_PASSWORD_VERIFY_LENGTH + SALT_LENGTH(ctx->encryption_method) + HMAC_LENGTH;
        }

        return 0;
    }

    case ZIP_SOURCE_SEEK: {
        zip_int64_t new_position = zip_source_seek_compute_offset(ctx->current_position, ctx->data_length, data, len, &ctx->error);

        if (new_position < 0) {
            return -1;
        }

        if (zip_source_seek(src, (zip_int64_t)(ctx->header_length + (zip_uint64_t)new_position), SEEK_SET) < 0 || !_zip_winzip_aes_seek(ctx->aes_ctx, (zip_uint64_t)new_position)) {
            zip_error_set_from_source(&ctx->error, src);
            return -1;
        }

        ctx->current_position = (zip_uint64_t)new_position;

        return 0;
    }

    case ZIP_SOURCE_TELL:
        return (zip_int64_t)ctx->current_position;

    case ZIP_SOURCE_SUPPORTS:
        return zip_source_make_command_bitmap(ZIP_SOURCE_AT_EOF, ZIP_SOURCE_OPEN, ZIP_SOURCE_READ, ZIP_SOURCE_CLOSE, ZIP_SOURCE_STAT, ZIP_SOURCE_ERROR, ZIP_SOURCE_FREE, ZIP_SOURCE_SUPPORTS_REOPEN, ZIP_SOURCE_SEEK, ZIP_SOURCE_TELL, -1);

    case ZIP_SOURCE_ERROR:
        return zip_error_to_data(&ctx->error, data, len);

    case ZIP_SOURCE_FREE:
        winzip_aes_free(ctx);
        return 0;

    default:
        return zip_source_pass_to_lower_layer(src, data, len, cmd);
    }
}


static void winzip_aes_free(struct winzip_aes *ctx) {
    if (ctx == NULL) {
        return;
    }

    _zip_crypto_clear(ctx->password, strlen(ctx->password));
    free(ctx->password);
    zip_error_fini(&ctx->error);
    _zip_winzip_aes_free(ctx->aes_ctx);
    free(ctx);
}


static struct winzip_aes *winzip_aes_new(zip_uint16_t encryption_method, const char *password, zip_error_t *error) {
    struct winzip_aes *ctx;

    if ((ctx = (struct winzip_aes *)malloc(sizeof(*ctx))) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    if ((ctx->password = strdup(password)) == NULL) {
        zip_error_set(error, ZIP_ER_MEMORY, 0);
        free(ctx);
        return NULL;
    }

    ctx->encryption_method = encryption_method;
    ctx->header_length = 0;
    ctx->hmac_position = 0;
    ctx->aes_ctx = NULL;
    ctx->hmac_verify_failed = false;
    ctx->hmac_verified = false;

    zip_error_init(&ctx->error);

    return ctx;
}
