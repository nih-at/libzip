/*
 zip_io_util.c -- I/O helper functions
 Copyright (C) 1999-2014 Dieter Baron and Thomas Klausner
 
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zipint.h"

zip_uint16_t
_zip_get_16(const zip_uint8_t **a)
{
    zip_uint16_t ret;

    ret = (zip_uint16_t)((*a)[0]+((*a)[1]<<8));
    *a += 2;

    return ret;
}


zip_uint32_t
_zip_get_32(const zip_uint8_t **a)
{
    zip_uint32_t ret;

    ret = ((((((zip_uint32_t)(*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;

    return ret;
}


zip_uint64_t
_zip_get_64(const zip_uint8_t **a)
{
    zip_uint64_t x, y;

    x = ((((((zip_uint64_t)(*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;
    y = ((((((zip_uint64_t)(*a)[3]<<8)+(*a)[2])<<8)+(*a)[1])<<8)+(*a)[0];
    *a += 4;

    return x+(y<<32);
}

int
_zip_read(struct zip_source *src, zip_uint8_t *b, zip_uint64_t length, struct zip_error *error)
{
    zip_int64_t n;

    if (length > ZIP_INT64_MAX) {
	zip_error_set(error, ZIP_ER_INTERNAL, 0);
	return -1;
    }

    if ((n = zip_source_read(src, b, length)) < 0) {
	_zip_error_set_from_source(error, src);
	return -1;
    }

    if (n < (zip_int64_t)length) {
	zip_error_set(error, ZIP_ER_EOF, 0);
	return -1;
    }

    return 0;
}


zip_uint8_t *
_zip_read_data(const zip_uint8_t **buf, struct zip_source *src, size_t len, int nulp, struct zip_error *error)
{
    zip_uint8_t *r;

    if (len == 0 && nulp == 0)
	return NULL;

    r = (zip_uint8_t *)malloc(nulp ? len+1 : len);
    if (!r) {
	zip_error_set(error, ZIP_ER_MEMORY, 0);
	return NULL;
    }

    if (buf) {
	memcpy(r, *buf, len);
	*buf += len;
    }
    else {
	if (_zip_read(src, r, len, error) < 0) {
	    free(r);
	    return NULL;
	}
    }

    if (nulp) {
	zip_uint8_t *o;
	/* replace any in-string NUL characters with spaces */
	r[len] = 0;
	for (o=r; o<r+len; o++)
	    if (*o == '\0')
		*o = ' ';
    }

    return r;
}


struct zip_string *
_zip_read_string(const zip_uint8_t **buf, struct zip_source *src, zip_uint16_t len, int nulp, struct zip_error *error)
{
    zip_uint8_t *raw;
    struct zip_string *s;

    if ((raw=_zip_read_data(buf, src, len, nulp, error)) == NULL)
	return NULL;

    s = _zip_string_new(raw, len, ZIP_FL_ENC_GUESS, error);
    free(raw);
    return s;
}




void
_zip_put_16(zip_uint8_t **p, zip_uint16_t i)
{
    *((*p)++) = (zip_uint8_t)(i&0xff);
    *((*p)++) = (zip_uint8_t)((i>>8)&0xff);
}


void
_zip_put_32(zip_uint8_t **p, zip_uint32_t i)
{
    *((*p)++) = (zip_uint8_t)(i&0xff);
    *((*p)++) = (zip_uint8_t)((i>>8)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>16)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>24)&0xff);
}


void
_zip_put_64(zip_uint8_t **p, zip_uint64_t i)
{
    *((*p)++) = (zip_uint8_t)(i&0xff);
    *((*p)++) = (zip_uint8_t)((i>>8)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>16)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>24)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>32)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>40)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>48)&0xff);
    *((*p)++) = (zip_uint8_t)((i>>56)&0xff);
}


void
_zip_put_data(zip_uint8_t **p, const char *s, size_t len)
{
    memcpy(*p, s, len);
    *p += len;
}


int
_zip_write(struct zip *za, const void *data, zip_uint64_t length)
{
    zip_int64_t n;
    
    if ((n = zip_source_write(za->src, data, length)) < 0) {
        _zip_error_set_from_source(&za->error, za->src);
        return -1;
    }
    if ((zip_uint64_t)n != length) {
        zip_error_set(&za->error, ZIP_ER_WRITE, EINTR);
        return -1;
    }
    
    return 0;
}
