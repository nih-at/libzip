/*
   SipHash reference C implementation

   Copyright (c) 2012-2022 Jean-Philippe Aumasson
   <jeanphilippe.aumasson@gmail.com>
   Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with
   this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "siphash.h"

#include <string.h>

/* default: SipHash-2-4 */
#ifndef cROUNDS
#define cROUNDS 2
#endif
#ifndef dROUNDS
#define dROUNDS 4
#endif

#define ROTL(x, b) (zip_uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                \
    (p)[0] = (zip_uint8_t)((v));       \
    (p)[1] = (zip_uint8_t)((v) >> 8);  \
    (p)[2] = (zip_uint8_t)((v) >> 16); \
    (p)[3] = (zip_uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                  \
    U32TO8_LE((p), (zip_uint32_t)((v))); \
    U32TO8_LE((p) + 4, (zip_uint32_t)((v) >> 32));

#define U8TO64_LE(p) (((zip_uint64_t)((p)[0])) | ((zip_uint64_t)((p)[1]) << 8) | ((zip_uint64_t)((p)[2]) << 16) | ((zip_uint64_t)((p)[3]) << 24) | ((zip_uint64_t)((p)[4]) << 32) | ((zip_uint64_t)((p)[5]) << 40) | ((zip_uint64_t)((p)[6]) << 48) | ((zip_uint64_t)((p)[7]) << 56))

#define SIPROUND           \
    do {                   \
        v0 += v1;          \
        v1 = ROTL(v1, 13); \
        v1 ^= v0;          \
        v0 = ROTL(v0, 32); \
        v2 += v3;          \
        v3 = ROTL(v3, 16); \
        v3 ^= v2;          \
        v0 += v3;          \
        v3 = ROTL(v3, 21); \
        v3 ^= v0;          \
        v2 += v1;          \
        v1 = ROTL(v1, 17); \
        v1 ^= v2;          \
        v2 = ROTL(v2, 32); \
    } while (0)

/*
    Computes a SipHash value
*/
zip_uint64_t siphash(const zip_uint8_t *data, const zip_uint8_t *key) {
    const zip_uint8_t *ni = data;
    const zip_uint8_t *kk = key;
    size_t inlen = strlen((const char *)data);
    zip_uint64_t out;

    zip_uint64_t v0 = UINT64_C(0x736f6d6570736575);
    zip_uint64_t v1 = UINT64_C(0x646f72616e646f6d);
    zip_uint64_t v2 = UINT64_C(0x6c7967656e657261);
    zip_uint64_t v3 = UINT64_C(0x7465646279746573);
    zip_uint64_t k0 = U8TO64_LE(kk);
    zip_uint64_t k1 = U8TO64_LE(kk + 8);
    zip_uint64_t m;
    int i;
    const unsigned char *end = ni + inlen - (inlen % sizeof(zip_uint64_t));
    const int left = inlen & 7;
    zip_uint64_t b = ((zip_uint64_t)inlen) << 56;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    for (; ni != end; ni += 8) {
        m = U8TO64_LE(ni);
        v3 ^= m;

        for (i = 0; i < cROUNDS; ++i) {
            SIPROUND;
        }

        v0 ^= m;
    }

    switch (left) {
    case 7:
        b |= ((zip_uint64_t)ni[6]) << 48;
        /* FALLTHRU */
    case 6:
        b |= ((zip_uint64_t)ni[5]) << 40;
        /* FALLTHRU */
    case 5:
        b |= ((zip_uint64_t)ni[4]) << 32;
        /* FALLTHRU */
    case 4:
        b |= ((zip_uint64_t)ni[3]) << 24;
        /* FALLTHRU */
    case 3:
        b |= ((zip_uint64_t)ni[2]) << 16;
        /* FALLTHRU */
    case 2:
        b |= ((zip_uint64_t)ni[1]) << 8;
        /* FALLTHRU */
    case 1:
        b |= ((zip_uint64_t)ni[0]);
        break;
    case 0:
        break;
    }

    v3 ^= b;

    for (i = 0; i < cROUNDS; ++i) {
        SIPROUND;
    }

    v0 ^= b;

    v2 ^= 0xff;

    for (i = 0; i < dROUNDS; ++i) {
        SIPROUND;
    }

    b = v0 ^ v1 ^ v2 ^ v3;
    U64TO8_LE((zip_uint8_t *)&out, b);

    return out;
}
