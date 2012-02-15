noinst_PROGRAMS= \
	add_dir \
	add_from_buffer \
	add_from_file \
	add_from_filep \
	add_from_zip \
	encrypt \
	fopen_unchanged \
	fread \
	get_comment \
	get_num_entries \
	name_locate \
	rename \
	set_comment_all \
	set_comment_localonly \
	set_comment_removeglobal \
	set_comment_revert \
	set_compression \
	tryopen

EXTRA_DIST= \
	CMakeLists.txt \
	runtest \
	${TESTS} \
	broken.zip \
	encrypt.zip \
	manyfiles.zip \
	rename_ok.zip \
	test.zip \
	testbuffer.zip \
	testdir.zip \
	testchanged.zip \
	testchangedlocal.zip \
	testcomment.zip \
	testcommentremoved.zip \
	testdeflated.zip \
	testempty.zip \
	testextrabytes.zip \
	testfile.txt \
	testfile.zip \
	testnottorrent.zip \
	teststored.zip \
	testtorrent.tzip

TESTS_ENVIRONMENT=	ZIPCMP=${top_builddir}/src/zipcmp ${srcdir}/runtest

TESTS= \
	add_dir.test \
	add_from_buffer.test \
	add_from_file.test \
	add_from_file_duplicate.test \
	add_from_file_twice_duplicate.test \
	add_from_filep.test \
	add_from_zip.test \
	encrypt.test \
	fopen_unchanged.test \
	fread.test \
	get_comment.test \
	get_num_entries_0.test \
	get_num_entries_0b.test \
	get_num_entries_4.test \
	get_num_entries_70000.test \
	name_locate.test \
	open_cons_extrabytes.test \
	open_empty.test \
	open_extrabytes.test \
	open_many_ok.test \
	open_new_but_exists.test \
	open_new_ok.test \
	open_nonarchive.test \
	open_nosuchfile.test \
	open_ok.test \
	rename_fail.test \
	rename_ok.test \
	set_comment_all.test \
	set_comment_localonly.test \
	set_comment_removeglobal.test \
	set_comment_revert.test \
	set_compression_deflate_to_deflate.test \
	set_compression_deflate_to_store.test \
	set_compression_store_to_deflate.test \
	set_compression_store_to_store.test \
	set_compression_unknown.test \
	torrent-already.test \
	torrent-new.test

DISABLED_TESTS= \
	open_truncate.test \
	encoding-ascii.test \
	encoding-cp437.test \
	encoding-cp437-all.test \
	encoding-utf-8.test

AM_CPPFLAGS=-I${top_srcdir}/lib -I../lib -I${top_srcdir}/src
LDADD=${top_builddir}/lib/libzip.la
