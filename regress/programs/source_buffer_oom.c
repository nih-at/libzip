/*
  source_buffer_oom.c -- test allocation-failure handling of zip_source_buffer
  Copyright (C) 2026

  This file is part of libzip, a library to manipulate ZIP archives.
*/

/*
 * This program replaces malloc(), calloc(), realloc() and free() with an
 * allocator over a private arena that can fail any chosen allocation on
 * demand.  Wherever the replacements are interposed on the library, every
 * allocation failure point of zip_source_buffer_create() and
 * zip_source_buffer_fragment_create() is failed in turn.
 *
 * If creating the source fails, the caller keeps ownership of the buffers
 * (even when freep is non-zero) and must free them; if it succeeds, the
 * source owns them (freep non-zero) or borrows them (freep zero).  For every
 * failure point the test checks that this cleanup works out: no allocation is
 * left behind and none is freed twice.  Failure paths used to free the
 * caller's buffers themselves, so a caller following the documented cleanup
 * freed them a second time and corrupted the heap.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"


/*
 * A simple free-list allocator over a static arena with coalescing.  It
 * needs no allocation of its own and can serve any number of allocations
 * and frees in any order, so it is safe to use from the replacement
 * allocation functions themselves and from the C library.
 *
 * Every block has a header immediately before its payload; the payload is
 * aligned to the strictest fundamental alignment.  Each block also records
 * whether it was allocated while a test case was running ("scoped"); only
 * scoped blocks take part in the leak check, so one-time lazy allocations
 * made by the C library outside the test cases don't matter.
 */
#define ARENA_SIZE (8 * 1024 * 1024)

union max_align {
    long double ld;
    void *p;
    long long ll;
};

struct blk {
    struct blk *prev, *next;
    size_t size;
    int is_free;
    int scoped;
    unsigned int magic;
};

#define BLK_MAGIC 0x7a697031
#define ALIGN_SIZE (sizeof(union max_align))
#define HDR_SIZE (((sizeof(struct blk) + ALIGN_SIZE - 1) / ALIGN_SIZE) * ALIGN_SIZE)

static union max_align arena[ARENA_SIZE / sizeof(union max_align)];
#define ARENA_BYTES (sizeof(arena))

static struct blk *first_blk, *last_blk;
static size_t arena_used;
static long live_scoped; /* live scoped blocks */
static int arena_error;  /* arena exhaustion or corruption */

/* Allocation failure injection state. */
static int alloc_armed;    /* nonzero while the library call runs */
static long alloc_seq;     /* allocations seen in this run */
static long alloc_fail_at; /* fail this allocation, 0 = fail none */

/* Leak tracking state. */
static int in_case; /* nonzero while a test case runs */


/*
 * Round an allocation size up to the arena alignment; returns nonzero on
 * overflow.
 */
static int round_up_size(size_t n, size_t *out) {
    if (n == 0) {
        n = 1;
    }
    if (n > (size_t)-1 - (ALIGN_SIZE - 1)) {
        return 1;
    }
    *out = ((n + ALIGN_SIZE - 1) / ALIGN_SIZE) * ALIGN_SIZE;
    return 0;
}


static void *arena_alloc(size_t n, int scoped) {
    struct blk *b;
    size_t need;

    if (round_up_size(n, &need)) {
        arena_error = 1;
        return NULL;
    }

    /* First fit among the free blocks. */
    for (b = first_blk; b != NULL; b = b->next) {
        if (!b->is_free || b->size < need) {
            continue;
        }
        /* Split the block if the remainder can hold a block. */
        if (b->size >= need + HDR_SIZE + ALIGN_SIZE) {
            struct blk *nb;

            nb = (struct blk *)((char *)b + HDR_SIZE + need);
            nb->prev = b;
            nb->next = b->next;
            nb->size = b->size - need - HDR_SIZE;
            nb->is_free = 1;
            nb->scoped = 0;
            nb->magic = 0;
            if (nb->next != NULL) {
                nb->next->prev = nb;
            }
            else {
                last_blk = nb;
            }
            b->next = nb;
            b->size = need;
        }
        b->is_free = 0;
        b->scoped = scoped;
        b->magic = BLK_MAGIC;
        if (scoped) {
            live_scoped++;
        }
        return (char *)b + HDR_SIZE;
    }

    /* Allocate a fresh block at the tail of the arena. */
    if (arena_used + HDR_SIZE + need > ARENA_BYTES) {
        arena_error = 1;
        return NULL;
    }
    b = (struct blk *)((char *)arena + arena_used);
    arena_used += HDR_SIZE + need;
    b->prev = last_blk;
    b->next = NULL;
    b->size = need;
    b->is_free = 0;
    b->scoped = scoped;
    b->magic = BLK_MAGIC;
    if (last_blk != NULL) {
        last_blk->next = b;
    }
    else {
        first_blk = b;
    }
    last_blk = b;
    if (scoped) {
        live_scoped++;
    }
    return (char *)b + HDR_SIZE;
}


/*
 * Release a block and coalesce it with its free neighbours.  A pointer that
 * is not in the arena at all was allocated by some other allocator (parts
 * of the C library allocate internally with symbols this test does not
 * replace); there is no portable way to release such a block here, so it is
 * left alone.  A block that is in the arena but already free is a double
 * free or worse and is reported.
 */
static void arena_free(void *p) {
    struct blk *b;

    if (p == NULL) {
        return;
    }
    if ((char *)p < (char *)arena + HDR_SIZE || (char *)p >= (char *)arena + ARENA_BYTES) {
        return;
    }
    b = (struct blk *)((char *)p - HDR_SIZE);
    if (b->magic != BLK_MAGIC) {
        /* Double free or corruption. */
        arena_error = 1;
        return;
    }
    b->magic = 0;
    b->is_free = 1;
    if (b->scoped) {
        live_scoped--;
    }

    if (b->next != NULL && b->next->is_free) {
        b->size += HDR_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next != NULL) {
            b->next->prev = b;
        }
        else {
            last_blk = b;
        }
    }
    if (b->prev != NULL && b->prev->is_free) {
        b->prev->size += HDR_SIZE + b->size;
        b->prev->next = b->next;
        if (b->next != NULL) {
            b->next->prev = b->prev;
        }
        else {
            last_blk = b->prev;
        }
    }
}


static void *arena_realloc(void *p, size_t n) {
    struct blk *b;
    size_t need;
    void *q;

    if (p == NULL) {
        return arena_alloc(n, in_case);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    b = (struct blk *)((char *)p - HDR_SIZE);
    if (b->magic != BLK_MAGIC) {
        arena_error = 1;
        return NULL;
    }
    if (round_up_size(n, &need)) {
        arena_error = 1;
        return NULL;
    }
    if (b->size >= need) {
        return p;
    }
    /*
     * Allocate before releasing, so that on failure the original block
     * remains valid and unchanged, as realloc() requires.  The copy
     * inherits the scope of the original block.
     */
    q = arena_alloc(n, b->scoped);
    if (q == NULL) {
        return NULL;
    }
    memcpy(q, p, b->size);
    arena_free(p);
    return q;
}


/*
 * Decide whether this allocation attempt should fail.  The counter runs
 * only while the library call executes, so allocations made by the test
 * itself never fail.
 */
static int should_fail(void) {
    if (!alloc_armed) {
        return 0;
    }
    alloc_seq++;
    return alloc_fail_at && alloc_seq == alloc_fail_at;
}


void *malloc(size_t n) {
    if (should_fail()) {
        return NULL;
    }
    return arena_alloc(n, in_case);
}


void *calloc(size_t m, size_t n) {
    void *p;

    if (should_fail()) {
        return NULL;
    }
    if (m != 0 && n > (size_t)-1 / m) {
        return NULL;
    }
    p = arena_alloc(m * n, in_case);
    if (p != NULL) {
        memset(p, 0, m * n);
    }
    return p;
}


void *realloc(void *p, size_t n) {
    if (should_fail()) {
        return NULL;
    }
    return arena_realloc(p, n);
}


void free(void *p) {
    arena_free(p);
}


/* ---- test cases ---- */

static const char *case_names[] = {
    "single buffer (freep=1)",
    "two fragments (freep=1)",
    "zero-length fragment (freep=1)",
    "single buffer (freep=0)",
};


/*
 * Run one test case.  With fail_at == 0 the source creation must succeed;
 * otherwise a failure is tolerated as long as the buffer cleanup works out.
 * Returns nonzero on a test failure.  If count is not NULL, the number of
 * allocations made during the library call is returned there.
 */
static int run_case(int which, long fail_at, long *count) {
    zip_error_t error;
    zip_source_t *zs;
    char *buf[2];
    zip_buffer_fragment_t fragments[2];
    int nbufs, nfragments, freep, i;

    switch (which) {
    case 0:
        nbufs = 1;
        nfragments = 1;
        freep = 1;
        break;
    case 1:
        nbufs = 2;
        nfragments = 2;
        freep = 1;
        break;
    case 2:
        nbufs = 2;
        nfragments = 2;
        freep = 1;
        break;
    default:
        nbufs = 1;
        nfragments = 1;
        freep = 0;
        break;
    }

    zip_error_init(&error);
    buf[0] = NULL;
    buf[1] = NULL;

    arena_error = 0;
    in_case = 1;
    buf[0] = malloc(16);
    if (nbufs > 1) {
        buf[1] = malloc(32);
    }
    if (buf[0] == NULL || (nbufs > 1 && buf[1] == NULL)) {
        fprintf(stderr, "%s: test fixture allocation failed\n", case_names[which]);
        arena_error = 1;
        in_case = 0;
        return 1;
    }

    fragments[0].data = (zip_uint8_t *)buf[0];
    fragments[0].length = 16;
    fragments[1].data = (zip_uint8_t *)buf[1];
    fragments[1].length = (which == 2) ? 0 : 32;

    alloc_seq = 0;
    alloc_fail_at = fail_at;
    alloc_armed = 1;
    if (nfragments == 1) {
        zs = zip_source_buffer_create(buf[0], 16, freep, &error);
    }
    else {
        zs = zip_source_buffer_fragment_create(fragments, 2, freep, &error);
    }
    alloc_armed = 0;

    if (count != NULL) {
        *count = alloc_seq;
    }

    if (zs == NULL) {
        /* The call failed: the caller keeps ownership of the buffers. */
        for (i = 0; i < nbufs; i++) {
            free(buf[i]);
        }
    }
    else {
        zip_source_free(zs);
        if (!freep) {
            for (i = 0; i < nbufs; i++) {
                free(buf[i]);
            }
        }
        /* else: the source owned the buffers and freed them (data of zero length fragments is freed when the source is created). */
    }
    in_case = 0;

    if (fail_at == 0 && zs == NULL) {
        fprintf(stderr, "%s: source creation failed without an injected failure: %s\n", case_names[which], zip_error_strerror(&error));
        zip_error_fini(&error);
        return 1;
    }
    zip_error_fini(&error);
    return 0;
}


/*
 * Fail every allocation of the source creation in turn and check that the
 * buffer cleanup works out in every case.  Returns -1 if the allocator
 * replacements are not used by libzip (no link-time interposition), nonzero
 * on a test failure, zero otherwise.
 */
static int sweep_case(int which) {
    long n, i;

    if (run_case(which, 0, &n) != 0) {
        return 1;
    }
    if (n == 0) {
        return -1;
    }
    if (arena_error) {
        fprintf(stderr, "%s: arena error (double free or corruption)\n", case_names[which]);
        return 1;
    }
    if (live_scoped != 0) {
        fprintf(stderr, "%s: leaked %ld block(s)\n", case_names[which], live_scoped);
        return 1;
    }

    for (i = 1; i <= n; i++) {
        if (run_case(which, i, NULL) != 0) {
            return 1;
        }
        if (arena_error) {
            fprintf(stderr, "%s: allocation %ld failing freed a block twice (or corrupted the heap)\n", case_names[which], i);
            return 1;
        }
        if (live_scoped != 0) {
            fprintf(stderr, "%s: allocation %ld failing leaked %ld block(s)\n", case_names[which], i, live_scoped);
            return 1;
        }
    }
    return 0;
}


int main(void) {
    int i, ret, failed = 0;

    /* Print early so the C library allocates any stdio buffers outside the measured windows. */
    printf("testing allocation-failure handling of zip_source_buffer\n");

    for (i = 0; i < 4; i++) {
        ret = sweep_case(i);
        if (ret < 0) {
            printf("SKIP: the allocator replacements are not used by libzip in this build\n");
            return 77;
        }
        if (ret != 0) {
            failed = 1;
        }
    }

    if (failed) {
        return 1;
    }
    printf("OK\n");
    return 0;
}
