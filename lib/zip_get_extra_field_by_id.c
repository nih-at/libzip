/*
  zip_get_extra_field_by_id.c -- get extrafield by ID
  Copyright (C) 2012 Dieter Baron and Thomas Klausner

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



#include "zipint.h"



const zip_uint8_t *
zip_get_extra_field_by_id(struct zip *za, int idx, int flags,
			  zip_uint16_t id, int id_index, zip_uint16_t *lenp)
{
    static zip_uint8_t empty[1];
    
    const zip_uint8_t *ef;
    int len;

    len = 17;
    ef = (const zip_uint8_t *)zip_get_file_extra(za, idx, &len, flags);
    
    if (ef == NULL) {
	if (len == 17)
	    return NULL;
	if (lenp)
	    *lenp = 0;
	return empty;
    }

    return _zip_extract_extra_field_by_id(&za->error, id, id_index, ef, len, lenp);
}
    

const zip_uint8_t *
_zip_extract_extra_field_by_id(struct zip_error *errorp, zip_uint16_t id, int id_index,
			       const zip_uint8_t *ef, zip_uint16_t ef_len, zip_uint16_t *lenp)
{
    int i;
    const zip_uint8_t *p;
    zip_uint16_t fid, flen;

    i = 0;
    for (p=ef; p<ef+ef_len; p+=flen+4) {
	if (p+4 > ef+ef_len) {
	    _zip_error_set(errorp, ZIP_ER_INCONS, 0);
	    return NULL;
	}

	fid = p[0]+(p[1]<<8);
	flen = p[2]+(p[3]<<8);

	if (p+flen+4 > ef+ef_len) {
	    _zip_error_set(errorp, ZIP_ER_INCONS, 0);
	    return NULL;
	}

	if (fid == id) {
	    if (i < id_index) {
		i++;
		continue;
	    }

	    if (lenp)
		*lenp = flen;
	    return p+4;
	}
    }

    _zip_error_set(errorp, ZIP_ER_NOENT, 0);
    return NULL;
}

