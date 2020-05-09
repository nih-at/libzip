/*
  zip_source_file_win32_write.c -- read/write Windows file source implementation
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

static zip_int64_t _zip_win32_write_op_commit_write(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_write_op_create_temp_output(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_write_op_commit_write(zip_source_file_context_t *ctx);
static bool _zip_win32_write_op_open(zip_source_file_context_t *ctx);
static zip_int64_t _zip_win32_write_op_remove(zip_source_file_context_t *ctx);
static void _zip_win32_write_op_rollback_write(zip_source_file_context_t *ctx);
static bool _zip_win32_write_op_stat(zip_source_file_context_t *ctx, zip_source_file_stat_t *st);
static char *_zip_win32_write_strdup(zip_source_file_context_t *ctx, const char *string);

static HANDLE win32_write_open(zip_source_file_context_t *ctx, const char *name, bool temporary, PSECURITY_ATTRIBUTES security_attributes);

/* clang-format off */
zip_source_file_operations_t _zip_source_file_win32_write_ops = {
    _zip_win32_op_close,
    _zip_win32_write_op_commit_write,
    _zip_win32_write_op_create_temp_output,
    NULL,
    _zip_win32_write_op_open,
    _zip_win32_op_read,
    _zip_win32_write_op_remove,
    _zip_win32_write_op_rollback_write,
    _zip_win32_op_seek,
    _zip_win32_write_op_stat,
    _zip_win32_write_strdup,
    _zip_win32_op_tell,
    _zip_win32_op_write
};
/* clang-format on */

static zip_int64_t
_zip_win32_write_op_commit_write(zip_source_file_context_t *ctx) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;
    
    if (!CloseHandle((HANDLE)ctx->fout)) {
        zip_error_set(&ctx->error, ZIP_ER_WRITE, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }
    
    DWORD attributes = write_ops->get_file_attributes(ctx->tmpname);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }

    if (attributes & FILE_ATTRIBUTE_TEMPORARY) {
        if (!write_ops->set_file_attributes(ctx->tmpname, attributes & ~FILE_ATTRIBUTE_TEMPORARY)) {
            zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
            return -1;
        }
    }

    if (!write_ops->move_file(ctx->tmpname, ctx->fname, MOVEFILE_REPLACE_EXISTING)) {
        zip_error_set(&ctx->error, ZIP_ER_RENAME, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }

    return 0;
}

static zip_int64_t
_zip_win32_write_op_create_temp_output(zip_source_file_context_t *ctx) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;

    int value;
    int i;
    HANDLE th = INVALID_HANDLE_VALUE;
    void *temp = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    PSECURITY_ATTRIBUTES psa = NULL;
    SECURITY_ATTRIBUTES sa;
    SECURITY_INFORMATION si;
    DWORD success;
    PACL dacl = NULL;
    char *tempname = NULL;
    size_t tempname_size = 0;

     if ((HANDLE)ctx->f != INVALID_HANDLE_VALUE && GetFileType((HANDLE)ctx->f) == FILE_TYPE_DISK) {
         si = DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION;
         success = GetSecurityInfo((HANDLE)ctx->f, SE_FILE_OBJECT, si, NULL, NULL, &dacl, NULL, &psd);
         if (success == ERROR_SUCCESS) {
             sa.nLength = sizeof(SECURITY_ATTRIBUTES);
             sa.bInheritHandle = FALSE;
             sa.lpSecurityDescriptor = psd;
             psa = &sa;
         }
     }

 #ifndef MS_UWP
    value = GetTickCount();
#else
    value = GetTickCount64();
#endif
    
    if ((tempname = write_ops->allocate_tempname(ctx->fname, 10, &tempname_size)) == NULL) {
        zip_error_set(&ctx->error, ZIP_ER_MEMORY, 0);
        return -1;
    }
        
    for (i = 0; i < 1024 && th == INVALID_HANDLE_VALUE; i++) {
        write_ops->make_tempname(tempname, tempname_size, ctx->fname, value + i);
        
        th = win32_write_open(ctx, tempname, true, psa);
        if (th == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS)
            break;
    }
    
    if (th == INVALID_HANDLE_VALUE) {
        free(tempname);
        LocalFree(psd);
        zip_error_set(&ctx->error, ZIP_ER_TMPOPEN, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }
    
    LocalFree(psd);
    ctx->fout = th;
    ctx->tmpname = tempname;
    
    return 0;
}


static bool
_zip_win32_write_op_open(zip_source_file_context_t *ctx) {
    HANDLE h = win32_write_open(ctx, ctx->fname, false, NULL);
    
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    ctx->f = h;
    return true;
}


static zip_int64_t
_zip_win32_write_op_remove(zip_source_file_context_t *ctx) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;

    if (!write_ops->delete_file(ctx->fname)) {
        zip_error_set(&ctx->error, ZIP_ER_REMOVE, _zip_win32_error_to_errno(GetLastError()));
        return -1;
    }

    return 0;
}


static void
_zip_win32_write_op_rollback_write(zip_source_file_context_t *ctx) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;

    if (ctx->fout) {
        CloseHandle((HANDLE)ctx->fout);
    }
    write_ops->delete_file(ctx->tmpname);
}


static bool
_zip_win32_write_op_stat(zip_source_file_context_t *ctx, zip_source_file_stat_t *st) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;
    
    WIN32_FILE_ATTRIBUTE_DATA file_attributes;

    if (!write_ops->get_file_attributes_ex(ctx->fname, GetFileExInfoStandard, &file_attributes)) {
        printf("get_file_attributes_ex failed: %d\n", (int)GetLastError());
        zip_error_set(&ctx->error, ZIP_ER_READ, _zip_win32_error_to_errno(GetLastError()));
        return false;
    }
    
    st->exists = true;
    st->regular_file = true; /* TODO: Is this always right? How to determine without a HANDLE? */
    if (!_zip_filetime_to_time_t(file_attributes.ftLastWriteTime, &st->mtime)) {
        printf("filetime_to_time_t failed\n");
        zip_error_set(&ctx->error, ZIP_ER_READ, ERANGE);
        return false;
    }
    st->size = ((zip_uint64_t)file_attributes.nFileSizeHigh << 32) | file_attributes.nFileSizeLow;

    /* TODO: fill in ctx->attributes */

    printf("stat succeded: size: %llu\n", st->size);
    return true;
}


static char *
_zip_win32_write_strdup(zip_source_file_context_t *ctx, const char *string) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;

    return write_ops->string_duplicate(string);
}



static HANDLE
win32_write_open(zip_source_file_context_t *ctx, const char *name, bool temporary, PSECURITY_ATTRIBUTES security_attributes) {
    zip_source_file_win32_write_operations_t *write_ops = (zip_source_file_win32_write_operations_t *)ctx->ops_userdata;

    DWORD access = GENERIC_READ;
    DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD creation_disposition = OPEN_EXISTING;
    DWORD file_attributes = FILE_ATTRIBUTE_NORMAL;
    
    if (temporary) {
        access = GENERIC_READ | GENERIC_WRITE;
        share_mode = FILE_SHARE_READ;
        creation_disposition = CREATE_NEW;
        file_attributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY;
    }

    HANDLE h = write_ops->create_file(name, access, share_mode, security_attributes, creation_disposition, file_attributes, NULL);
    
    if (h == INVALID_HANDLE_VALUE) {
        zip_error_set(&ctx->error, ZIP_ER_OPEN, _zip_win32_error_to_errno(GetLastError()));
    }

    return h;
}
