man_MANS=${MAN1} ${MAN3}

DISTCLEANFILES=${man_MANS}
EXTRA_DIST=${MANDOC} ${MAN}

MAN1=	zipcmp.1
MAN3=	zip_close.3 \
	zip_delete.3 \
	zip_fopen.3 \
	zip_open.3 \
	zip_unchange.3 \
	zip_unchange_all.3

install-man: link-man3

LN=ln

link-man3: install-man3
	${LN} ${man3dir}/zip_fopen.3 ${man3dir}/zip_fopen_index.3

MANDOC=${MAN1:.1=.mdoc} ${MAN3:.3=.mdoc}
MAN=${MANDOC:.mdoc=.man}

MDOC2MAN=mdoc2man
SUFFIXES=.man .mdoc

.PHONY: mkman

mkman: ${MAN}

.mdoc.man:
	${MDOC2MAN} $< > $@.$$$$ && mv $@.$$$$ $@

