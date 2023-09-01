/*
  This file was generated automatically by CMake
  from zip.h and zipint.h\; make changes there.
*/

#include "zipint.h"

#define L ZIP_ET_LIBZIP
#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZLIB

#define E ZIP_DETAIL_ET_ENTRY
#define G ZIP_DETAIL_ET_GLOBAL

const struct _zip_err_info _zip_err_str[] = {{N, "ZIP_ER_OK"}, {N, "ZIP_ER_MULTIDISK"}, {S, "ZIP_ER_RENAME"}, {S, "ZIP_ER_CLOSE"}, {S, "ZIP_ER_SEEK"}, {S, "ZIP_ER_READ"}, {S, "ZIP_ER_WRITE"}, {N, "ZIP_ER_CRC"}, {N, "ZIP_ER_ZIPCLOSED"}, {N, "ZIP_ER_NOENT"}, {N, "ZIP_ER_EXISTS"}, {S, "ZIP_ER_OPEN"}, {S, "ZIP_ER_TMPOPEN"}, {Z, "ZIP_ER_ZLIB"}, {N, "ZIP_ER_MEMORY"}, {N, "ZIP_ER_CHANGED"}, {N, "ZIP_ER_COMPNOTSUPP"}, {N, "ZIP_ER_EOF"}, {N, "ZIP_ER_INVAL"}, {N, "ZIP_ER_NOZIP"}, {N, "ZIP_ER_INTERNAL"}, {L, "ZIP_ER_INCONS"}, {S, "ZIP_ER_REMOVE"}, {N, "ZIP_ER_DELETED"}, {N, "ZIP_ER_ENCRNOTSUPP"}, {N, "ZIP_ER_RDONLY"}, {N, "ZIP_ER_NOPASSWD"}, {N, "ZIP_ER_WRONGPASSWD"}, {N, "ZIP_ER_OPNOTSUPP"}, {N, "ZIP_ER_INUSE"}, {S, "ZIP_ER_TELL"}, {N, "ZIP_ER_COMPRESSED_DATA"}, {N, "ZIP_ER_CANCELLED"}};

const int _zip_err_str_count = sizeof(_zip_err_str)/sizeof(_zip_err_str[0]);

const struct _zip_err_info _zip_err_details[] = {{G, "ZIP_ER_DETAIL_NO_DETAIL"}, {G, "ZIP_ER_DETAIL_CDIR_OVERLAPS_EOCD"}, {G, "ZIP_ER_DETAIL_COMMENT_LENGTH_INVALID"}, {G, "ZIP_ER_DETAIL_CDIR_LENGTH_INVALID"}, {E, "ZIP_ER_DETAIL_CDIR_ENTRY_INVALID"}, {G, "ZIP_ER_DETAIL_CDIR_WRONG_ENTRIES_COUNT"}, {E, "ZIP_ER_DETAIL_ENTRY_HEADER_MISMATCH"}, {G, "ZIP_ER_DETAIL_EOCD_LENGTH_INVALID"}, {G, "ZIP_ER_DETAIL_EOCD64_OVERLAPS_EOCD"}, {G, "ZIP_ER_DETAIL_EOCD64_WRONG_MAGIC"}, {G, "ZIP_ER_DETAIL_EOCD64_MISMATCH"}, {G, "ZIP_ER_DETAIL_CDIR_INVALID"}, {E, "ZIP_ER_DETAIL_VARIABLE_SIZE_OVERFLOW"}, {E, "ZIP_ER_DETAIL_INVALID_UTF8_IN_FILENAME"}, {E, "ZIP_ER_DETAIL_INVALID_UTF8_IN_COMMENT"}, {E, "ZIP_ER_DETAIL_INVALID_ZIP64_EF"}, {E, "ZIP_ER_DETAIL_INVALID_WINZIPAES_EF"}, {E, "ZIP_ER_DETAIL_EF_TRAILING_GARBAGE"}, {E, "ZIP_ER_DETAIL_INVALID_EF_LENGTH"}, {E, "ZIP_ER_DETAIL_INVALID_FILE_LENGTH"}};

const int _zip_err_details_count = sizeof(_zip_err_details) / sizeof(_zip_err_details[0]);