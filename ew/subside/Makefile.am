INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS              = subside
subside_SOURCES           = subside_main.c
subside_DEPENDENCIES      = libsubside.la
subsideinclude_HEADERS    = subside.h
subsideincludedir         = $(includedir)/ew-2.0

lib_LTLIBRARIES           = libsubside.la
libsubside_la_SOURCES     = subside.c

subside_LDADD             = -lsubside -lglib-2.0 -lm

if ENABLE_CHECK

bin_PROGRAMS                   += subside_unit_test
subside_unit_test_SOURCES       = subside_unit_test.c
subside_unit_test_DEPENDENCIES  = libsubside.la

subside_unit_test_LDADD   = -lsubside -lglib-2.0 -lm @CHECK_LIBS@
subside_unit_test_CFLAGS  = @CHECK_CFLAGS@

endif
