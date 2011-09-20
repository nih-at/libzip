/*
  encoding.c -- tool for encoding tests
  Copyright (C) 2011 Dieter Baron and Thomas Klausner

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipint.h"

const char *result[] = {
    "ASCII",
    "UTF-8",
    "CP437"
};

int
main(int argc, char *argv[])
{
    int ret;
    const char *conv = NULL;
    struct zip_error err;

    if (argc != 2) {
	fprintf(stderr, "usage: %s string_to_guess\n", argv[0]);
	exit(1);
    }
    ret = _zip_guess_encoding((const zip_uint8_t *)argv[1], strlen(argv[1]));

    printf("guessing %s: %s\n", argv[1], result[ret]);
    if (ret == ZIP_ENCODING_CP437) {
	conv = (const char *)_zip_cp437_to_utf8((const zip_uint8_t *)argv[1], strlen(argv[1]), &err);
	printf("UTF-8 version: %s\n", conv ? conv : "(conversion error)");
    }

    exit(0);
}
