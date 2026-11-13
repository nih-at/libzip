include(GenerateCommandTable)

set(SHELL_SOURCES
  shell/argument.c
  shell/command.c
  shell/complete.c
  shell/error_message.c
  shell/invocation.c
  shell/help.c
  shell/shell.c
  shell/words.c
)

set(ZIPSH_SOURCES
  zipsh/add.c
  zipsh/add_directory.c
  zipsh/archive_comment.c
  zipsh/argument_types.c
  zipsh/cat.c
  zipsh/delete.c
  zipsh/extra_field.c
  zipsh/extra_field_add.c
  zipsh/extra_field_count.c
  zipsh/extra_field_delete.c
  zipsh/extra_field_get.c
  zipsh/extra_field_replace.c
  zipsh/file_comment.c
  zipsh/file_compression.c
  zipsh/file_crc.c
  zipsh/file_encryption.c
  zipsh/file_reference.c
  zipsh/global.c
  zipsh/list.c
  zipsh/rename.c
  zipsh/set_default_password.c
  zipsh/standard_flags.c
  zipsh/stat.c
  zipsh/util.c
  CACHE INTERNAL "ZIPSH_SOURCES"
)

set(COMPAT_SOURCES)
if (NOT HAVE_ASPRINTF)
  list(APPEND COMPAT_SOURCES compat/asprintf.c)
endif()
if (NOT HAVE_GETLINE)
  list(APPEND COMPAT_SOURCES compat/getline.c)
endif()
if (NOT HAVE_FNMATCH)
  list(APPEND COMPAT_SOURCES compat/fnmatch.c)
endif()
if (NOT HAVE_GETOPT)
  list(APPEND COMPAT_SOURCES compat/getopt.c)
endif()
if (NOT HAVE_VASPRINTF)
  list(APPEND COMPAT_SOURCES compat/vasprintf.c)
endif()

if (COMPAT_SOURCES)
  add_library(compat STATIC ${COMPAT_SOURCES})
  target_include_directories(compat PRIVATE ${PROJECT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
endif()

add_library(shell STATIC ${SHELL_SOURCES})
target_include_directories(shell PRIVATE shell ${PROJECT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
if (USE_LINE_EDITING)
if (HAVE_EDITLINE)
  target_link_libraries(shell editline::editline)
  target_sources(shell PRIVATE shell/editline.c)
elseif (HAVE_READLINE)
  target_link_libraries(shell readline::readline)
  target_sources(shell PRIVATE shell/readline.c)
elseif (HAVE_GETCONSOLEMODE)
  target_sources(shell PRIVATE shell/windows_line_editing.c)
else()
  message(FATAL_ERROR "-- internal error: USE_LINE_EDITING is set, but no line editing implementation is available")
endif()
else()
  target_sources(shell PRIVATE shell/no_line_editing.c)
endif()
if (ENABLE_COVERAGE)
  target_compile_options(shell PRIVATE -coverage)
  target_link_options(shell PRIVATE -coverage)
endif()
if (TARGET compat)
  target_link_libraries(shell compat)
endif()

foreach(PROGRAM zipcmp zipmerge ziptool)
  add_executable(${PROGRAM} ${PROGRAM}.c)
  target_link_libraries(${PROGRAM} zip)
  if (TARGET compat)
    target_link_libraries(${PROGRAM} compat)
  endif()
  target_include_directories(${PROGRAM} PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})
  if(LIBZIP_DO_INSTALL)
    install(TARGETS ${PROGRAM} EXPORT ${PROJECT_NAME}-bin-targets DESTINATION bin)
  endif()
  if (ENABLE_COVERAGE)
    target_compile_options(${PROGRAM} PRIVATE -coverage)
    target_link_options(${PROGRAM} PRIVATE -coverage)
  endif()
endforeach()
target_sources(zipcmp PRIVATE diff_output.c)
target_link_libraries(zipcmp ${FTS_LIB} ZLIB::ZLIB)

add_library(zipsh_common STATIC ${ZIPSH_SOURCES})
target_link_libraries(zipsh_common shell zip)
target_include_directories(zipsh_common PUBLIC ${PROJECT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR} shell zipsh ${PROJECT_BINARY_DIR})

add_executable(zipsh zipsh/zipsh.c zipsh_commands.c)
target_link_libraries(zipsh zipsh_common shell zip)
if(LIBZIP_DO_INSTALL)
  install(TARGETS zipsh EXPORT ${PROJECT_NAME}-bin-targets DESTINATION bin)
endif()
if (ENABLE_COVERAGE)
  target_compile_options(zipsh PRIVATE -coverage)
  target_link_options(zipsh PRIVATE -coverage)
endif()

# Generate zipsh_commands.c from command definitions in source files
generate_command_table(
  OUTPUT_FILE zipsh_commands.c
  SOURCES ${ZIPSH_SOURCES}
)
