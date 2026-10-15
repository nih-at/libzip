set(SHELL_SOURCES
  shell/argument.c
  shell/command.c
  shell/error_message.c
  shell/help.c
  shell/shell.c
  shell/words.c
)

set(ZIPSH_SOURCES
  zipsh/cat.c
  zipsh/set_default_password.c
  zipsh/zipsh.c
)


add_library(shell STATIC ${SHELL_SOURCES})
target_include_directories(shell PRIVATE shell ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})
if (ENABLE_COVERAGE)
  target_compile_options(shell PRIVATE -coverage)
  target_link_options(shell PRIVATE -coverage)
endif()

check_function_exists(getopt HAVE_GETOPT)
foreach(PROGRAM zipcmp zipmerge ziptool)
  add_executable(${PROGRAM} ${PROGRAM}.c)
  target_link_libraries(${PROGRAM} zip)
  target_include_directories(${PROGRAM} PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})
  if(LIBZIP_DO_INSTALL)
    install(TARGETS ${PROGRAM} EXPORT ${PROJECT_NAME}-bin-targets DESTINATION bin)
  endif()
  if(NOT HAVE_GETOPT)
    target_sources(${PROGRAM} PRIVATE getopt.c)
  endif(NOT HAVE_GETOPT)
  if (ENABLE_COVERAGE)
    target_compile_options(${PROGRAM} PRIVATE -coverage)
    target_link_options(${PROGRAM} PRIVATE -coverage)
  endif()
endforeach()
target_sources(zipcmp PRIVATE diff_output.c)
target_link_libraries(zipcmp ${FTS_LIB} ZLIB::ZLIB)


add_executable(zipsh ${ZIPSH_SOURCES} zipsh_commands.c)
if(NOT HAVE_GETOPT)
  target_sources(zipsh PRIVATE getopt.c)
endif(NOT HAVE_GETOPT)
target_link_libraries(zipsh shell zip)
target_include_directories(zipsh PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib shell zipsh ${PROJECT_BINARY_DIR})
if(LIBZIP_DO_INSTALL)
  install(TARGETS zipsh EXPORT ${PROJECT_NAME}-bin-targets DESTINATION bin)
endif()
if (ENABLE_COVERAGE)
  target_compile_options(zipsh PRIVATE -coverage)
  target_link_options(zipsh PRIVATE -coverage)
endif()

# create zipsh_commands.c from sources
set(ZIPSH_SOURCES_ABSOLUTE)
foreach(source IN LISTS ZIPSH_SOURCES)
  list(APPEND ZIPSH_SOURCES_ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
endforeach()
# Preserve list elements when forwarding to script mode via -D.
string(REPLACE ";" "\\;" ZIPSH_SOURCES_ABS_ESCAPED "${ZIPSH_SOURCES_ABSOLUTE}")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zipsh_commands.c
  COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    "-DSOURCES=${ZIPSH_SOURCES_ABS_ESCAPED}"
    "-DOUTPUT_FILE=zipsh_commands.c"
    "-P" "${PROJECT_SOURCE_DIR}/cmake/GenerateZipshCommands.cmake"
  DEPENDS
    ${PROJECT_SOURCE_DIR}/cmake/GenerateZipshCommands.cmake
    ${ZIPSH_SOURCES_ABSOLUTE}
  COMMENT "Generating zipsh_commands.c"
)
