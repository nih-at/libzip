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

#include "cmocka_utilities.h"

#include "zipint.h"

#define TEST_DATA_SIZE 6

struct test_data {
    size_t start;
    size_t length;
    zip_uint8_t input[TEST_DATA_SIZE];
    zip_uint8_t output[TEST_DATA_SIZE];
};

typedef struct test_data test_data_t;

/* clang-format off */
const test_data_t test_data[] = {
    {.start = 1, .length = 4, .input = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a}, .output = {0xa5, 0, 0, 0, 0, 0x5a}},
    {.start = 2, .length = 0, .input = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a}, .output = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a}}, 
    {.start = 0, .length = 6, .input = {0xa5, 0x11, 0x22, 0x33, 0x44, 0x5a}, .output = {0, 0, 0, 0, 0, 0}}};
/* clang-format on */

size_t test_data_count = sizeof(test_data) / sizeof(test_data[0]);

void run_test(void **state) {
    test_data_t *test = (test_data_t *)*state;
    zip_uint8_t buffer[TEST_DATA_SIZE];

    memcpy(buffer, test->input, TEST_DATA_SIZE);
    _zip_crypto_clear(buffer + test->start, test->length);

    assert_memory_equal(buffer, test->output, TEST_DATA_SIZE);
}

int main(void) {
    return run_test_table("crypto_clear", run_test, (void *)test_data, sizeof(test_data[0]), test_data_count);
}
