## Comment line

bin_PROGRAMS = xpat vel advec
#xpat_SOURCES = xpat_main.c xpat.c advec.c
#vel_SOURCES = velocity_main.c xpat.c advec.c
#advec_SOURCES = advec_main.c advec.c xpat.c
xpat_SOURCES = xpat_main.c advec.c
vel_SOURCES = velocity_main.c advec.c
advec_SOURCES = advec_main.c advec.c

include_HEADERS = xpat.h

lib_LTLIBRARIES = libxpat.la
libxpat_la_SOURCES = xpat.c

#lib_LIBRARIES = libxpat.a
#libxpat_a_SOURCES = xpat.c

xpat_LDADD = -lxpat -lsedflux -lutils -lglib-2.0
vel_LDADD  = -lxpat -lsedflux -lutils -lglib-2.0
advec_LDADD  = -lxpat -lsedflux -lutils -lglib-2.0

