man_MANS=${MAN1} ${MAN3}

DISTCLEANFILES=${man_MANS}

MAN1=	zipcmp.1
MAN3=	zip_open.3

MANDOC=${MAN1:.1=.mdoc} ${MAN3:.3=.mdoc}
MAN=${MANDOC:.mdoc=.man}

EXTRA_DIST=${MANDOC} ${MAN}
