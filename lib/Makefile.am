noinst_LIBRARIES=libzip.a
noinst_HEADERS=zip.h \
	zipint.h

libzip_a_SOURCES=zip_open.c zip_close.c zip_err_str.c zip_name_locate.c \
	zip_fopen.c zip_fopen_index.c zip_fclose.c zip_fread.c \
	zip_add.c zip_repalce.c zip_add_zip.c zip_replace_zip.c \
	zf_new_free.c

BUILT_SOURCES=zip_err_str.c

zip_err_str.c: zip.h make_zip_err_str.sh
	make_zip_err_str.sh zip.h zip_err_str.c