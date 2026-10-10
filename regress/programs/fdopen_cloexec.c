/*
  fdopen_cloexec.c -- check close-on-exec protection of the owned descriptor
  Copyright (C) 2026 KiritoYG

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

#include "zipint.h"
#include "zip_source_file.h"

#if defined(ENABLE_FDOPEN) && defined(F_DUPFD_CLOEXEC) && defined(HAVE_UNISTD_H)
#include <unistd.h>

static int check_archive(const char *path, int original_flags) {
    int fd, owned_fd, flags, error;
    zip_t *za;
    zip_file_t *file;
    zip_source_file_context_t *ctx;
    unsigned char byte;
    int ret = 0;

    if ((fd = open(path, O_RDONLY)) < 0) {
        perror("prepare descriptor");
        return 1;
    }
    if (fcntl(fd, F_SETFD, original_flags) < 0) {
        perror("set descriptor flags");
        close(fd);
        return 1;
    }
    if ((za = zip_fdopen(fd, 0, &error)) == NULL) {
        fprintf(stderr, "zip_fdopen failed: %d\n", error);
        close(fd);
        return 1;
    }

    /* Inspect the actual owned stream, without assuming a descriptor number. */
    ctx = (zip_source_file_context_t *)za->src->ud;
    owned_fd = fileno((FILE *)ctx->f);
    flags = fcntl(owned_fd, F_GETFD);
    if (flags < 0 || !(flags & FD_CLOEXEC)) {
        fprintf(stderr, "libzip-owned descriptor lacks FD_CLOEXEC\n");
        ret = 1;
    }
    if ((file = zip_fopen_index(za, 0, 0)) == NULL) {
        fprintf(stderr, "zip_fopen_index failed\n");
        ret = 1;
    }
    else {
        if (zip_fread(file, &byte, 1) != 1 || byte != 'a') {
            fprintf(stderr, "archive read failed\n");
            ret = 1;
        }
        if (zip_fclose(file) < 0) {
            ret = 1;
        }
    }
    if (zip_close(za) < 0) {
        fprintf(stderr, "zip_close failed\n");
        zip_discard(za);
        ret = 1;
    }
    if (fcntl(owned_fd, F_GETFD) != -1 || errno != EBADF) {
        fprintf(stderr, "archive close leaked its descriptor\n");
        ret = 1;
    }
    return ret;
}

static int check_error(const char *path, int flags, int expected_error) {
    int fd, error, ret = 0;
    zip_t *za;

    if ((fd = open(path, O_RDONLY)) < 0) {
        perror("prepare descriptor");
        return 1;
    }
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
        perror("set descriptor flags");
        close(fd);
        return 1;
    }
    if ((za = zip_fdopen(fd, flags, &error)) != NULL) {
        fprintf(stderr, "invalid input accepted\n");
        zip_discard(za);
        return 1;
    }
    if (error != expected_error || fcntl(fd, F_GETFD) != FD_CLOEXEC) {
        fprintf(stderr, "failed open changed descriptor ownership or flags\n");
        ret = 1;
    }
    close(fd);
    return ret;
}
#endif

int main(int argc, char *argv[]) {
#if defined(ENABLE_FDOPEN) && defined(F_DUPFD_CLOEXEC) && defined(HAVE_UNISTD_H)
    int ret;
    if (argc != 3) {
        fprintf(stderr, "usage: %s valid-archive invalid-archive\n", argv[0]);
        return 1;
    }
    ret = check_archive(argv[1], 0);
    ret |= check_archive(argv[1], FD_CLOEXEC);
    ret |= check_error(argv[1], ZIP_CREATE, ZIP_ER_INVAL);
    ret |= check_error(argv[2], 0, ZIP_ER_NOZIP);
    return ret;
#else
    (void)argc;
    (void)argv;
    return 77;
#endif
}
