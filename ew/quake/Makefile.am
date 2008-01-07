INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la

bin_PROGRAMS             = earthquake 
earthquake_SOURCES       = earthquake_main.c

lib_LTLIBRARIES          = libquake.la
libquake_la_SOURCES      = earthquake.c

LDADD                    = -lquake -lglib-2.0

quakeconvert_to$(EXEEXT): earthquake
	ln -s earthquake quakeconvert_to

quakeconvert_from$(EXEEXT): earthquake
	ln -s earthquake quakeconvert_from

