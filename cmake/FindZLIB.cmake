# - Find ZLIB
# Find the native ZLIB includes and library.
#
#  ZLIB_INCLUDE_DIRS - where to find zlib.h, etc.
#  ZLIB_LIBRARIES    - List of libraries when using zlib.
#  ZLIB_FOUND        - True if zlib found.

find_package(PkgConfig)
pkg_check_modules(PC_ZLIB QUIET zlib)

find_path(ZLIB_INCLUDE_DIR zlib.h
    HINTS ${PC_ZLIB_INCLUDEDIR} ${PC_ZLIB_INCLUDE_DIRS})

find_library(ZLIB_LIBRARY NAMES z zlib
    HINTS ${PC_ZLIB_LIBDIR} ${PC_ZLIB_LIBRARY_DIRS})

set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB DEFAULT_MSG
    ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

if(ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
    set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION "${ZLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
endif()

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)
