/*
   SipHash reference C implementation

   Copyright (c) 2012-2021 Jean-Philippe Aumasson
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

#include "zipint.h"

/**
 * Computes a SipHash value.
 *
 * @param data The data to hash.
 * @param length The number of bytes to hash.
 * @param key The 16-byte key to use for hashing.
 * @return The computed SipHash value.
 */
zip_uint64_t siphash(const zip_uint8_t *data, size_t length, const zip_uint8_t *key);
