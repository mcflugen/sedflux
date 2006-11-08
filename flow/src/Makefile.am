## Comment line

bin_PROGRAMS = flow flow_2d flow_3d
flow_SOURCES = flow_main.c flow.c
flow_2d_SOURCES = flow_2d_main.c flow.c
flow_3d_SOURCES = flow_3d_main.c flow.c
include_HEADERS = flow.h

lib_LIBRARIES = libflow.a

libflow_a_SOURCES = flow.c

flow_LDADD = -lsedflux -lutils -lglib-2.0
flow_2d_LDADD = -lsedflux -lutils -lglib-2.0 
flow_3d_LDADD = -lsedflux -lutils -lglib-2.0 

#noinst_HEADERS = bing.h

