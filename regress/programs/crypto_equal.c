/*
 crypto_equal.c -- verify internal crypto buffer comparison
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

#include "cmocka_utilities.h"

#include "zipint.h"

struct test_data {
    size_t length;
};

typedef struct test_data test_data_t;

const test_data_t test_data[] = {{0}, {1}, {2}, {10}, {20}, {31}, {32}, {64}};
size_t test_data_count = sizeof(test_data) / sizeof(test_data[0]);

void run_test(void **state) {
    const test_data_t *test = (const test_data_t *)*state;
    zip_uint8_t expected[64] = {0};
    zip_uint8_t *left, *right;
    size_t i, j;
    unsigned int bit;

    if (test->length == 0) {
        assert_true(_zip_crypto_equal(NULL, NULL, 0));
        assert_true(_zip_crypto_equal(expected, NULL, 0));
        return;
    }

    left = (zip_uint8_t *)malloc(test->length);
    right = (zip_uint8_t *)malloc(test->length);
    assert_non_null(left);
    assert_non_null(right);

    for (i = 0; i < test->length; i++) {
        expected[i] = (zip_uint8_t)(0x80 + 37 * i);
    }
    memcpy(left, expected, test->length);
    memcpy(right, expected, test->length);

    assert_true(_zip_crypto_equal(left, right, test->length));
    assert_true(_zip_crypto_equal(left, left, test->length));

    for (i = 0; i < test->length; i++) {
        for (bit = 0; bit < 8; bit++) {
            right[i] ^= (zip_uint8_t)(1U << bit);
            assert_false(_zip_crypto_equal(left, right, test->length));
            assert_false(_zip_crypto_equal(right, left, test->length));
            right[i] ^= (zip_uint8_t)(1U << bit);
        }
    }
    assert_memory_equal(left, expected, test->length);
    assert_memory_equal(right, expected, test->length);

    if (test->length == 1) {
        for (i = 0; i < 256; i++) {
            for (j = 0; j < 256; j++) {
                left[0] = (zip_uint8_t)i;
                right[0] = (zip_uint8_t)j;
                assert_int_equal(_zip_crypto_equal(left, right, 1), i == j);
            }
        }
    }

    free(left);
    free(right);
}

int main(void) {
    return run_test_table("crypto_equal", run_test, (void *)test_data, sizeof(test_data[0]), test_data_count);
}
