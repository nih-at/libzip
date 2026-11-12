# Utility function to generate command table source files
# from a set of source files containing COMMAND definitions

function(generate_command_table)
  set(options)
  set(oneValueArgs OUTPUT_FILE)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(GCT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT GCT_OUTPUT_FILE)
    message(FATAL_ERROR "generate_command_table: OUTPUT_FILE is required")
  endif()

  if(NOT GCT_SOURCES)
    message(FATAL_ERROR "generate_command_table: SOURCES is required")
  endif()

  # Convert relative paths to absolute paths
  set(SOURCES_ABSOLUTE)
  foreach(source IN LISTS GCT_SOURCES)
    if(IS_ABSOLUTE "${source}")
      list(APPEND SOURCES_ABSOLUTE "${source}")
    else()
      list(APPEND SOURCES_ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    endif()
  endforeach()

  # Prepare escaped source list for script mode (-D parameter)
  if(NOT WIN32)
    string(REPLACE ";" "\\;" SOURCES_ABS_ESCAPED "${SOURCES_ABSOLUTE}")
  else()
    set(SOURCES_ABS_ESCAPED "${SOURCES_ABSOLUTE}")
  endif()

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GCT_OUTPUT_FILE}
    COMMAND "${CMAKE_COMMAND}"
      "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
      "-DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
      "-DSOURCES=${SOURCES_ABS_ESCAPED}"
      "-DOUTPUT_FILE=${GCT_OUTPUT_FILE}"
      "-P" "${PROJECT_SOURCE_DIR}/cmake/GenerateCommandTableImpl.cmake"
    DEPENDS
      ${PROJECT_SOURCE_DIR}/cmake/GenerateCommandTableImpl.cmake
      ${SOURCES_ABSOLUTE}
    COMMENT "Generating ${GCT_OUTPUT_FILE}"
  )
endfunction()
