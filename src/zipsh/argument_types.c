#include "zipsh.h"

/* clang-format off */
static argument_type_enum_entry_t compression_method_entries[] = {
    {"default", ZIP_CM_DEFAULT },
    {"store", ZIP_CM_STORE },
    {"deflate", ZIP_CM_DEFLATE },
#if defined(HAVE_LIBBZ2)
    {"bzip2", ZIP_CM_BZIP2 },
#endif
#if defined(HAVE_LIBLZMA)
    /*  Disabled - because 7z isn't able to unpack ZIP+LZMA ZIP+LZMA2
        archives made this way - and vice versa.

        {"lzma2", ZIP_CM_LZMA2},
    */
    {"lzma", ZIP_CM_LZMA },
    {"xz", ZIP_CM_XZ },
#endif
#if defined(HAVE_LIBZSTD)
    {"zstd", ZIP_CM_ZSTD },
#endif
    {NULL, {0}}
};

static argument_type_enum_entry_t encryption_method_entries[] = {
    {"none", ZIP_EM_NONE },
    {"PKWARE", ZIP_EM_TRAD_PKWARE },
    {"AES-128", ZIP_EM_AES_128 },
    {"AES-192", ZIP_EM_AES_192 },
    {"AES-256", ZIP_EM_AES_256 },
    {NULL, {0}}
};
/* clang-format on */

const argument_type_t argument_type_compression_method_instance = ARGUMENT_TYPE_ENUM("compression_method", compression_method_entries);
const argument_type_t argument_type_encryption_method_instance = ARGUMENT_TYPE_ENUM("encryption_method", encryption_method_entries);
