/*
  $NiH$

  zip_error_sterror.c -- get string representation of struct zip_error
  Copyright (C) 1999, 2003 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipint.h"



const char *
_zip_error_strerror(struct zip_error *err)
{
    _zip_error_fini(err);

    if (err->zip_err < 0 || err->zip_err >= zip_nerr_str) {
	/* XXX */
	asprintf(&err->str, "Unknown error %d", err->zip_err);
	return err->str;
    }

    switch (_zip_err_type[err->zip_err]) {
    case ZIP_ET_SYS:
	/* XXX */
	asprintf(&err->str, "%s: %s",
		 zip_err_str[err->zip_err], strerror(err->sys_err));
	return err->str;

    case ZIP_ET_ZIP:
	/* XXX */
	asprintf(&err->str, "%s: %s",
		 zip_err_str[err->zip_err], zError(err->sys_err));
	return err->str;

    default:
	return zip_err_str[err->zip_err];
    }
}
