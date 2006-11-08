## Comment line

bin_PROGRAMS = compact compact_unit_test
compact_SOURCES = compact_main.c

lib_LTLIBRARIES = libcompact.la
libcompact_la_SOURCES = compact.c

compact_LDADD = -lcompact -lsedflux -lutils -lglib-2.0 -lm
compact_unit_test_SOURCES = compact_unit_test.c
compact_unit_test_LDADD = -lcompact -lsedflux -lutils -lglib-2.0 -lm

