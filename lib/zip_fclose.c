/*
  $NiH: zip_fclose.c,v 1.10 2004/04/16 09:40:28 dillo Exp $

  zip_fclose.c -- close file in zip archive
  Copyright (C) 1999, 2004 Dieter Baron and Thomas Klausner

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
zip_fclose(struct zip_file *zff)
{
    int i, ret;
    
    if (zff->zstr)
	inflateEnd(zff->zstr);
    free(zff->buffer);
    free(zff->zstr);

    for (i=0; i<zff->zf->nfile; i++) {
	if (zff->zf->file[i] == zff) {
	    zff->zf->file[i] = zff->zf->file[zff->zf->nfile-1];
	    zff->zf->nfile--;
	    break;
	}
    }

    /* if EOF, compare CRC */
    /* XXX: also compare for stored */
    if ((zff->flags & ZIP_ZF_COMP) == 0 && (zff->flags & ZIP_ZF_EOF))
	ret = (zff->crc_orig == zff->crc);
    else
	ret = zff->error.zip_err;

    free(zff);
    return ret;
}
