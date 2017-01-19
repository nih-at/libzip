/*
  nonrandomopen.c -- override open() of /dev/urandom
  Copyright (C) 2017 Dieter Baron and Thomas Klausner

  This file is part of ckmame, a program to check rom sets for MAME.
  The authors can be contacted at <ckmame@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The name of the author may not be used to endorse or promote
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

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU

#include "config.h"

#if !defined(RTLD_NEXT)
#define RTLD_NEXT	RTLD_DEFAULT
#endif

static int inited = 0;
static int (*real_open)(const char *path, int mode, ...) = NULL;

static void
init(void)
{
    char *foo;
    real_open = dlsym(RTLD_NEXT, "open");
    if (!real_open)
	abort();
    inited = 1;
}

int
open(const char *path, int flags, ...)
{
    va_list ap;
    mode_t mode;

    if (!inited) {
	init();
    }

    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);

    if (strcmp(path, "/dev/urandom") == 0) {
	return real_open("/dev/zero", flags, mode);
    } else {
	return real_open(path, flags, mode);
    }
}
