.IGNORE:

DOCXX_DIR = /home/plum/huttone/local/doc++

EXTRA_DIST = squall.dxx
pkgdata_DATA = html

html: force_rebuild
	${DOCXX_DIR}/bin/docify ${top_srcdir}/squall.dxx ${top_srcdir}/squall.doc
	${DOCXX_DIR}/bin/doc++ -d html ${top_srcdir}/squall.doc
	rm -f ${top_srcdir}/squall.doc

install: html
	cp html/* ${pkgdatadir}

force_rebuild:



