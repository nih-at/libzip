AC_PREREQ(2.54)
AC_INIT([libzip],[0.5a],[nih@giga.or.at])
AC_CONFIG_SRCDIR([lib/zip_add.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

dnl AC_CANONICAL_HOST

# We want these before the checks, so the checks can modify their values.
dnl test -z "$CFLAGS" && CFLAGS='-Wall -g' auto_cflags=1


AC_PROG_CC
AC_PROG_RANLIB


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
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <zlib.h>]], [[extern ZEXPORT unzOpen (const char *path);]])],[id_cv_lib_zlib_ok=yes],[id_cv_lib_zlib_ok=no]))
if test "$id_cv_lib_zlib_ok" = "no"
then
    AC_MSG_ERROR([ZLIB version too old, please install at least v1.1.2])
fi

dnl case $host_os
dnl in
dnl 	*bsd*) MANFMT=mdoc;;
dnl 	*) MANFMT=man;;
dnl esac

AC_EXEEXT

dnl AM_DISABLE_STATIC
AM_PROG_LIBTOOL

case $host_os
in
    *bsd*) MANFMT=mdoc;;
    *) MANFMT=man;;
esac

AC_CONFIG_LINKS([man/zipcmp.1:man/zipcmp.${MANFMT}
		 man/zip_close.3:man/zip_close.${MANFMT}
		 man/zip_delete.3:man/zip_delete.${MANFMT}
		 man/zip_unchange.3:man/zip_unchange.${MANFMT}
		 man/zip_unchange_all.3:man/zip_unchange_all.${MANFMT}
		 man/zip_open.3:man/zip_open.${MANFMT}])

AC_CONFIG_FILES([Makefile
		 libzip.pc
		 libzip-uninstalled.pc
		 man/Makefile
		 lib/Makefile
		 regress/Makefile
		 src/Makefile])
AC_OUTPUT
