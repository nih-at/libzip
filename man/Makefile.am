man_MANS=${MAN1} ${MAN3}

DISTCLEANFILES=${man_MANS}
EXTRA_DIST=${MANDOC} ${MAN}

MAN1=	zipcmp.1
MAN3=	zip_open.3

MANDOC=${MAN1:.1=.mdoc} ${MAN3:.3=.mdoc}
MAN=${MANDOC:.mdoc=.man}

MDOC2MAN=mdoc2man
SUFFIXES=.man .mdoc

.PHONY: mkman

mkman: ${MAN}

.mdoc.man:
	${MDOC2MAN} $< > $@.$$$$ && mv $@.$$$$ $@

