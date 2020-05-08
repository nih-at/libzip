/*
  zip_source_file_win32_utf16.c -- Windows file source write operations for UTF-16 interface
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

static char *utf16_allocate_tempname(const char *name, size_t extra_chars, size_t *lengthp);
static HANDLE utf16_create_file(const char *name, DWORD access, DWORD share_mode, PSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD file_attributes, HANDLE template_file);
static BOOL utf16_delete_file(const char *name);
static DWORD utf16_get_file_attributes(const char *name);
static void utf16_make_tempname(char *buf, size_t len, const char *name, int i);
static BOOL utf16_move_file(const char *from, const char *to, DWORD flags);
static BOOL utf16_set_file_attributes(const char *name, DWORD attributes);
static char *utf16_strdup(const char *string);

zip_source_file_win32_write_operations_t ops_utf16 = {
    utf16_allocate_tempname,
    utf16_create_file,
    utf16_delete_file,
    utf16_get_file_attributes,
    utf16_make_tempname,
    utf16_move_file,
    utf16_set_file_attributes,
    utf16_strdup
};

ZIP_EXTERN zip_source_t *
zip_source_win32w(zip_t *za, const wchar_t *fname, zip_uint64_t start, zip_int64_t len) {
    if (za == NULL)
    return NULL;

    return zip_source_win32w_create(fname, start, len, &za->error);
}


ZIP_EXTERN zip_source_t *
zip_source_win32w_create(const wchar_t *fname, zip_uint64_t start, zip_int64_t length, zip_error_t *error) {
    if (fname == NULL || length < -1) {
    zip_error_set(error, ZIP_ER_INVAL, 0);
    return NULL;
    }


    return zip_source_file_common_new((const char *)fname, NULL, start, length, NULL, &_zip_source_file_win32_write_ops, &ops_utf16, error);
}


static char *
utf16_allocate_tempname(const char *name, size_t extra_chars, size_t *lengthp) {
    *lengthp = wcslen((const wchar_t *)name) + extra_chars;
    return (char *)malloc(*lengthp * sizeof(wchar_t));
}


HANDLE
utf16_create_file(const char *name, DWORD access, DWORD share_mode, PSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD file_attributes, HANDLE template_file) {
#ifdef MS_UWP
    CREATEFILE2_EXTENDED_PARAMETERS extParams = {0};
    extParams.dwFileAttributes = file_attributes;
        extParams.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
        extParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
        extParams.dwSize = sizeof(extParams);
        extParams.hTemplateFile = NULL;
        extParams.lpSecurityAttributes = security_attributes;

        return CreateFile2((const wchar_t *)name, access, share_mode, creation_disposition, &extParams);
    #else
        return CreateFileW((const wchar_t *)name, access, share_mode, security_attributes, creation_disposition, file_attributes, NULL);
    #endif
}


static BOOL
utf16_delete_file(const char *name) {
    return DeleteFileW((const wchar_t *)name);
}


static DWORD
utf16_get_file_attributes(const char *name) {
    return GetFileAttributesW((const wchar_t *)name);
}


static void
utf16_make_tempname(char *buf, size_t len, const char *name, int i) {
    _snwprintf((wchar_t *)buf, len, L"%s.%08x", (const wchar_t *)name, i);
}

static BOOL
utf16_move_file(const char *from, const char *to, DWORD flags) {
    return MoveFileExW((const wchar_t *)from, (const wchar_t *)to, flags);
}


static BOOL
utf16_set_file_attributes(const char *name, DWORD attributes) {
    return SetFileAttributesW((const wchar_t *)name, attributes);
}


static char *
utf16_strdup(const char *string) {
    return (char *)_wcsdup((const wchar_t *)string);
}
