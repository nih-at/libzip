noinst_LIBRARIES=libzip.a
noinst_HEADERS=zip.h \
	zipint.h

SUBDIRS=. tests

libzip_a_SOURCES=zip_open.c zip_close.c zip_err_str.c zip_name_locate.c \
	zip_fopen.c zip_fopen_index.c zip_fclose.c zip_fread.c \
	zip_add.c zip_replace.c zip_add_zip.c zip_replace_zip.c \
	zip_add_data.c zip_replace_data.c zip_add_file.c zip_replace_file.c \
	zip_add_filep.c zip_replace_filep.c \
	zip_delete.c zip_unchange.c zip_rename.c \
	zip_new.c zip_free.c zip_new_entry.c zip_free_entry.c \
	zip_new_meta.c zip_free_meta.c zip_merge_meta.c zip_unchange_data.c \
	zip_set_name.c

BUILT_SOURCES=zip_err_str.c

EXTRA_DIST=make_zip_err_str.sh

zip_err_str.c: zip.h make_zip_err_str.sh
	$(srcdir)/make_zip_err_str.sh $(srcdir)/zip.h zip_err_str.c
