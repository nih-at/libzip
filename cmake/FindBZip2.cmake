# - Try to find BZip2
# Once done this will define
#
#  BZIP2_FOUND - system has BZip2
#  BZIP2_INCLUDE_DIRS - the BZip2 include directory
#  BZIP2_LIBRARIES - Link these to use BZip2
#  BZIP2_VERSION - The version of the BZip2 library found

find_package(PkgConfig)
pkg_check_modules(PC_BZIP2 QUIET bzip2)

find_path(BZIP2_INCLUDE_DIR bzlib.h
    HINTS ${PC_BZIP2_INCLUDEDIR}
    PATHS /usr/include /usr/local/include
)

find_library(BZIP2_LIBRARY NAMES bz2
    HINTS ${PC_BZIP2_LIBDIR}
    PATHS /usr/lib /usr/local/lib /lib
    PATH_SUFFIXES x86_64-linux-gnu
)

set(BZIP2_INCLUDE_DIRS ${BZIP2_INCLUDE_DIR})
set(BZIP2_LIBRARIES ${BZIP2_LIBRARY})

if(BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")
    file(STRINGS "${BZIP2_INCLUDE_DIR}/bzlib.h" BZIP2_VERSION_LINE REGEX "^#define BZ_VERSION[ \t]+\".*\"")
    if(BZIP2_VERSION_LINE)
        string(REGEX REPLACE "^#define BZ_VERSION[ \t]+\"([0-9.]+).*\"" "\\1" BZIP2_VERSION "${BZIP2_VERSION_LINE}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BZip2
    FOUND_VAR BZIP2_FOUND
    REQUIRED_VARS BZIP2_LIBRARY BZIP2_INCLUDE_DIR
    VERSION_VAR BZIP2_VERSION
)

if(BZIP2_FOUND AND NOT TARGET BZip2::BZip2)
    add_library(BZip2::BZip2 UNKNOWN IMPORTED)
    set_target_properties(BZip2::BZip2 PROPERTIES
        IMPORTED_LOCATION "${BZIP2_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${BZIP2_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(BZIP2_INCLUDE_DIR BZIP2_LIBRARY)
