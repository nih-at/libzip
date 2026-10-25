/*
  progress_reentrant.c -- test (un)registering callbacks from callbacks and ud_free
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

/* Each test case checks that every userdata is freed exactly once. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

struct cleanup_state {
    unsigned *count;
};

static unsigned progress_calls;
static unsigned replacement_calls;
static unsigned cancel_calls;
static unsigned progress_frees;
static unsigned replacement_frees;
static unsigned cancel_frees;
static int replacement_failed;
static unsigned unregister_on_call;
static zip_t *current_archive;
static int verbose;

static struct cleanup_state *new_cleanup_state(unsigned *count) {
    struct cleanup_state *state = malloc(sizeof(*state));

    if (state != NULL) {
        state->count = count;
    }
    return state;
}

static void cleanup_state(void *userdata) {
    struct cleanup_state *state = userdata;

    if (state != NULL) {
        (*state->count)++;
        free(state);
    }
}

static void count_progress(zip_t *archive, double progress, void *userdata) {
    (void)archive;
    (void)progress;
    (void)userdata;
    progress_calls++;
}

static void self_unregister_progress(zip_t *archive, double progress, void *userdata) {
    (void)progress;
    (void)userdata;

    progress_calls++;
    if (progress_calls == unregister_on_call) {
        (void)zip_register_progress_callback_with_state(archive, 0.0, NULL, NULL, NULL);
    }
}

/* ud_free functions that unregister their own callback. */
static void cleanup_unregister_progress(void *userdata) {
    (void)zip_register_progress_callback_with_state(current_archive, 0.0, NULL, NULL, NULL);
    cleanup_state(userdata);
}

static void cleanup_unregister_cancel(void *userdata) {
    (void)zip_register_cancel_callback_with_state(current_archive, NULL, NULL, NULL);
    cleanup_state(userdata);
}

static void replacement_progress(zip_t *archive, double progress, void *userdata) {
    (void)archive;
    (void)progress;
    (void)userdata;
    replacement_calls++;
}

static void replace_progress(zip_t *archive, double progress, void *userdata) {
    struct cleanup_state *replacement_state;

    (void)progress;
    (void)userdata;
    progress_calls++;
    if (progress_calls != 2) {
        return;
    }

    replacement_state = new_cleanup_state(&replacement_frees);
    if (replacement_state == NULL || zip_register_progress_callback_with_state(archive, 0.0, replacement_progress, cleanup_state, replacement_state) < 0) {
        free(replacement_state);
        replacement_failed = 1;
    }
}

static int count_cancel(zip_t *archive, void *userdata) {
    (void)archive;
    (void)userdata;
    cancel_calls++;
    return 0;
}

static int self_unregister_cancel(zip_t *archive, void *userdata) {
    (void)userdata;
    cancel_calls++;
    if (cancel_calls == 1) {
        (void)zip_register_cancel_callback_with_state(archive, NULL, NULL, NULL);
    }
    return 0;
}

static int run_case_with_cleanup(zip_progress_callback progress_callback, void (*progress_free)(void *), zip_cancel_callback cancel_callback, void (*cancel_free)(void *)) {
    static const unsigned char data[65537] = {0x41};
    struct cleanup_state *progress_state;
    struct cleanup_state *cancel_state = NULL;
    zip_error_t error;
    zip_source_t *archive_source;
    zip_source_t *file_source;
    zip_t *archive;

    progress_state = new_cleanup_state(&progress_frees);
    if (progress_state == NULL) {
        return -1;
    }
    if (cancel_callback != NULL) {
        cancel_state = new_cleanup_state(&cancel_frees);
        if (cancel_state == NULL) {
            free(progress_state);
            return -1;
        }
    }

    zip_error_init(&error);
    archive_source = zip_source_buffer_create(NULL, 0, 0, &error);
    if (archive_source == NULL) {
        free(progress_state);
        free(cancel_state);
        zip_error_fini(&error);
        return -1;
    }
    archive = zip_open_from_source(archive_source, ZIP_TRUNCATE, &error);
    if (archive == NULL) {
        zip_source_free(archive_source);
        free(progress_state);
        free(cancel_state);
        zip_error_fini(&error);
        return -1;
    }
    zip_error_fini(&error);
    current_archive = archive;

    file_source = zip_source_buffer(archive, data, sizeof(data), 0);
    if (file_source == NULL) {
        free(progress_state);
        free(cancel_state);
        zip_discard(archive);
        return -1;
    }
    if (zip_file_add(archive, "entry", file_source, ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(file_source);
        free(progress_state);
        free(cancel_state);
        zip_discard(archive);
        return -1;
    }
    if (zip_set_file_compression(archive, 0, ZIP_CM_STORE, 0) < 0 || zip_register_progress_callback_with_state(archive, 0.0, progress_callback, progress_free, progress_state) < 0) {
        free(progress_state);
        free(cancel_state);
        zip_discard(archive);
        return -1;
    }
    if (cancel_callback != NULL && zip_register_cancel_callback_with_state(archive, cancel_callback, cancel_free, cancel_state) < 0) {
        free(cancel_state);
        zip_discard(archive);
        return -1;
    }
    if (zip_close(archive) < 0) {
        zip_discard(archive);
        return -1;
    }
    return 0;
}

static int run_case(zip_progress_callback progress_callback, zip_cancel_callback cancel_callback) {
    return run_case_with_cleanup(progress_callback, cleanup_state, cancel_callback, cleanup_state);
}

static void reset_counts(void) {
    progress_calls = 0;
    replacement_calls = 0;
    cancel_calls = 0;
    progress_frees = 0;
    replacement_frees = 0;
    cancel_frees = 0;
    replacement_failed = 0;
    unregister_on_call = 2;
}

static int check(const char *name, int ret, int ok) {
    if (ret < 0 || !ok) {
        if (verbose) {
            printf("%s: failed: zip_close %s, progress calls %u, replacement calls %u, cancel calls %u, progress frees %u, replacement frees %u, cancel frees %u\n", name, ret < 0 ? "failed" : "succeeded", progress_calls, replacement_calls, cancel_calls, progress_frees, replacement_frees, cancel_frees);
        }
        return 1;
    }
    if (verbose) {
        printf("%s: ok\n", name);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int fail = 0;
    int ret;

    if (argc == 2 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    else if (argc != 1) {
        fprintf(stderr, "usage: %s [-v]\n", argv[0]);
        return 1;
    }

    reset_counts();
    ret = run_case(self_unregister_progress, NULL);
    fail += check("progress callback unregisters itself", ret, progress_calls == 2 && progress_frees == 1);

    reset_counts();
    unregister_on_call = 1;
    ret = run_case(self_unregister_progress, NULL);
    fail += check("progress callback unregisters itself on first call", ret, progress_calls == 1 && progress_frees == 1);

    reset_counts();
    ret = run_case(replace_progress, NULL);
    fail += check("progress callback replaces itself", ret, !replacement_failed && progress_calls == 2 && replacement_calls > 0 && progress_frees == 1 && replacement_frees == 1);

    reset_counts();
    ret = run_case(self_unregister_progress, count_cancel);
    fail += check("progress callback unregisters itself, cancel callback set", ret, progress_calls == 2 && cancel_calls > 0 && progress_frees == 1 && cancel_frees == 1);

    reset_counts();
    ret = run_case(count_progress, self_unregister_cancel);
    fail += check("cancel callback unregisters itself", ret, progress_calls > 0 && cancel_calls == 1 && progress_frees == 1 && cancel_frees == 1);

    reset_counts();
    ret = run_case_with_cleanup(count_progress, cleanup_unregister_progress, NULL, NULL);
    fail += check("progress ud_free unregisters progress callback", ret, progress_calls > 0 && progress_frees == 1);

    reset_counts();
    ret = run_case_with_cleanup(count_progress, cleanup_state, count_cancel, cleanup_unregister_cancel);
    fail += check("cancel ud_free unregisters cancel callback", ret, progress_calls > 0 && cancel_calls > 0 && progress_frees == 1 && cancel_frees == 1);

    return fail ? 1 : 0;
}
