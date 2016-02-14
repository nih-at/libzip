ENABLE_TESTING()

CHECK_FUNCTION_EXISTS(getopt HAVE_GETOPT)
IF(NOT HAVE_GETOPT)
  SET(SRC_EXTRA_FILES ../src/getopt.c)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../src)
ENDIF(NOT HAVE_GETOPT)

SET(STANDALONE_TEST_PROGRAMS
  add_from_filep
  fopen_unchanged
#  fread
)

SET(HELPER_TEST_PROGRAMS
#  tryopen
)

SET(GETOPT_USERS
  fread
  tryopen
)

SET(EXTRA_TESTS
	add_dir.test
	add_from_buffer.test
	add_from_file.test
	add_from_file_duplicate.test
	add_from_file_twice_duplicate.test
	add_from_filep.test
	add_from_stdin.test
	add_from_zip_closed.test
	add_from_zip_deflated.test
	add_from_zip_partial_deflated.test
	add_from_zip_partial_stored.test
	add_from_zip_stored.test
	add_stored.test
	add_stored_in_memory.test
	count_entries.test
	decrypt-correct-password.test
	decrypt-wrong-password.test
	delete_add_same.test
	delete_invalid.test
	delete_last.test
	delete_multiple_last.test
	delete_multiple_partial.test
	delete_renamed_rename.test
	encrypt.test
	extra_add.test
	extra_add_multiple.test
	extra_count.test
	extra_count_by_id.test
	extra_count_ignore_zip64.test
	extra_delete.test
	extra_delete_by_id.test
	extra_field_align.test
	extra_get.test
	extra_get_by_id.test
	extra_set.test
	extra_set_modify_c.test
	extra_set_modify_l.test
	file_comment_encmismatch.test
	fopen_unchanged.test
	fread.test
	get_comment.test
	name_locate.test
	open_cons_extrabytes.test
	open_empty.test
	open_empty_2.test
	open_extrabytes.test
	open_filename_empty.test
	open_incons.test
	open_many_ok.test
	open_multidisk.test
	open_new_but_exists.test
	open_new_ok.test
	open_nonarchive.test
	open_nosuchfile.test
	open_ok.test
	open_too_short.test
	open_truncate.test
	open_zip64_3mf.test
	open_zip64_ok.test
	rename_ascii.test
	rename_cp437.test
	rename_deleted.test
	rename_fail.test
	rename_ok.test
	rename_utf8.test
	rename_utf8_encmismatch.test
	set_comment_all.test
	set_comment_localonly.test
	set_comment_removeglobal.test
	set_comment_revert.test
	set_compression_deflate_to_deflate.test
	set_compression_deflate_to_store.test
	set_compression_store_to_deflate.test
	set_compression_store_to_store.test
	set_compression_unknown.test
	stat_index_cp437_guess.test
	stat_index_cp437_raw.test
	stat_index_cp437_strict.test
	stat_index_fileorder.test
	stat_index_streamed.test
	stat_index_streamed_zip64.test
	stat_index_utf8_guess.test
	stat_index_utf8_raw.test
	stat_index_utf8_strict.test
	stat_index_utf8_unmarked_strict.test
	stat_index_zip64.test
	utf-8-standardization.test
	zip64_creation.test
	zip64_stored_creation.test
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

FOREACH(PROGRAM ${GETOPT_USERS})
  ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.c ${SRC_EXTRA_FILES})
  TARGET_LINK_LIBRARIES(${PROGRAM} zip)
ENDFOREACH(PROGRAM ${GETOPT_USERS})

ADD_TEST(fread ${CMAKE_CURRENT_SOURCE_DIR}/runtest ${CMAKE_CURRENT_SOURCE_DIR}/fread)

FOREACH(CASE ${EXTRA_TESTS})
  ADD_TEST(${CASE} ${CMAKE_CURRENT_SOURCE_DIR}/runtest ${CMAKE_CURRENT_SOURCE_DIR}/${CASE})
ENDFOREACH(CASE ${EXTRA_TESTS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../lib
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_BINARY_DIR}/..)
