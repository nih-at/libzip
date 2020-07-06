# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.

#[=======================================================================[.rst:
FindNettle
-------

Finds the Nettle library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Nettle::Nettle``
  The Nettle library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Nettle_FOUND``
  True if the system has the Nettle library.
``Nettle_VERSION``
  The version of the Nettle library which was found.
``Nettle_INCLUDE_DIRS``
  Include directories needed to use Nettle.
``Nettle_LIBRARIES``
  Libraries needed to link to Nettle.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Nettle_INCLUDE_DIR``
  The directory containing ``foo.h``.
``Nettle_LIBRARY``
  The path to the Nettle library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_Nettle QUIET nettle)

find_path(Nettle_INCLUDE_DIR
  NAMES nettle/aes.h nettle/md5.h nettle/pbkdf2.h nettle/ripemd160.h nettle/sha.h
  PATHS ${PC_Nettle_INCLUDE_DIRS}
  PATH_SUFFIXES Nettle
)
find_library(Nettle_LIBRARY
  NAMES nettle
  PATHS ${PC_Nettle_LIBRARY_DIRS}
)

# Extract version information from the header file
if(Nettle_INCLUDE_DIR)
    file(STRINGS ${Nettle_INCLUDE_DIR}/nettle/version.h _ver_major_line
         REGEX "^#define NETTLE_VERSION_MAJOR  *[0-9]+"
         LIMIT_COUNT 1)
    string(REGEX MATCH "[0-9]+"
           Nettle_MAJOR_VERSION "${_ver_major_line}")
    file(STRINGS ${Nettle_INCLUDE_DIR}/nettle/version.h _ver_minor_line
         REGEX "^#define NETTLE_VERSION_MINOR  *[0-9]+"
         LIMIT_COUNT 1)
    string(REGEX MATCH "[0-9]+"
           Nettle_MINOR_VERSION "${_ver_minor_line}")
    set(Nettle_VERSION "${Nettle_MAJOR_VERSION}.${Nettle_MINOR_VERSION}")
    unset(_ver_major_line)
    unset(_ver_minor_line)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nettle
  FOUND_VAR Nettle_FOUND
  REQUIRED_VARS
    Nettle_LIBRARY
    Nettle_INCLUDE_DIR
  VERSION_VAR Nettle_VERSION
)

if(Nettle_FOUND)
  set(Nettle_LIBRARIES ${Nettle_LIBRARY})
  set(Nettle_INCLUDE_DIRS ${Nettle_INCLUDE_DIR})
  set(Nettle_DEFINITIONS ${PC_Nettle_CFLAGS_OTHER})
endif()

if(Nettle_FOUND AND NOT TARGET Nettle::Nettle)
  add_library(Nettle::Nettle UNKNOWN IMPORTED)
  set_target_properties(Nettle::Nettle PROPERTIES
    IMPORTED_LOCATION "${Nettle_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_Nettle_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Nettle_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  Nettle_INCLUDE_DIR
  Nettle_LIBRARY
)

# compatibility variables
set(Nettle_VERSION_STRING ${Nettle_VERSION})
