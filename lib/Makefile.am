lib_LTLIBRARIES = libzip.la
noinst_HEADERS = zipint.h
include_HEADERS = zip.h

libzip_la_LDFLAGS=-version-info 0:5

libzip_la_SOURCES=\
	zip_add.c \
	zip_add_data.c \
	zip_add_file.c \
	zip_add_filep.c \
	zip_add_zip.c \
	zip_close.c \
	zip_delete.c \
	zip_dirent.c \
	zip_err_str.c \
	zip_error.c \
	zip_error_str.c \
	zip_error_strerror.c \
	zip_fclose.c \
	zip_file_get_error.c \
	zip_file_strerror.c \
	zip_fopen.c \
	zip_fopen_index.c \
	zip_fread.c \
	zip_free.c \
	zip_free_entry.c \
	zip_free_meta.c \
	zip_get_error.c \
	zip_get_num_files.c \
	zip_get_name.c \
	zip_merge_meta.c \
	zip_name_locate.c \
	zip_new.c \
	zip_new_entry.c \
	zip_new_meta.c \
	zip_open.c \
	zip_rename.c \
	zip_replace.c \
	zip_replace_data.c \
	zip_replace_file.c \
	zip_replace_filep.c \
	zip_replace_zip.c \
	zip_set_name.c \
	zip_stat.c \
	zip_stat_index.c \
	zip_strerror.c \
	zip_unchange.c \
	zip_unchange_all.c \
	zip_unchange_data.c

BUILT_SOURCES=zip_err_str.c

EXTRA_DIST=make_zip_err_str.sh

zip_err_str.c: zip.h make_zip_err_str.sh
	$(srcdir)/make_zip_err_str.sh $(srcdir)/zip.h zip_err_str.c
