.IGNORE:

EXTRA_DIST = sakura.input sakura.flood sakura.bathy sakura.data doc
pkgdata_DATA = sakura.input sakura.flood sakura.bathy sakura.data doc

doc: force_rebuild
	mkdir ./doc
	/home/plum/huttone/local/doc++/bin/doc++ --html --dir ./doc $(srcdir)/../src/sakura_main.c $(srcdir)/../src/sakura.h

force_rebuild:

