/*
  zip_source_file_win32_ansi.c -- read/write Windows ANSI file source implementation
  Copyright (C) 1999-2020 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

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

#include "zip_source_file_win32.h"

static zip_int64_t _zip_win32_ansi_op_commit_write(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_ansi_op_create_temp_output(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_ansi_op_commit_write(zip_source_file_context_t *ctx);
static bool _zip_win32_ansi_op_open(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_ansi_op_remove(zip_source_file_context_t *ctx);
static void _zip_win32_ansi_op_rollback_write(zip_source_file_context_t *ctx);
static bool _zip_win32_ansi_op_stat(zip_source_file_context_t *ctx, zip_source_file_stat_t *st);

static HANDLE win32_ansi_open(zip_source_file_context_t *ctx);

zip_source_file_operations_t ops_win32_read = {
    _zip_win32_op_close,
    _zip_win32_ansi_op_commit_write,
    _zip_win32_ansi_op_create_temp_output,
    NULL,
    _zip_win32_ansi_op_open,
    _zip_win32_op_read,
    _zip_win32_ansi_op_remove,
    _zip_win32_ansi_op_rollback_write,
    _zip_win32_op_seek,
    _zip_win32_ansi_op_stat,
    strdup,
    _zip_win32_op_tell,
    _zip_win32_op_write
}

static zip_int64_t
_zip_win32_ansi_op_commit_write(zip_source_file_context_t *ctx) {
    if (!CloseHandle((HANDLE)ctx->fout)) {
        zip_error_set(&ctx->error, ZIP_ER_WRITE, _zip_win32_error_to_errno(GetLastError()));
        return -1
    }
    
    DWORD attributes = GetFileAttributesA(ctx->tmpname);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }

    if (attributes & FILE_ATTRIBUTE_TEMPORARY) {
        if (!SetFileAttributesA(ctx->tmpname, attributes & ~FILE_ATTRIBUTE_TEMPORARY)) {
            zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
            return -1;
        }
    }

    if (!MoveFileExA(ctx->tmpname, ctx->fname, MOVEFILE_REPLACE_EXISTING)) {
        zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }

    return 0;
}

static zip_int64_t
_zip_win32_ansi_op_create_temp_output(zip_source_file_context_t *ctx) {
    size_t len;
    void *temp;
    HANDLE h;

    len = strlen(ctx->fname) + 10;
    if ((temp = malloc(len)) == NULL) {
        zip_error_set(&ctx->error, ZIP_ER_MEMORY, 0);
        return false;
    }
    if (sprintf((char *)*temp, "%s.%08x", (const char *)ctx->fname, value) != len - 1) {
        return -1;
    }
    
    h = CreateFileA((const char *)*temp, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, sa, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        free(temp);
        return -1;
    }
    
    ctx->tmpname = temp;
    ctx->fout = h;
    
    return 0;
}



static bool
_zip_win32_ansi_op_open(zip_source_file_context_t *ctx) {
    HANDLE h = win32_ansi_open(ctx);
    
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    ctx->f = h;
    return true;
}


static zip_int64_t
_zip_win32_ansi_op_remove(zip_source_file_context_t *ctx) {
    DeleteFileA((const char *)fname);
    /* TODO: propagate errors */
    return 0;
}


static void
_zip_win32_ansi_op_rollback_write(zip_source_file_context_t *ctx) {
    if (ctx->fout) {
        CloseHandle((HANDLE)ctx->fout);
    }
    DeleteFileA(ctx->tmpname);
}


static bool
_zip_win32_ansi_op_stat(zip_source_file_context_t *ctx, zip_source_file_stat_t *st) {
    HANDLE h = win32_ansi_open(ctx);
    
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    return _zip_stat_win32(ctx, st, h);
}


static HANDLE
win32_ansi_open(zip_source_file_context_t *ctx) {
    HANDLE h = CreateFileA(ctx->fname, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (h == INVALID_HANDLE_VALUE) {
        zip_error_set(&ctx->error, ZIP_ER_OPEN, _zip_win32_error_to_errno(GetLastError()));
    }

    return h;
}
