AC_PREREQ([2.60])
AC_INIT([libzip],[1.1.2],[libzip@nih.at])
AC_CONFIG_SRCDIR([lib/zip_add.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

AC_CANONICAL_HOST
dnl Add rpath flags to installed pkg-config (.pc) file on systems that prefer it this way.
case $host_os in
    *bsd*)
        PKG_CONFIG_RPATH=-Wl,-R\${libdir};;
    *)
        PKG_CONFIG_RPATH=;;
esac
AC_SUBST(PKG_CONFIG_RPATH)


AC_PROG_CC
AC_SYS_LARGEFILE
AM_PROG_CC_C_O

AC_ARG_WITH(zlib,
    [  --with-zlib=PREFIX  specify prefix for ZLIB library],,
    with_zlib=yes)

if test "$with_zlib" != "yes"
then
    if test -f "$with_zlib"/zlib.h
    then
	# PREFIX is to uninstalled version in distribution directory
	CFLAGS="$CFLAGS -I$with_zlib"
	LDFLAGS="$LDFLAGS -L$with_zlib"
    else if test -f "$with_zlib"/include/zlib.h
    then
	# PREFIX is installation prefix
	CFLAGS="$CFLAGS -I$with_zlib/include"
	LDFLAGS="$LDFLAGS -L$with_zlib/lib"
    fi
    fi
fi

AC_CHECK_LIB(z, main)
AC_CACHE_CHECK(new ZLIB version, id_cv_lib_zlib_ok, 
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <zlib.h>]], [[extern ZEXPORT int unzOpen (const char *path);]])],[id_cv_lib_zlib_ok=yes],[id_cv_lib_zlib_ok=no]))
if test "$id_cv_lib_zlib_ok" = "no"
then
    AC_MSG_ERROR([ZLIB version too old, please install at least v1.1.2])
fi

AC_EXEEXT

LT_INIT

AC_CHECK_FUNCS([_close _dup _fdopen _fileno _open _setmode _snprintf _strdup _stricmp fileno fseeko ftello getopt getprogname open setmode snprintf strcasecmp strdup stricmp])
AC_CHECK_FUNCS([mkstemp], [], [AC_LIBOBJ(mkstemp)])

AC_CACHE_CHECK([if libc defines __progname], ac_cv_libc_defines___progname,
	       [ AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],
						 [[ extern char *__progname; printf("%s", __progname); ]])],
						 [ ac_cv_libc_defines___progname="yes" ],
						 [ ac_cv_libc_defines___progname="no" ])
	       ])
if test "x$ac_cv_libc_defines___progname" = "xyes" ; then
    AC_DEFINE([HAVE___PROGNAME], [1], [Define if libc defines __progname])
fi


AC_CHECK_HEADERS([fts.h stdbool.h strings.h unistd.h])

AC_CHECK_TYPES([int8_t])
AC_CHECK_TYPES([int16_t])
AC_CHECK_TYPES([int32_t])
AC_CHECK_TYPES([int64_t])
AC_CHECK_TYPES([uint8_t])
AC_CHECK_TYPES([uint16_t])
AC_CHECK_TYPES([uint32_t])
AC_CHECK_TYPES([uint64_t])
AC_CHECK_TYPES([ssize_t])

AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([off_t])
AC_CHECK_SIZEOF([size_t])

AC_STRUCT_TIMEZONE

case $host_os
in
    *bsd*) MANFMT=mdoc;;
    *) MANFMT=man;;
esac
AC_SUBST([MANFMT])

AH_BOTTOM([
#ifndef HAVE_SSIZE_T
#  if SIZEOF_SIZE_T == SIZEOF_INT
typedef int ssize_t;
#  elif SIZEOF_SIZE_T == SIZEOF_LONG
typedef long ssize_t;
#  elif SIZEOF_SIZE_T == SIZEOF_LONG_LONG
typedef long long ssize_t;
#  else
#error no suitable type for ssize_t found
#  endif
#endif
])

gl_VISIBILITY

AC_CONFIG_FILES([Makefile
		 libzip.pc
		 libzip-uninstalled.pc
		 examples/Makefile
		 man/Makefile
		 lib/Makefile
		 regress/Makefile
		 src/Makefile])
AC_OUTPUT
