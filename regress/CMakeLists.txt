
include(GenerateCommandTable)

if (CMOCKA_FOUND)
  set(CMOCKA_TEST_PROGRAMS
    crypto_clear
    shell_completion
  )

  foreach(PROGRAM IN LISTS CMOCKA_TEST_PROGRAMS)
    add_executable(${PROGRAM} programs/${PROGRAM}.c programs/cmocka_utilities.c)
    target_link_libraries(${PROGRAM} zip ${CMOCKA_LIBRARIES})
    target_include_directories(${PROGRAM} PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR} ${CMOCKA_INCLUDE_DIRS}) 

    add_test(NAME ${PROGRAM} COMMAND ${PROGRAM})
  endforeach()

  # The internal symbol is hidden when zip is a shared library, so link against the file directly.
  target_sources(crypto_clear PRIVATE ${PROJECT_SOURCE_DIR}/lib/zip_crypto.c)

  target_link_libraries(shell_completion shell)
endif()

if (NIHTEST)
  set(TEST_PROGRAMS
    add_from_filep
    can_clone_file
    exact_read_integrity
    filep_error
    fopen_unchanged
    fread
    fseek
    source_seek
    tryopen
  )

  set(HOLE_USERS
    hole
    ziptool_regress
    ziptool_regress_nonrandom
  )

  set(ZIPSH_REGRESS_SOURCES
    programs/zipsh/add_nul.c
    programs/zipsh/count.c
    programs/zipsh/echo.c
    programs/zipsh/source_nul.c
    programs/zipsh/zipsh_regress.c
  )

  add_library(zipsh_regress_common STATIC ${ZIPSH_REGRESS_SOURCES})
  target_link_libraries(zipsh_regress_common zipsh_common)
  target_include_directories(zipsh_regress_common PUBLIC programs/zipsh_regress ${PROJECT_BINARY_DIR})

  foreach(PROGRAM in zipsh_regress zipsh_regress_nonrandom)
    add_executable(${PROGRAM} zipsh_regress_commands.c)
    target_link_libraries(${PROGRAM} zipsh_regress_common)
    if (ENABLE_COVERAGE)
      target_compile_options(${PROGRAM} PRIVATE -coverage)
      target_link_options(${PROGRAM} PRIVATE -coverage)
    endif()
  endforeach()
  target_link_libraries(zipsh_regress zip)
  target_link_libraries(zipsh_regress_nonrandom zip_nonrandom)

  set(ZIP_PROGRAMS ${TEST_PROGRAMS} ${HOLE_USERS})

  foreach(PROGRAM IN LISTS ZIP_PROGRAMS)
    if (PROGRAM STREQUAL "ziptool_regress_nonrandom")
      add_executable(${PROGRAM} programs/ziptool_regress.c)
      target_link_libraries(${PROGRAM} zip_nonrandom)
    else()
      add_executable(${PROGRAM} programs/${PROGRAM}.c)
      target_link_libraries(${PROGRAM} zip)
      
      if (ENABLE_COVERAGE)
        target_compile_options(${PROGRAM} PRIVATE -coverage)
        target_link_options(${PROGRAM} PRIVATE -coverage)
      endif()
    endif()
    target_include_directories(${PROGRAM} PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})
    if (TARGET compat)
      target_link_libraries(${PROGRAM} compat)
    endif()
  endforeach()

  foreach(PROGRAM IN LISTS HOLE_USERS)
    target_sources(${PROGRAM} PRIVATE programs/source_hole.c)
  endforeach()

  foreach(PROGRAM IN LISTS FUZZ_PROGRAMS)
    target_sources(${PROGRAM} PRIVATE programs/fuzz_main.c)
  endforeach()

  add_executable(files_differ programs/files_differ.c)

  # for including ziptool.c
  target_include_directories(ziptool_regress PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/src)
  target_include_directories(ziptool_regress_nonrandom PRIVATE BEFORE ${PROJECT_SOURCE_DIR}/src)


  add_custom_target(cleanup
      COMMAND ${CMAKE_COMMAND} -DDIR=${PROJECT_BINARY_DIR}/regress -P ${PROJECT_SOURCE_DIR}/regress/support/cleanup.cmake
      )

  add_custom_target(testinput
    ALL
    VERBATIM
    COMMAND ${CMAKE_COMMAND} -E tar x ${PROJECT_SOURCE_DIR}/regress/data/manyfiles-zip.zip
    COMMAND ${CMAKE_COMMAND} -E tar x ${PROJECT_SOURCE_DIR}/regress/data/bigzero-zip.zip
    DEPENDS ${PROJECT_SOURCE_DIR}/regress/data/manyfiles-zip.zip ${PROJECT_SOURCE_DIR}/regress/data/bigzero-zip.zip
  )

  set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    bigzero.zip
    manyfiles-133000.zip
    manyfiles-65536.zip
    manyfiles-fewer.zip
    manyfiles-more.zip
    manyfiles-zip64-modulo.zip
    manyfiles-zip64.zip
    manyfiles.zip
  )

  set(path "$<TARGET_FILE_DIR:zip>;$ENV{PATH}")
  if (TARGET zlib)
    set(path "$<TARGET_FILE_DIR:zlib>;${path}")
  endif()
  if (NOT IS_WINDOWS)
  string(REPLACE ";" "\\;" path "${path}")
  endif()

  # set(ENV{srcdir} ${PROJECT_SOURCE_DIR}/regress)

  file(GLOB EXTRA_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/*.test)
  foreach(FULL_CASE IN LISTS EXTRA_TESTS)
    get_filename_component(CASE ${FULL_CASE} NAME)
    add_test(NAME ${CASE} COMMAND ${NIHTEST} -v ${CASE})
    # TODO: add --bin-sub-directory $<CONFIG>
    set_tests_properties(${CASE} PROPERTIES SKIP_RETURN_CODE 77)
    set_tests_properties(${CASE} PROPERTIES ENVIRONMENT "PATH=${path}")
  endforeach()
endif(NIHTEST)

set(XFAIL_TESTS
#  zipcmp_zip_dir_slash.test
)

foreach(CASE ${XFAIL_TESTS})
  set_tests_properties(${CASE} PROPERTIES WILL_FAIL TRUE)
endforeach()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

configure_file(support/nihtest.conf.in nihtest.conf @ONLY)

if(ENABLE_COVERAGE)
  add_custom_target(coverage
          # gather data
          COMMAND ${LCOV} --directory . --capture --output-file coverage.info
          # generate report
          COMMAND ${GENHTML} --demangle-cpp -o coverage coverage.info WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          COMMAND ${CMAKE_COMMAND} -E cmake_echo_color "Open coverage/index.html in browser to view results."
  )
endif()


# Generate zipsh_regress_commands.c from command definitions in source files
set(ZIPSH_REGRESS_COMMAND_SOURCES ${ZIPSH_REGRESS_SOURCES})
foreach(source IN LISTS ZIPSH_SOURCES)
  list(APPEND ZIPSH_REGRESS_COMMAND_SOURCES "../src/${source}")
endforeach()

generate_command_table(
  OUTPUT_FILE zipsh_regress_commands.c
  SOURCES ${ZIPSH_REGRESS_COMMAND_SOURCES}
)
