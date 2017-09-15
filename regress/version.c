/*
  version.c -- tool to check libzip version
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "zip.h"


int
main(int argc, char *argv[])
{
	printf("Buildtime version: %s, %d.%d.%d (0x%x)\n",
		LIBZIP_VERSION,
		LIBZIP_VERSION_MAJOR,
		LIBZIP_VERSION_MINOR,
		LIBZIP_VERSION_MICRO,
		LIBZIP_VERSION_ID);
	printf("Runtime Version:   %s, %d.%d.%d (0x%x)\n",
		zip_get_version(),
		zip_get_version_major(),
		zip_get_version_minor(),
		zip_get_version_micro(),
		zip_get_version_id(0));

	if (zip_get_version_major() != LIBZIP_VERSION_MAJOR) return 1;
	if (zip_get_version_minor() != LIBZIP_VERSION_MINOR) return 2;
	if (zip_get_version_micro() != LIBZIP_VERSION_MICRO) return 3;
	if (strcmp(zip_get_version(), LIBZIP_VERSION)) return 4;
	if (zip_get_version_id(0) != LIBZIP_VERSION_ID) return 5;

    return 0;
}
