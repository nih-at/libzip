/*
  zip_bounds_safety.h -- portability macros for optional -fbounds-safety
  Copyright (C) 2026 Jeff Bindel <jeff@incrediblybased.co>

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <info@libzip.org>

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

  When ZIP_SUPPORT_FBOUNDS_SAFETY is defined (typically via
  -DZIP_SUPPORT_FBOUNDS_SAFETY and a Clang toolchain that implements
  -fbounds-safety), these macros expand to Clang bounds annotations.
  Otherwise they expand to nothing so default builds are unchanged.

  Pattern matches libwebp / libpng / giflib / lz4 / zstd inert-macro
  -fbounds-safety adoption: annotations are inert unless explicitly enabled.
*/

#ifndef _HAD_ZIP_BOUNDS_SAFETY_H
#define _HAD_ZIP_BOUNDS_SAFETY_H

#ifdef ZIP_SUPPORT_FBOUNDS_SAFETY

#  include <ptrcheck.h>
/* Non-ABI-breaking sized-by annotations for byte buffers whose companion
 * field / argument is a capacity in bytes (e.g. zip_buffer.size).
 * Prefer ZIP_SIZED_BY for buffers that are non-NULL when live; use
 * *_OR_NULL when the pointer may be NULL while the companion size is zero.
 */
#  define ZIP_SIZED_BY(n) __sized_by(n)
#  define ZIP_SIZED_BY_OR_NULL(n) __sized_by_or_null(n)
#  define ZIP_COUNTED_BY(n) __counted_by(n)
#  define ZIP_COUNTED_BY_OR_NULL(n) __counted_by_or_null(n)

#else /* !ZIP_SUPPORT_FBOUNDS_SAFETY */

#  define ZIP_SIZED_BY(n)
#  define ZIP_SIZED_BY_OR_NULL(n)
#  define ZIP_COUNTED_BY(n)
#  define ZIP_COUNTED_BY_OR_NULL(n)

#endif /* ZIP_SUPPORT_FBOUNDS_SAFETY */

#endif /* _HAD_ZIP_BOUNDS_SAFETY_H */
