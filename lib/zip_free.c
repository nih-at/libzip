/*
  $NiH: zip_free.c,v 1.6 2003/10/02 14:13:30 dillo Exp $

  zip_free.c -- free struct zip
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



/* _zip_free:
   frees the space allocated to a zipfile struct, and closes the
   corresponding file. */

void
_zip_free(struct zip *zf)
{
    int i;

    if (zf == NULL)
	return 0;

    if (zf->zn)
	free(zf->zn);

    if (zf->zp)
	fclose(zf->zp);

    if (zf->com)
	free(zf->com);

    if (zf->entry) {
	for (i=0; i<zf->nentry; i++) {
	    _zip_free_entry(zf->entry+i);
	}
	free (zf->entry);
    }

    for (i=0; i<zf->nfile; i++) {
	zf->file[i]->flags = ZERR_ZIPCLOSED;
	zf->file[i]->zf = NULL;
	zf->file[i]->name = NULL;
    }

    free(zf->file);
    
    free(zf);

    return;
}
