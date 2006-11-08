.IGNORE:

DOCXX_DIR = /home/plum/huttone/local/doc++

EXTRA_DIST = sakura.dxx
pkgdata_DATA = html

html: force_rebuild
	${DOCXX_DIR}/bin/docify ${top_srcdir}/sakura.dxx ${top_srcdir}/sakura.doc
	${DOCXX_DIR}/bin/doc++ -d html ${top_srcdir}/sakura.doc
	rm -f ${top_srcdir}/sakura.doc

install: html
	cp html/* ${pkgdatadir}

force_rebuild:



