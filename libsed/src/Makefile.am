## Comment line

#lib_LIBRARIES = libsedflux.a
lib_LTLIBRARIES = libsedflux.la
libsedflux_la_SOURCES = datadir.h sed_cell.c sed_column.c sed_cube.c sed_epoch.c sed_hydro.c sed_process.c sed_property.c sed_property_file.c sed_sediment.c sed_tripod.c sed_wave.c
#libsedflux_a_SOURCES = sed_cube.c sed_cell.c sed_column.c sed_hydro.c sed_sediment.c sed_print.c sed_epoch.c sed_process.c

include_HEADERS = datadir.h sed_cell.h sed_column.h sed_const.h sed_cube.h sed_epoch.h sed_hydro.h sed_process.h sed_property.h sed_property_file.h sed_sedflux.h sed_sediment.h sed_tripod.h sed_wave.h

bin_PROGRAMS = libsed_unit_test
libsed_unit_test_SOURCES = datadir.h unit_test.c sed_cell_unit_test.c sed_wave_unit_test.c sed_column_unit_test.c sed_sediment_unit_test.c sed_hydro_unit_test.c

#libsedflux_la_LIBADD = -lpthread -lglib-2.0 -lgthread-2.0 -lutils
libsedflux_la_LIBADD = -lutils -lgthread-2.0 -lglib-2.0
libsedflux_la_LDFLAGS = -version-info 1:1
#test_LDADD = -lthread -lglib-2.0 -lgthread-2.0 -lutils -lsedflux
libsed_unit_test_LDADD = -lsedflux -lutils -lgthread-2.0 -lglib-2.0

EXTRA_DIST = sedflux.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sedflux.pc
$(pkgconfig_DATA): config.status

DISTCLEANFILES = datadir.h
datadir.h:
	echo '#if !defined(DATADIR)'
	echo '# define DATADIR "$(datadir)"' >$@
	echo '#endif'

