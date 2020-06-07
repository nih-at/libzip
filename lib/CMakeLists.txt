INCLUDE(CheckFunctionExists)

IF(LIBZIP_DO_INSTALL)
  INSTALL(FILES zip.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
ENDIF()

SET(CMAKE_C_VISIBILITY_PRESET hidden)

SET(LIBZIP_SOURCES
  zip_add.c
  zip_add_dir.c
  zip_add_entry.c
  zip_algorithm_deflate.c
  zip_buffer.c
  zip_close.c
  zip_delete.c
  zip_dir_add.c
  zip_dirent.c
  zip_discard.c
  zip_entry.c
  zip_error.c
  zip_error_clear.c
  zip_error_get.c
  zip_error_get_sys_type.c
  zip_error_strerror.c
  zip_error_to_str.c
  zip_extra_field.c
  zip_extra_field_api.c
  zip_fclose.c
  zip_fdopen.c
  zip_file_add.c
  zip_file_error_clear.c
  zip_file_error_get.c
  zip_file_get_comment.c
  zip_file_get_external_attributes.c
  zip_file_get_offset.c
  zip_file_rename.c
  zip_file_replace.c
  zip_file_set_comment.c
  zip_file_set_encryption.c
  zip_file_set_external_attributes.c
  zip_file_set_mtime.c
  zip_file_strerror.c
  zip_filerange_crc.c
  zip_fopen.c
  zip_fopen_encrypted.c
  zip_fopen_index.c
  zip_fopen_index_encrypted.c
  zip_fread.c
  zip_fseek.c
  zip_ftell.c
  zip_get_archive_comment.c
  zip_get_archive_flag.c
  zip_get_encryption_implementation.c
  zip_get_file_comment.c
  zip_get_name.c
  zip_get_num_entries.c
  zip_get_num_files.c
  zip_hash.c
  zip_io_util.c
  zip_libzip_version.c
  zip_memdup.c
  zip_name_locate.c
  zip_new.c
  zip_open.c
  zip_pkware.c
  zip_progress.c
  zip_rename.c
  zip_replace.c
  zip_set_archive_comment.c
  zip_set_archive_flag.c
  zip_set_default_password.c
  zip_set_file_comment.c
  zip_set_file_compression.c
  zip_set_name.c
  zip_source_accept_empty.c
  zip_source_begin_write.c
  zip_source_begin_write_cloning.c
  zip_source_buffer.c
  zip_source_call.c
  zip_source_close.c
  zip_source_commit_write.c
  zip_source_compress.c
  zip_source_crc.c
  zip_source_error.c
  zip_source_file_common.c
  zip_source_file_stdio.c
  zip_source_free.c
  zip_source_function.c
  zip_source_get_file_attributes.c
  zip_source_is_deleted.c
  zip_source_layered.c
  zip_source_open.c
  zip_source_pkware_decode.c
  zip_source_pkware_encode.c
  zip_source_read.c
  zip_source_remove.c
  zip_source_rollback_write.c
  zip_source_seek.c
  zip_source_seek_write.c
  zip_source_stat.c
  zip_source_supports.c
  zip_source_tell.c
  zip_source_tell_write.c
  zip_source_window.c
  zip_source_write.c
  zip_source_zip.c
  zip_source_zip_new.c
  zip_stat.c
  zip_stat_index.c
  zip_stat_init.c
  zip_strerror.c
  zip_string.c
  zip_unchange.c
  zip_unchange_all.c
  zip_unchange_archive.c
  zip_unchange_data.c
  zip_utf-8.c
)

IF(WIN32)
  SET(LIBZIP_OPSYS_FILES
    zip_source_file_win32.c
    zip_source_file_win32_named.c
    zip_source_file_win32_utf16.c
    zip_source_file_win32_utf8.c
  )
  IF(CMAKE_SYSTEM_NAME MATCHES WindowsPhone OR CMAKE_SYSTEM_NAME MATCHES WindowsStore)
    SET(LIBZIP_OPSYS_FILES "${LIBZIP_OPSYS_FILES}"
      zip_random_uwp.c
    )
  ELSE()
    SET(LIBZIP_OPSYS_FILES "${LIBZIP_OPSYS_FILES}"
      zip_source_file_win32_ansi.c
      zip_random_win32.c
    )
  ENDIF()
ELSE(WIN32)
  SET(LIBZIP_OPSYS_FILES
    zip_mkstempm.c
    zip_source_file_stdio_named.c
    zip_random_unix.c
  )
ENDIF(WIN32)

INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})

file(READ ${PROJECT_SOURCE_DIR}/lib/zip.h zip_h)
string(REGEX MATCHALL "#define ZIP_ER_([A-Z_]+) ([0-9]+)[ \t]+/([-*0-9a-zA-Z ']*)/" zip_h_err ${zip_h})
set(zip_err_str [=[
/*
   This file was generated automatically by CMake
   from zip.h\; make changes there.
 */

#include "zipint.h"

const char * const _zip_err_str[] = {
]=])
set(zip_err_type)
foreach(errln ${zip_h_err})
  string(REGEX MATCH "#define ZIP_ER_([A-Z_]+) ([0-9]+)[ \t]+/([-*0-9a-zA-Z ']*)/" err_t_tt ${errln})
  string(REGEX MATCH "([N|S|Z]+) ([-0-9a-zA-Z ']*)" err_t_tt "${CMAKE_MATCH_3}")
  string(APPEND zip_err_type "    ${CMAKE_MATCH_1},\n")
  string(STRIP "${CMAKE_MATCH_2}" err_t_tt)
  string(APPEND zip_err_str "    \"${err_t_tt}\",\n")
endforeach()
string(APPEND zip_err_str [=[}\;

const int _zip_nerr_str = sizeof(_zip_err_str)/sizeof(_zip_err_str[0])\;

#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZLIB

const int _zip_err_type[] = {
]=])
string(APPEND zip_err_str "${zip_err_type}}\;\n")
file(WRITE ${CMAKE_BINARY_DIR}/zip_err_str.c ${zip_err_str})
list(APPEND LIBZIP_SOURCES ${CMAKE_BINARY_DIR}/zip_err_str.c)

IF(HAVE_LIBBZ2)
  SET(LIBZIP_OPTIONAL_FILES zip_algorithm_bzip2.c)
ENDIF()

IF(HAVE_LIBLZMA)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_algorithm_xz.c)
ENDIF()

IF(HAVE_COMMONCRYPTO)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_crypto_commoncrypto.c
)
ELSEIF(HAVE_WINDOWS_CRYPTO)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_crypto_win.c
)
ELSEIF(HAVE_GNUTLS)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_crypto_gnutls.c
)
ELSEIF(HAVE_OPENSSL)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_crypto_openssl.c
)
ELSEIF(HAVE_MBEDTLS)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_crypto_mbedtls.c
)
ENDIF()

IF(HAVE_CRYPTO)
  SET(LIBZIP_OPTIONAL_FILES ${LIBZIP_OPTIONAL_FILES} zip_winzip_aes.c zip_source_winzip_aes_decode.c zip_source_winzip_aes_encode.c
)
ENDIF()

ADD_LIBRARY(zip ${LIBZIP_SOURCES} ${LIBZIP_EXTRA_FILES} ${LIBZIP_OPTIONAL_FILES} ${LIBZIP_OPSYS_FILES})

IF(SHARED_LIB_VERSIONNING)
SET_TARGET_PROPERTIES(zip PROPERTIES VERSION 5.3 SOVERSION 5)
ENDIF()

TARGET_LINK_LIBRARIES(zip PRIVATE ${ZLIB_LIBRARIES} ${OPTIONAL_LIBRARY})

IF(LIBZIP_DO_INSTALL)
  INSTALL(TARGETS zip
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF()
