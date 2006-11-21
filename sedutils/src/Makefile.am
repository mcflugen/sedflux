INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

EXTRA_DIST                = acsed.m4

bin_PROGRAMS              = read_hydro \
                            sedrescale \
                            getfloor \
                            sedextract \
                            sed2binary \
                            read_usgs \
                            create_hydro \
                            sedwheeler

read_hydro_SOURCES        = read_hydro.c
sedrescale_SOURCES        = sedrescale.c
getfloor_SOURCES          = getfloor.c
sedextract_SOURCES        = sedextract.c
sed2binary_SOURCES        = sed2binary.c
read_usgs_SOURCES         = read_usgs.c
create_hydro_SOURCES      = create_hydro.c
sedwheeler_SOURCES        = sedwheeler.c

LDADD                     = -lglib-2.0

