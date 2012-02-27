/*
  zip_get_file_comment.c -- get file comment
  Copyright (C) 2006-2012 Dieter Baron and Thomas Klausner

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



#include <string.h>

#include "zipint.h"



ZIP_EXTERN const char *
zip_get_file_comment(struct zip *za, zip_uint64_t idx, int *lenp, int flags)
{
    const char *ret;

    if (idx >= za->nentry) {
	_zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	return NULL;
    }

    if ((flags & ZIP_FL_UNCHANGED) || (za->entry[idx].changes.valid & ZIP_DIRENT_COMMENT) == 0) {
	if (za->cdir == NULL) {
	    _zip_error_set(&za->error, ZIP_ER_NOENT, 0);
	    return NULL;
	}
	if (idx >= za->cdir->nentry) {
	    _zip_error_set(&za->error, ZIP_ER_INVAL, 0);
	    return NULL;
	}
	    
	if (lenp != NULL)
	    *lenp = za->cdir->entry[idx].settable.comment_len;
	ret = za->cdir->entry[idx].settable.comment;

	if (flags & ZIP_FL_NAME_RAW)
	    return ret;

	/* file comment already is UTF-8? */
	if (za->cdir->entry[idx].bitflags & ZIP_GPBF_ENCODING_UTF_8)
	    return ret;
	
	/* undeclared, start guessing */
	if (za->cdir->entry[idx].fc_type == ZIP_ENCODING_UNKNOWN)
	    za->cdir->entry[idx].fc_type = _zip_guess_encoding(ret, za->cdir->entry[idx].settable.comment_len);

	if (((flags & ZIP_FL_NAME_STRICT) && (za->cdir->entry[idx].fc_type != ZIP_ENCODING_ASCII))
	    || (za->cdir->entry[idx].fc_type == ZIP_ENCODING_CP437)) {
	    if (za->cdir->entry[idx].comment_converted == NULL)
		za->cdir->entry[idx].comment_converted = _zip_cp437_to_utf8(ret, za->cdir->entry[idx].settable.comment_len,
									    &za->cdir->entry[idx].comment_converted_len, &za->error);
	    ret = za->cdir->entry[idx].comment_converted;
	    if (lenp != NULL)
		*lenp = za->cdir->entry[idx].comment_converted_len;
	}

	return ret;
    }

    /* already UTF-8, no conversion necessary */
    if (lenp != NULL)
	*lenp = za->entry[idx].changes.comment_len;
    return za->entry[idx].changes.comment;
}
