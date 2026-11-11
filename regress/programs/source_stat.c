/*
  source_stat.c -- stat a source after closing an empty archive
  Copyright (C) 2026 astunm

  This file is part of libzip, a library to manipulate ZIP archives.

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

#include <errno.h>

#include "cmocka_utilities.h"
#include "zip.h"

static void run_test(void **state) {
    int flags = *(int *)*state;
    zip_error_t error;
    zip_source_t *source;
    zip_t *archive;
    zip_stat_t st;
    int removed, result, zip_error, system_error;

    zip_error_init(&error);
    source = zip_source_buffer_create(NULL, 0, 0, &error);
    assert_non_null(source);
    archive = zip_open_from_source(source, flags, &error);
    assert_non_null(archive);
    zip_source_keep(source);

    zip_stat_init(&st);
    assert_int_equal(zip_source_stat(source, &st), 0);
    assert_int_equal(zip_close(archive), 0);
    removed = zip_source_is_deleted(source);
    result = zip_source_stat(source, &st);
    zip_error = zip_error_code_zip(zip_source_error(source));
    system_error = zip_error_code_system(zip_source_error(source));
    zip_source_free(source);
    zip_error_fini(&error);

    assert_int_equal(removed, (flags & ZIP_TRUNCATE) != 0);
    assert_int_equal(result, removed ? -1 : 0);
    if (removed) {
        assert_int_equal(zip_error, ZIP_ER_READ);
        assert_int_equal(system_error, ENOENT);
    }
}

int main(void) {
    int flags[] = {ZIP_CREATE, ZIP_CREATE | ZIP_TRUNCATE};

    return run_test_table("source_stat", run_test, flags, sizeof(flags[0]), sizeof(flags) / sizeof(flags[0]));
}
