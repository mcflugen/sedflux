INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

#EXTRA_DIST                = acsed.m4

bin_PROGRAMS              = read_hydro \
                            sedrescale \
                            getfloor \
                            sedextract \
                            sed2binary \
                            read_usgs \
                            create_hydro \
                            sedwheeler \
                            sedflux-make-bathy \
                            sedflux-read-hydro \
                            sedflux-make-sequence

read_hydro_SOURCES            = read_hydro.c
sedrescale_SOURCES            = sedrescale.c
getfloor_SOURCES              = getfloor.c
sedextract_SOURCES            = sedextract.c
sed2binary_SOURCES            = sed2binary.c
read_usgs_SOURCES             = read_usgs.c
create_hydro_SOURCES          = create_hydro.c
sedwheeler_SOURCES            = sedwheeler.c
sedflux_make_bathy_SOURCES    = sedflux-make-bathy.c
sedflux_make_sequence_SOURCES = sedflux-make-sequence.c
sedflux_read_hydro_SOURCES    = sedflux-read-hydro.c

LDADD                     = -lglib-2.0 -lutils

