# Copyright (C) 2020 Dieter Baron and Thomas Klausner
#
# The authors can be contacted at <info@libzip.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# 3. The names of the authors may not be used to endorse or promote
#   products derived from this software without specific prior
#   written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindReadline
-------

Finds the readline library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``readline::libreadline_shared``
  The shared readline library
``readline::libreadline_static``
  The static readline library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``readline_FOUND``
  True if the system has the readline library.
``readline_VERSION``
  The version of the readline library which was found.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``READLINE_INCLUDE_DIR``
  The directory containing ``readline.h``.
``READLINE_STATIC_LIBRARY``
  The path to the readline static library.
``READLINE_SHARED_LIBRARY``
  The path to the readline shared library.
``READLINE_DLL``
  The path to the readline DLL.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_READLINE QUIET readline)

find_path(READLINE_INCLUDE_DIR
  NAMES readline/readline.h
  HINTS ${PC_READLINE_INCLUDE_DIRS}
)

find_file(READLINE_DLL
  NAMES libreadline.dll readline.dll
  PATH_SUFFIXES bin
  HINTS ${PC_READLINE_PREFIX}
)

# On Windows, we manually define the library names to avoid mistaking the
# implib for the static library
if(READLINE_DLL)
  set(_readline_win_static_name readline-static)
  set(_readline_win_shared_name readline)
else()
  # vcpkg removes the -static suffix in static builds
  set(_readline_win_static_name readline readline_static)
  set(_readline_win_shared_name)
endif()

set(_previous_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".dylib" ".dll.a" ".lib" ".tbd")
find_library(READLINE_SHARED_LIBRARY
  NAMES readline ${_readline_win_shared_name}
  HINTS ${PC_READLINE_LIBDIR}
)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
find_library(READLINE_STATIC_LIBRARY
  NAMES readline ${_readline_win_static_name}
  HINTS ${PC_READLINE_LIBDIR}
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_previous_suffixes})

# Set readline_LIBRARY to the shared library or fall back to the static library
if(READLINE_SHARED_LIBRARY)
  set(_READLINE_LIBRARY ${READLINE_SHARED_LIBRARY})
else()
  set(_READLINE_LIBRARY ${READLINE_STATIC_LIBRARY})
endif()

if(_READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
  # Check if readline links without additional libraries, or if it needs additional libraries.
  set(_READLINE_LINK_LIBRARIES)
  if(NOT WIN32 AND NOT CYGWIN AND _READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    include(CheckSymbolExists)
    set(_previous_required_includes ${CMAKE_REQUIRED_INCLUDES})
    set(_previous_required_libraries ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${READLINE_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${_READLINE_LIBRARY})

    # Test if readline links without additional libraries
    check_symbol_exists(readline readline/readline.h _READLINE_LINKS)

    if(NOT _READLINE_LINKS)
      for(lib in curses termcap)
        if(lib eq curses)
          find_package(Curses QUIET)
          if(CURSES_FOUND)
            set(additional_libraries ${CURSES_LIBRARIES})
          endif()
        elsif(lib eq termcap)
          find_library(additional_libraries NAMES termcap)
        endif()
        if(additional_libraries)
          set(_READLINE_LINKS)
          set(CMAKE_REQUIRED_LIBRARIES ${_READLINE_LIBRARY} ${additional_libraries})
          check_symbol_exists(readline readline/readline.h _READLINE_LINKS)
          if(_READLINE_LINKS)
            set(_READLINE_LINK_LIBRARIES ${additional_libraries})
            break()
          endif()
        endif()
        set(_READLINE_LINKS FALSE)
        set(additional_libraries)
      endfor()
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${_previous_required_libraries})
    set(CMAKE_REQUIRED_INCLUDES ${_previous_required_includes})
  endif()
endif()

if (NOT _READLINE_LINKS)
  # We couldn't find required dependencies, so we can't use the readline library.
  set(_READLINE_LIBRARY)
endif()

# Extract version information from the header file
if(READLINE_INCLUDE_DIR)
  # #define RL_READLINE_VERSION 	0x0402
  file(STRINGS ${READLINE_INCLUDE_DIR}/readline/readline.h _ver_line
    REGEX "^#define RL_READLINE_VERSION[ \t]+0x[0-9]+" LIMIT_COUNT 1)
  string(REGEX MATCH "0x(..)(..)" _ver_line "${_ver_line}")
  set(_ver_major "${CMAKE_MATCH_1}")
  set(_ver_minor "${CMAKE_MATCH_2}")
  string(REGEX REPLACE "^0" "" READLINE_MAJOR_VERSION "${_ver_major}")
  string(REGEX REPLACE "^0" "" READLINE_MINOR_VERSION "${_ver_minor}")
  set(READLINE_VERSION "${READLINE_MAJOR_VERSION}.${READLINE_MINOR_VERSION}")
  unset(_ver_line)
  unset(_ver_major)
  unset(_ver_minor)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Readline
  FOUND_VAR READLINE_FOUND
  REQUIRED_VARS
    _READLINE_LIBRARY
    READLINE_INCLUDE_DIR
  VERSION_VAR READLINE_VERSION
)

if(READLINE_FOUND AND READLINE_SHARED_LIBRARY AND NOT TARGET readline::libreadline_shared)
  add_library(readline::libreadline_shared SHARED IMPORTED)
  if(WIN32 OR CYGWIN)
    set_target_properties(readline::libreadline_shared PROPERTIES
      IMPORTED_LOCATION "${READLINE_DLL}"
      IMPORTED_IMPLIB "${READLINE_SHARED_LIBRARY}"
    )
  else()
    set_target_properties(readline::libreadline_shared PROPERTIES
      IMPORTED_LOCATION "${READLINE_SHARED_LIBRARY}"
    )
  endif()

  set_target_properties(readline::libreadline_shared PROPERTIES
    INTERFACE_COMPILE_OPTIONS "${PC_READLINE_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${READLINE_INCLUDE_DIR}"
  )

  if(_READLINE_LINK_LIBRARIES)
    set_target_properties(readline::libreadline_shared PROPERTIES
      INTERFACE_LINK_LIBRARIES "${_READLINE_LINK_LIBRARIES}"
    )
  endif()
endif()

if(READLINE_FOUND AND READLINE_STATIC_LIBRARY AND NOT TARGET readline::libreadline_static)
  add_library(readline::libreadline_static STATIC IMPORTED)
  set_target_properties(readline::libreadline_static PROPERTIES
    IMPORTED_LOCATION "${READLINE_STATIC_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_READLINE_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${READLINE_INCLUDE_DIR}"
  )

  if(_READLINE_LINK_LIBRARIES)
    set_target_properties(readline::libreadline_static PROPERTIES
      INTERFACE_LINK_LIBRARIES "${_READLINE_LINK_LIBRARIES}"
    )
  endif()
endif()

# Create an alias target for convenience: prefer shared over static
if(READLINE_FOUND AND NOT TARGET readline::readline)
  if(READLINE_SHARED_LIBRARY AND TARGET readline::libreadline_shared)
    add_library(readline::readline ALIAS readline::libreadline_shared)
  elseif(READLINE_STATIC_LIBRARY AND TARGET readline::libreadline_static)
    add_library(readline::readline ALIAS readline::libreadline_static)
  endif()
endif()

mark_as_advanced(
  READLINE_INCLUDE_DIR
  READLINE_DLL
  READLINE_SHARED_LIBRARY
  READLINE_STATIC_LIBRARY
  _READLINE_TERMCAP_LIBRARY
)
