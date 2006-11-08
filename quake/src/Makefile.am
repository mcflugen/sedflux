## Comment line

#bin_PROGRAMS = earthquake quakeconvert_to quakeconvert_from
bin_PROGRAMS = earthquake 
earthquake_SOURCES = earthquake.c earthquake_main.c
#quakeconvert_to_SOURCES =
#quakeconvert_from_SOURCES =

lib_LIBRARIES = libquake.a
libquake_a_SOURCES = earthquake.c

#INCLUDES = -I$(top_srcdir)/../sed -I$(top_srcdir)/../utils

LDADD = -lutils -lglib-2.0

quakeconvert_to$(EXEEXT): earthquake
	ln -s earthquake quakeconvert_to

quakeconvert_from$(EXEEXT): earthquake
	ln -s earthquake quakeconvert_from

