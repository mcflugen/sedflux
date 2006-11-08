.IGNORE:

DOCXX_DIR = /home/plum/huttone/local/doc++

EXTRA_DIST = libsed.dxx
pkgdata_DATA = html

html: force_rebuild
	${DOCXX_DIR}/bin/docify ${top_srcdir}/libsed.dxx ${top_srcdir}/libsed.doc
	${DOCXX_DIR}/bin/doc++ -d html ${top_srcdir}/libsed.doc
	rm -f ${top_srcdir}/libsed.doc

install: html
	cp html/* ${pkgdatadir}

force_rebuild:



