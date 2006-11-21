INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = flow flow_2d flow_3d
flow_SOURCES              = flow_main.c
flow_2d_SOURCES           = flow_2d_main.c
flow_3d_SOURCES           = flow_3d_main.c
include_HEADERS           = flow.h

lib_LTLIBRARIES             = libflow.la
libflow_la_SOURCES         = flow.c

flow_LDADD                = -lflow -lglib-2.0
flow_2d_LDADD             = -lflow -lglib-2.0 
flow_3d_LDADD             = -lflow -lglib-2.0 

