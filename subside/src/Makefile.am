## Comment line

bin_PROGRAMS = subside
subside_SOURCES = subside_main.c
include_HEADERS = subside.h

lib_LTLIBRARIES = libsubside.la
libsubside_la_SOURCES = subside.c

#INCLUDES = -I$(top_srcdir)/../sed -I$(top_srcdir)/../utils -I/usr/freeware/include

subside_LDADD = -lsubside -lsedflux -lutils -lglib-2.0 -lm

