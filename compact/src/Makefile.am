INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = compact
compact_SOURCES           = compact_main.c

lib_LTLIBRARIES           = libcompact.la
libcompact_la_SOURCES     = compact.c

compact_LDADD             = -lcompact -lglib-2.0 -lm

if ENABLE_CHECK

bin_PROGRAMS             += compact_unit_test
compact_unit_test_SOURCES = compact_unit_test.c

compact_unit_test_LDADD   = -lcompact -lglib-2.0 -lm @CHECK_LIBS@
compact_unit_test_CFLAGS  = @CHECK_CFLAGS@

endif

