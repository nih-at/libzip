man_MANS=${MAN1} ${MAN3}

DISTCLEANFILES=${man_MANS}
EXTRA_DIST=${MANDOC} ${MAN}

MAN1=	zipcmp.1
MAN3=	zip_close.3 \
	zip_delete.3 \
	zip_fclose.3 \
	zip_file_strerror.3 \
	zip_fopen.3 \
	zip_fread.3 \
	zip_get_name.3 \
	zip_name_locate.3 \
	zip_open.3 \
	zip_rename.3 \
	zip_stat.3 \
	zip_strerror.3 \
	zip_unchange.3 \
	zip_unchange_all.3

install-man: link-man3

LN=ln

link-man3: install-man3
	-rm -f ${man3dir}/zip_fopen_index.3
	${LN} ${man3dir}/zip_fopen.3 ${man3dir}/zip_fopen_index.3
	-rm -f ${man3dir}/zip_stat_index.3
	${LN} ${man3dir}/zip_stat.3 ${man3dir}/zip_stat_index.3

MANDOC=${MAN1:.1=.mdoc} ${MAN3:.3=.mdoc}
MAN=${MANDOC:.mdoc=.man}

MDOC2MAN=mdoc2man
SUFFIXES=.man .mdoc

.PHONY: mkman

mkman: ${MAN}

.mdoc.man:
	${MDOC2MAN} $< > $@.$$$$ && mv $@.$$$$ $@

