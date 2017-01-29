# for 'make dist'
SUBDIRS=	gladman-fcrypt

AM_CFLAGS=	@CFLAG_VISIBILITY@
libincludedir = ${libdir}/@PACKAGE@/include

lib_LTLIBRARIES = libzip.la
noinst_HEADERS = zipint.h gladman-fcrypt.h
include_HEADERS = zip.h
nodist_libinclude_HEADERS = zipconf.h

# also update CMakeLists.txt when changing version
libzip_la_LDFLAGS=-no-undefined -version-info 5:0:0
libzip_la_CPPFLAGS=-I$(srcdir)/gladman-fcrypt
libzip_la_LIBADD=@LTLIBOBJS@

if WIN32_HOST
OS_SOURCES=\
	zip_random_win32.c \
	zip_source_win32a.c \
	zip_source_win32handle.c \
	zip_source_win32utf8.c \
	zip_source_win32w.c
else
OS_SOURCES=\
	zip_random_unix.c \
	zip_source_file.c
endif

libzip_la_SOURCES=\
	${OS_SOURCES} \
	gladman-fcrypt.c \
	zip_add.c \
	zip_add_dir.c \
	zip_add_entry.c \
	zip_buffer.c \
	zip_close.c \
	zip_delete.c \
	zip_dir_add.c \
	zip_dirent.c \
	zip_discard.c \
	zip_entry.c \
	zip_err_str.c \
	zip_error.c \
	zip_error_clear.c \
	zip_error_get.c \
	zip_error_get_sys_type.c \
	zip_error_strerror.c \
	zip_error_to_str.c \
	zip_extra_field.c \
	zip_extra_field_api.c \
	zip_fclose.c \
	zip_fdopen.c \
	zip_file_add.c \
	zip_file_error_clear.c \
	zip_file_error_get.c \
	zip_file_get_comment.c \
	zip_file_get_external_attributes.c \
	zip_file_get_offset.c \
	zip_file_rename.c \
	zip_file_replace.c \
	zip_file_set_comment.c \
	zip_file_set_encryption.c \
	zip_file_set_external_attributes.c \
	zip_file_set_mtime.c \
	zip_file_strerror.c \
	zip_filerange_crc.c \
	zip_fopen.c \
	zip_fopen_encrypted.c \
	zip_fopen_index.c \
	zip_fopen_index_encrypted.c \
	zip_fread.c \
	zip_fseek.c \
	zip_ftell.c \
	zip_get_archive_comment.c \
	zip_get_archive_flag.c \
	zip_get_compression_implementation.c \
	zip_get_encryption_implementation.c \
	zip_get_file_comment.c \
	zip_get_num_entries.c \
	zip_get_num_files.c \
	zip_get_name.c \
	zip_hash.c \
	zip_io_util.c \
	zip_memdup.c \
	zip_name_locate.c \
	zip_new.c \
	zip_open.c \
	zip_rename.c \
	zip_replace.c \
	zip_set_archive_comment.c \
	zip_set_archive_flag.c \
	zip_set_default_password.c \
	zip_set_file_comment.c \
	zip_set_file_compression.c \
	zip_set_name.c \
	zip_source_begin_write.c \
	zip_source_buffer.c \
	zip_source_call.c \
	zip_source_close.c \
	zip_source_commit_write.c \
	zip_source_crc.c \
	zip_source_deflate.c \
	zip_source_error.c \
	zip_source_filep.c \
	zip_source_free.c \
	zip_source_function.c \
	zip_source_is_deleted.c \
	zip_source_layered.c \
	zip_source_open.c \
	zip_source_pkware.c \
	zip_source_read.c \
	zip_source_remove.c \
	zip_source_rollback_write.c \
	zip_source_seek.c \
	zip_source_seek_write.c \
	zip_source_stat.c \
	zip_source_supports.c \
	zip_source_tell.c \
	zip_source_tell_write.c \
	zip_source_window.c \
	zip_source_winzip_aes_decode.c \
	zip_source_winzip_aes_encode.c \
	zip_source_write.c \
	zip_source_zip.c \
	zip_source_zip_new.c \
	zip_stat.c \
	zip_stat_index.c \
	zip_stat_init.c \
	zip_strerror.c \
	zip_string.c \
	zip_unchange.c \
	zip_unchange_all.c \
	zip_unchange_archive.c \
	zip_unchange_data.c \
	zip_utf-8.c

BUILT_SOURCES=zipconf.h
CLEANFILES=	${BUILT_SOURCES}

EXTRA_DIST=	CMakeLists.txt \
		compat.h \
		make_zip_err_str.sh \
		make_zipconf.sh \
		zip_source_win32a.c \
		zip_source_win32handle.c \
		zip_source_win32utf8.c \
		zip_source_win32w.c \
		zipwin32.h

zip_err_str.c: zip.h make_zip_err_str.sh
	sh $(srcdir)/make_zip_err_str.sh $(srcdir)/zip.h zip_err_str.c

zipconf.h: ${top_builddir}/config.h make_zipconf.sh
	sh ${srcdir}/make_zipconf.sh ${top_builddir}/config.h zipconf.h
