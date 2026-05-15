# - Try to find LibLZMA
# Once done this will define
#
#  LIBLZMA_FOUND - system has LibLZMA
#  LIBLZMA_INCLUDE_DIRS - the LibLZMA include directory
#  LIBLZMA_LIBRARIES - Link these to use LibLZMA
#  LIBLZMA_VERSION - The version of the LibLZMA library found

find_package(PkgConfig)
pkg_check_modules(PC_LIBLZMA QUIET liblzma)

find_path(LIBLZMA_INCLUDE_DIR lzma.h
    HINTS ${PC_LIBLZMA_INCLUDEDIR}
)

find_library(LIBLZMA_LIBRARY NAMES lzma
    HINTS ${PC_LIBLZMA_LIBDIR}
)

set(LIBLZMA_INCLUDE_DIRS ${LIBLZMA_INCLUDE_DIR})
set(LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})

if(LIBLZMA_INCLUDE_DIR AND EXISTS "${LIBLZMA_INCLUDE_DIR}/lzma.h")
    file(STRINGS "${LIBLZMA_INCLUDE_DIR}/lzma.h" LIBLZMA_VERSION_LINE REGEX "^#define LZMA_VERSION_STRING[ \t]+\".*\"")
    if(LIBLZMA_VERSION_LINE)
        string(REGEX REPLACE "^#define LZMA_VERSION_STRING[ \t]+\"([0-9.]+)\"" "\\1" LIBLZMA_VERSION "${LIBLZMA_VERSION_LINE}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibLZMA
    FOUND_VAR LIBLZMA_FOUND
    REQUIRED_VARS LIBLZMA_LIBRARY LIBLZMA_INCLUDE_DIR
    VERSION_VAR LIBLZMA_VERSION
)

if(LIBLZMA_FOUND AND NOT TARGET LibLZMA::LibLZMA)
    add_library(LibLZMA::LibLZMA UNKNOWN IMPORTED)
    set_target_properties(LibLZMA::LibLZMA PROPERTIES
        IMPORTED_LOCATION "${LIBLZMA_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBLZMA_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(LIBLZMA_INCLUDE_DIR LIBLZMA_LIBRARY)
