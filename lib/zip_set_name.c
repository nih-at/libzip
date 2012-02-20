/*
  zip_set_name.c -- rename helper function
  Copyright (C) 1999-2012 Dieter Baron and Thomas Klausner

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



#include <stdlib.h>
#include <string.h>

#include "zipint.h"



int
_zip_set_name(struct zip *za, zip_uint64_t idx, const char *name)
{
    char *s;
    const char *com;
    int comlen;
    zip_int64_t i;
    enum zip_encoding_type enc, com_enc;
    
    if (idx >= za->nentry || name == NULL) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return -1;
    }

    if ((enc=_zip_guess_encoding(name, strlen(name))) == ZIP_ENCODING_CP437) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return -1;
    }

    if ((i=_zip_name_locate(za, name, 0, NULL)) != -1 && i != idx) {
	_zip_error_set(&za->error, ZIP_ER_EXISTS, 0);
	return -1;
    }

    /* no effective name change */
    if (i == idx)
	return 0;

    com = zip_get_file_comment(za, idx, &comlen, 0);
    if (com == NULL)
	com_enc = ZIP_ENCODING_ASCII;
    else
	com_enc = _zip_guess_encoding(com, comlen);
    if (com_enc == ZIP_ENCODING_CP437 && enc == ZIP_ENCODING_UTF8) {
	_zip_error_set(&za->error, ZIP_ER_ENCMISMATCH, 0);
	return -1;
    }

    if ((s=strdup(name)) == NULL) {
	_zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
	return -1;
    }
    
    if (za->entry[idx].state == ZIP_ST_UNCHANGED) 
	za->entry[idx].state = ZIP_ST_RENAMED;

    if (za->entry[idx].changes.valid & ZIP_DIRENT_FILENAME)
	free(za->entry[idx].changes.filename);
    za->entry[idx].changes.filename = s;
    za->entry[idx].changes.valid |= ZIP_DIRENT_FILENAME;

    return 0;
}
