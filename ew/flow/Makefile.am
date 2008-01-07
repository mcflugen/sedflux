INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS              = flow flow_2d flow_3d
flow_SOURCES              = flow_main.c
flow_2d_SOURCES           = flow_2d_main.c
flow_3d_SOURCES           = flow_3d_main.c
flowinclude_HEADERS       = flow.h
flowincludedir            = $(includedir)/ew-2.0

lib_LTLIBRARIES             = libflow.la
libflow_la_SOURCES         = flow.c

flow_LDADD                = -lflow -lglib-2.0
flow_2d_LDADD             = -lflow -lglib-2.0 
flow_3d_LDADD             = -lflow -lglib-2.0 

