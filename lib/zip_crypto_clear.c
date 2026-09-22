/*
  zip_crypto_clear.c -- clear sensitive internal buffers
  SPDX-License-Identifier: BSD-3-Clause

  This file is part of libzip, a library to manipulate ZIP archives.
*/

#include "zipint.h"

#include <string.h>


void _zip_crypto_clear(void *buffer, size_t length) {
#ifdef HAVE_EXPLICIT_MEMSET
    explicit_memset(buffer, 0, length);
#elif defined(HAVE_EXPLICIT_BZERO)
    explicit_bzero(buffer, length);
#else
    volatile unsigned char *p = (volatile unsigned char *)buffer;

    /* Ordinary memset stores may be removed when the buffer is no longer used. */
    while (length > 0) {
        *p++ = 0;
        length--;
    }
#endif
}
