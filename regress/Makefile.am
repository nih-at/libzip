noinst_PROGRAMS= \
	add_from_buffer \
	add_from_filep \
	add_from_zip \
	fopen_unchanged \
	fread \
	modify \
	name_locate \
	rename \
	set_comment_all \
	set_comment_localonly \
	set_comment_removeglobal \
	set_comment_revert \
	set_compression \
	stat_index \
	tryopen

EXTRA_DIST= \
	CMakeLists.txt \
	runtest \
	${TESTS} \
	bigzero-zip.zip \
	broken.zip \
	encrypt.zip \
	manyfiles-zip.zip \
	rename_ok.zip \
	streamed.zip \
	streamed-zip64.zip \
	test.zip \
	test2.zip \
	test-cp437.zip \
	test-cp437-fc.zip \
	test-utf8.zip \
	test-utf8-unmarked.zip \
	testbuffer.zip \
	testdir.zip \
	testchanged.zip \
	testchangedlocal.zip \
	testcomment.zip \
	testcomment13.zip \
	testcommentremoved.zip \
	testdeflated.zip \
	testempty.zip \
	testextrabytes.zip \
	testfile.txt \
	testfile.zip \
	testfile-UTF8.zip \
	testnottorrent.zip \
	teststored.zip \
	testtorrent.tzip \
	zip64.tzip

TESTS_ENVIRONMENT=	ZIPCMP=${top_builddir}/src/zipcmp ${srcdir}/runtest

TESTS= \
	add_dir.test \
	add_from_buffer.test \
	add_from_file.test \
	add_from_file_duplicate.test \
	add_from_file_twice_duplicate.test \
	add_from_filep.test \
	add_from_zip_deflated.test \
	add_from_zip_stored.test \
	delete_add_same.test \
	delete_invalid.test \
	delete_last.test \
	delete_multiple_last.test \
	delete_multiple_partial.test \
	delete_renamed_rename.test \
	encrypt.test \
	file_comment_encmismatch.test \
	fopen_unchanged.test \
	fread.test \
	get_comment.test \
	name_locate.test \
	open_cons_extrabytes.test \
	open_empty.test \
	open_empty_2.test \
	open_extrabytes.test \
	open_many_ok.test \
	open_new_but_exists.test \
	open_new_ok.test \
	open_nonarchive.test \
	open_nosuchfile.test \
	open_ok.test \
	open_truncate.test \
	open_zip64_ok.test \
	rename_ascii.test \
	rename_cp437.test \
	rename_deleted.test \
	rename_fail.test \
	rename_ok.test \
	rename_utf8.test \
	rename_utf8_encmismatch.test \
	set_comment_all.test \
	set_comment_localonly.test \
	set_comment_removeglobal.test \
	set_comment_revert.test \
	set_compression_deflate_to_deflate.test \
	set_compression_deflate_to_store.test \
	set_compression_store_to_deflate.test \
	set_compression_store_to_store.test \
	set_compression_unknown.test \
	stat_index_cp437_guess.test \
	stat_index_cp437_raw.test \
	stat_index_cp437_strict.test \
	stat_index_streamed.test \
	stat_index_streamed_zip64.test \
	stat_index_utf8_guess.test \
	stat_index_utf8_raw.test \
	stat_index_utf8_strict.test \
	stat_index_utf8_unmarked_strict.test \
	stat_index_zip64.test \
	torrent-already.test

DISABLED_TESTS= \
	encoding-ascii.test \
	encoding-cp437.test \
	encoding-cp437-all.test \
	encoding-utf-8.test \
	open_incons.test \
	torrent-new.test

AM_CPPFLAGS=-I${top_srcdir}/lib -I../lib -I${top_srcdir}/src
LDADD=${top_builddir}/lib/libzip.la

check-am: bigzero.zip manyfiles.zip

bigzero.zip: bigzero-zip.zip
	${UNZIP} ${srcdir}/bigzero-zip.zip
	${TOUCH} ${builddir}/bigzero.zip

manyfiles.zip: manyfiles-zip.zip
	${UNZIP} ${srcdir}/manyfiles-zip.zip
	${TOUCH} ${builddir}/manyfiles.zip

CLEANFILES=bigzero.zip manyfiles.zip
