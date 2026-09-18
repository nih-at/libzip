/*
  allocate.c -- regress checked allocation size arithmetic
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
#include <stdlib.h>

#include "zipint.h"

static int failed = 0;

static void check_size(const char *name, zip_uint64_t nmemb, zip_uint64_t element_size, zip_uint64_t extra_bytes, bool expected_ok, size_t expected_size) {
    size_t size = 0;
    zip_error_t error;
    bool ok;

    zip_error_init(&error);
    ok = _zip_size_of_array(nmemb, element_size, extra_bytes, &size, &error);

    if (ok != expected_ok) {
        fprintf(stderr, "%s: expected %s, got %s\n", name, expected_ok ? "success" : "failure", ok ? "success" : "failure");
        failed = 1;
    }
    else if (ok && size != expected_size) {
        fprintf(stderr, "%s: expected size %llu, got %llu\n", name, (unsigned long long)expected_size, (unsigned long long)size);
        failed = 1;
    }
    else if (!ok && zip_error_code_zip(&error) != ZIP_ER_MEMORY) {
        fprintf(stderr, "%s: expected error %d, got %d\n", name, ZIP_ER_MEMORY, zip_error_code_zip(&error));
        failed = 1;
    }

    zip_error_fini(&error);
}


static void test_size_of_array(void) {
    size_t max_size = SIZE_MAX;

    check_size("empty", 0, 0, 0, true, 0);
    check_size("array", 10, 4, 0, true, 40);
    check_size("array with extra bytes", 10, 4, 7, true, 47);
    /* element_size 0 must not divide by zero */
    check_size("zero sized elements", 5, 0, 17, true, 17);
    check_size("maximum size", max_size, 1, 0, true, max_size);
    check_size("maximum size in bytes", 1, max_size, 0, true, max_size);
    check_size("maximum size in extra bytes", 0, 0, max_size, true, max_size);

    check_size("one byte too large", max_size, 1, 1, false, 0);
    check_size("product too large", max_size / 2 + 1, 2, 0, false, 0);
    check_size("element too large", 2, max_size / 2 + 1, 0, false, 0);
    check_size("extra bytes too large", 1, 1, max_size, false, 0);
    /* the computation itself must not overflow */
    check_size("unsigned 64 bit overflow", ZIP_UINT64_MAX, ZIP_UINT64_MAX, 0, false, 0);
    check_size("unsigned 64 bit wrap to small value", ZIP_UINT64_MAX / 4 + 1, 4, 0, false, 0);
}


static void test_allocate(void) {
    zip_error_t error;
    zip_uint8_t *memory;

    zip_error_init(&error);

    if ((memory = (zip_uint8_t *)_zip_allocate(4, 8, 1, &error)) == NULL) {
        fprintf(stderr, "allocate: can't allocate 33 bytes: %s\n", zip_error_strerror(&error));
        failed = 1;
    }
    else {
        memory[32] = '\0';
        free(memory);
    }

    if ((memory = (zip_uint8_t *)_zip_allocate(SIZE_MAX / 2 + 1, 2, 0, &error)) != NULL) {
        fprintf(stderr, "allocate: overflowing allocation succeeded\n");
        free(memory);
        failed = 1;
    }
    else if (zip_error_code_zip(&error) != ZIP_ER_MEMORY) {
        fprintf(stderr, "allocate: expected error %d, got %d\n", ZIP_ER_MEMORY, zip_error_code_zip(&error));
        failed = 1;
    }

    /* _zip_allocate must work without an error to report to */
    if ((memory = (zip_uint8_t *)_zip_allocate(SIZE_MAX, 2, 0, NULL)) != NULL) {
        fprintf(stderr, "allocate: overflowing allocation without error succeeded\n");
        free(memory);
        failed = 1;
    }

    zip_error_fini(&error);
}


static void test_realloc(void) {
    zip_error_t error;
    void *memory = NULL;
    zip_uint64_t alloced_elements = 0;

    zip_error_init(&error);

    if (!zip_realloc(&memory, &alloced_elements, sizeof(zip_uint32_t), 10, &error)) {
        fprintf(stderr, "realloc: can't allocate 10 elements: %s\n", zip_error_strerror(&error));
        failed = 1;
    }
    else if (alloced_elements != 10) {
        fprintf(stderr, "realloc: expected 10 elements, got %llu\n", (unsigned long long)alloced_elements);
        failed = 1;
    }

    if (zip_realloc(&memory, &alloced_elements, sizeof(zip_uint32_t), ZIP_UINT64_MAX / 2, &error)) {
        fprintf(stderr, "realloc: overflowing reallocation succeeded\n");
        failed = 1;
    }
    else if (zip_error_code_zip(&error) != ZIP_ER_MEMORY) {
        fprintf(stderr, "realloc: expected error %d, got %d\n", ZIP_ER_MEMORY, zip_error_code_zip(&error));
        failed = 1;
    }
    else if (alloced_elements != 10) {
        fprintf(stderr, "realloc: failed reallocation changed the number of elements to %llu\n", (unsigned long long)alloced_elements);
        failed = 1;
    }

    free(memory);

    /* element_size 0 must not divide by zero */
    memory = NULL;
    alloced_elements = 0;
    if (zip_realloc(&memory, &alloced_elements, 0, 5, &error)) {
        free(memory);
    }

    zip_error_fini(&error);
}


int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [ignored]\n", argv[0]);
        return 1;
    }

    test_size_of_array();
    test_allocate();
    test_realloc();

    return failed;
}
