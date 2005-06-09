man_MANS=${MAN1} ${MAN3}

DISTCLEANFILES=${man_MANS} ${HTML}
EXTRA_DIST=${MANDOC} ${MAN}

MAN1=	zipcmp.1 zipmerge.1
MAN3=	libzip.3 \
	zip_add.3 \
	zip_close.3 \
	zip_delete.3 \
	zip_error_get.3 \
	zip_error_get_sys_type.3 \
	zip_error_to_str.3 \
	zip_errors.3 \
	zip_fclose.3 \
	zip_file_strerror.3 \
	zip_fopen.3 \
	zip_fread.3 \
	zip_get_name.3 \
	zip_get_num_files.3 \
	zip_name_locate.3 \
	zip_open.3 \
	zip_rename.3 \
	zip_source_buffer.3 \
	zip_source_file.3 \
	zip_source_filep.3 \
	zip_source_free.3 \
	zip_source_function.3 \
	zip_source_zip.3 \
	zip_stat.3 \
	zip_unchange.3 \
	zip_unchange_all.3

install-data-hook: link-man3

LN=ln

link-man3: install-man3 uninstall-hook
	${LN} ${DESTDIR}${man3dir}/zip_error_get.3 \
		${DESTDIR}${man3dir}/zip_file_error_get.3
	${LN} ${DESTDIR}${man3dir}/zip_fopen.3 \
		${DESTDIR}${man3dir}/zip_fopen_index.3
	${LN} ${DESTDIR}${man3dir}/zip_add.3 \
		${DESTDIR}${man3dir}/zip_replace.3
	${LN} ${DESTDIR}${man3dir}/zip_stat.3 \
		${DESTDIR}${man3dir}/zip_stat_index.3
	${LN} ${DESTDIR}${man3dir}/zip_file_strerror.3 \
		${DESTDIR}${man3dir}/zip_strerror.3

uninstall-hook:
	-rm -f ${DESTDIR}${man3dir}/zip_file_error_get.3
	-rm -f ${DESTDIR}${man3dir}/zip_fopen_index.3
	-rm -f ${DESTDIR}${man3dir}/zip_replace.3
	-rm -f ${DESTDIR}${man3dir}/zip_stat_index.3
	-rm -f ${DESTDIR}${man3dir}/zip_strerror.3

link-html: ${HTML}
	-rm -f zip_file_error_get.html zip_fopen_index.html \
		zip_replace.html zip_stat_index.html \
		zip_strerror.html
	${LN} zip_error_get.html zip_file_error_get.html
	${LN} zip_fopen.html zip_fopen_index.html
	${LN} zip_add.html zip_replace.html
	${LN} zip_stat.html zip_stat_index.html
	${LN} zip_file_strerror.html zip_strerror.html

HTML=${MANDOC:.mdoc=.html}
MANDOC=${MAN1:.1=.mdoc} ${MAN3:.3=.mdoc}
MAN=${MANDOC:.mdoc=.man}

MDOC2MAN=mdoc2man
SUFFIXES=.man .mdoc .html

.PHONY: mkman update-errors

mkman: ${MAN}
mkhtml: ${HTML} link-html

update-errors:
	sh $(srcdir)/make_zip_errors.sh $(srcdir)/../lib/zip.h zip_errors.mdoc

.mdoc.man:
	${MDOC2MAN} $< > $@.$$$$ && mv $@.$$$$ $@

.mdoc.html:
	nroff -mdoc2html $< | sed -e "s,../html3/,," > $@.$$$$ && mv $@.$$$$ $@
