#!/bin/sh

if [ "$#" -ne 2 ]
then
    echo "Usage: $0 in_file out_file" >&2
    echo "       e.g. $0 zip.h zip_err_str.c" >&2
    exit 1
fi

if [ "$1" = "$2" ]
then
    echo "$0: error: output file = input file" >&2
    exit 1
fi

cat <<EOF >> "$2.$$" || exit 1
/* This file was generated automatically by $0
   from $1; make changes there. */

#include "zip.h"
#include "zipint.h"



const char * const zip_err_str[] = {
EOF

sed -n  '/^#define ZERR_/ s/.*\/\* . \([^*]*\) \*\//    "\1",/p' "$1" \
    >> "$2.$$" || exit 1

cat <<EOF >> "$2.$$" || exit 1
};

const int zip_nerr_str = sizeof(zip_err_str)/sizeof(zip_err_str[0]);

#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZIP

const int _zip_err_type[] = {
EOF

sed -n  '/^#define ZERR_/ s/.*\/\* \(.\) \([^*]*\) \*\//    \1,/p' "$1" \
    >> "$2.$$" || exit 1

echo '};' >> "$2.$$" || exit 1

mv "$2.$$" "$2" || exit 1
