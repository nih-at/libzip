/*
  $NiH: zip_merge_meta.c,v 1.8 2003/10/02 14:13:30 dillo Exp $

  zip_merge_meta.c -- merge two meta information structures
  Copyright (C) 1999 Dieter Baron and Thomas Klausner

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



#include <stdlib.h>
#include "zip.h"
#include "zipint.h"

int
_zip_merge_meta_fix(struct zip_meta *dest, struct zip_meta *src)
{
    if (!src)
	return 0;
    if (!dest) {
	/* XXX: zip_err = ZERR_INTERNAL; */
	return -1;
    }
    
    if (src->version_made != (unsigned short)-1)
	dest->version_made = src->version_made;
    if (src->version_need != (unsigned short)-1)
	dest->version_need = src->version_need;
    if (src->bitflags != (unsigned short)-1)
	dest->bitflags = src->bitflags;
    if (src->comp_method != (unsigned short)-1)
	dest->comp_method = src->comp_method;
    if (src->disknrstart != (unsigned short)-1)
	dest->disknrstart = src->disknrstart;
    if (src->int_attr != (unsigned short)-1)
	dest->int_attr = src->int_attr;
    
    if (src->crc != (unsigned int)-1)
	dest->crc = src->crc;
    if (src->uncomp_size != (unsigned int)-1)
	dest->uncomp_size = src->uncomp_size;
    if (src->comp_size != (unsigned int)-1)
	dest->comp_size = src->comp_size;
    if (src->ext_attr != (unsigned int)-1)
	dest->ext_attr = src->ext_attr;
    if (src->local_offset != (unsigned int)-1)
	dest->local_offset = src->local_offset;

    if (src->last_mod != 0)
	dest->last_mod = src->last_mod;
    
    return 0;
}



int
_zip_merge_meta(struct zip_meta *dest, struct zip_meta *src)
{
    if (!src)
	return 0;
    if (!dest) {
	/* XXX: zip_err = ZERR_INTERNAL; */
	return -1;
    }

    _zip_merge_meta_fix(dest, src);
    
    if ((src->ef_len != (unsigned short)-1) && src->ef) {
	free(dest->ef);
	dest->ef = _zip_memdup(src->ef, src->ef_len);
	if (!dest->ef) {
	    /* zip_err = ZERR_MEMORY; */
	    return -1;
	}
	dest->ef_len = src->ef_len;
    }

    if ((src->lef_len != (unsigned short)-1) && src->lef) {
	free(dest->lef);
	dest->lef = _zip_memdup(src->lef, src->lef_len);
	if (!dest->lef) {
	    /* zip_err = ZERR_MEMORY; */
	    return -1;
	}
	dest->lef_len = src->lef_len;
    }

    if ((src->fc_len != (unsigned short)-1) && src->fc) {
	free(dest->fc);
	dest->fc = _zip_memdup(src->fc, src->fc_len);
	if (!dest->fc) {
	    /* zip_err = ZERR_MEMORY; */
	    return -1;
	}
	dest->fc_len = src->fc_len;
    }

    return 0;
}
