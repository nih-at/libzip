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



const char * const zip_err_str[] = {
EOF

sed -n  '/^#define ZERR_/ s/.*\/\* \([^*]*\) \*\//    "\1",/p' "$1" \
    >> "$2.$$" || exit 1
echo '};' >> "$2.$$" || exit 1

mv "$2.$$" "$2" || exit 1
