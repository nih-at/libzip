#[=======================================================================[.rst:
FindEditline
------------

Finds the editline library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``editline::editline``
  The editline library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``editline_FOUND``
  True if the system has the editline library.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``EDITLINE_INCLUDE_DIR``
  The directory containing ``histedit.h``.
``EDITLINE_LIBRARY``
  The path to the editline library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_EDITLINE QUIET editline)

find_path(EDITLINE_INCLUDE_DIR
  NAMES histedit.h
  HINTS ${PC_EDITLINE_INCLUDE_DIRS}
)

find_library(EDITLINE_LIBRARY
  NAMES edit
  HINTS ${PC_EDITLINE_LIBDIR}
)

if(EDITLINE_LIBRARY AND EDITLINE_INCLUDE_DIR)
  # Check if editline links without additional libraries, or if it needs additional libraries.
  set(_EDITLINE_LINK_LIBRARIES)
  if(NOT WIN32 AND NOT CYGWIN)
    include(CheckSymbolExists)
    set(_previous_required_includes ${CMAKE_REQUIRED_INCLUDES})
    set(_previous_required_libraries ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${EDITLINE_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${EDITLINE_LIBRARY})

    # Test if editline links without additional libraries
    check_symbol_exists(el_init histedit.h _EDITLINE_LINKS)

    if(NOT _EDITLINE_LINKS)
      foreach(lib curses termcap)
        set(additional_libraries)
        if(lib STREQUAL curses)
          find_package(Curses QUIET)
          if(CURSES_FOUND)
            set(additional_libraries ${CURSES_LIBRARIES})
          endif()
        elseif(lib STREQUAL termcap)
          find_library(additional_libraries NAMES termcap)
        endif()
        if(additional_libraries)
          set(_EDITLINE_LINKS FALSE)
          set(CMAKE_REQUIRED_LIBRARIES ${EDITLINE_LIBRARY} ${additional_libraries})
          check_symbol_exists(el_init histedit.h _EDITLINE_LINKS)
          if(_EDITLINE_LINKS)
            set(_EDITLINE_LINK_LIBRARIES ${additional_libraries})
            break()
          endif()
        endif()
      endforeach()
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${_previous_required_libraries})
    set(CMAKE_REQUIRED_INCLUDES ${_previous_required_includes})
  endif()
endif()

if(NOT _EDITLINE_LINKS)
  # We couldn't find required dependencies, so we can't use the editline library.
  set(EDITLINE_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Editline
  FOUND_VAR EDITLINE_FOUND
  REQUIRED_VARS
    EDITLINE_LIBRARY
    EDITLINE_INCLUDE_DIR
)

if(EDITLINE_FOUND AND NOT TARGET editline::editline)
  add_library(editline::editline SHARED IMPORTED)
  set_target_properties(editline::editline PROPERTIES
    IMPORTED_LOCATION "${EDITLINE_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_EDITLINE_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${EDITLINE_INCLUDE_DIR}"
  )

  if(_EDITLINE_LINK_LIBRARIES)
    set_target_properties(editline::editline PROPERTIES
      INTERFACE_LINK_LIBRARIES "${_EDITLINE_LINK_LIBRARIES}"
    )
  endif()
endif()

mark_as_advanced(
  EDITLINE_INCLUDE_DIR
  EDITLINE_LIBRARY
)
