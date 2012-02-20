ENABLE_TESTING()

SET(STANDALONE_TEST_PROGRAMS
  add_from_buffer
  add_from_file
  add_from_filep
  add_from_zip
  encrypt
  fopen_unchanged
  fread
  get_comment
  name_locate
  set_comment_all
  set_comment_localonly
  set_comment_removeglobal
  set_comment_revert
)

SET(HELPER_TEST_PROGRAMS
  modify
  rename
  set_compression
  stat_index
  tryopen
)

SET(EXTRA_TESTS
  delete_invalid.test
  delete_last.test
  delete_multiple_last.test
  delete_multiple_partial.test
  file_comment_encmismatch.test
  open_cons_extrabytes.test
  open_empty.test
  open_empty_2.test
  open_extrabytes.test
  open_new_but_exists.test
  open_new_ok.test
  open_nonarchive.test
  open_nosuchfile.test
  open_ok.test
  open_truncate.test
  rename_ascii.test
  rename_cp437.test
  rename_deleted.test
  rename_fail.test
  rename_ok.test
  rename_utf8.test
  set_compression_deflate_to_deflate.test
  set_compression_deflate_to_store.test
  set_compression_store_to_deflate.test
  set_compression_store_to_store.test
  set_compression_unknown.test
  stat_index_cp437_guess.test
  stat_index_cp437_raw.test
  stat_index_cp437_strict.test
  stat_index_utf8_guess.test
  stat_index_utf8_raw.test
  stat_index_utf8_strict.test
)

SET(ENV{srcdir} ${CMAKE_CURRENT_SOURCE_DIR})
FOREACH(PROGRAM ${STANDALONE_TEST_PROGRAMS})
  ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.c)
  TARGET_LINK_LIBRARIES(${PROGRAM} zip)
  ADD_TEST(${PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/runtest ${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM})
ENDFOREACH(PROGRAM ${STANDALONE_TEST_PROGRAMS})

FOREACH(PROGRAM ${HELPER_TEST_PROGRAMS})
  ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.c)
  TARGET_LINK_LIBRARIES(${PROGRAM} zip)
ENDFOREACH(PROGRAM ${HELPER_TEST_PROGRAMS})

FOREACH(CASE ${EXTRA_TESTS})
  ADD_TEST(${CASE} ${CMAKE_CURRENT_SOURCE_DIR}/runtest ${CMAKE_CURRENT_SOURCE_DIR}/${CASE})
ENDFOREACH(CASE ${EXTRA_TESTS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../lib
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_BINARY_DIR}/..)
