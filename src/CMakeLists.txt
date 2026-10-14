file(GLOB ZIPSH_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/zipsh/*.c)

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


add_executable(zipsh zipsh.c zipsh_commands.c ${ZIPSH_SOURCES})
target_link_libraries(zipsh zip)
target_include_directories(zipsh PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib zipsh ${PROJECT_BINARY_DIR})
if(LIBZIP_DO_INSTALL)
  install(TARGETS zipsh EXPORT ${PROJECT_NAME}-bin-targets DESTINATION bin)
endif()
if(NOT HAVE_GETOPT)
  target_sources(zipsh PRIVATE getopt.c)
endif(NOT HAVE_GETOPT)
if (ENABLE_COVERAGE)
  target_compile_options(zipsh PRIVATE -coverage)
  target_link_options(zipsh PRIVATE -coverage)
endif()

# create zipsh_commands.c from sources
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zipsh_commands.c
  COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "-DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    "-P" "${PROJECT_SOURCE_DIR}/cmake/GenerateZipshCommands.cmake"
  DEPENDS
    ${PROJECT_SOURCE_DIR}/cmake/GenerateZipshCommands.cmake
    ${ZIPSH_SOURCES}
  COMMENT "Generating zipsh_commands.c"
)
