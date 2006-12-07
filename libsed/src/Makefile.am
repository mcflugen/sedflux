INCLUDES                 = -I$(top_srcdir)/libutils/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la

BUILT_SOURCES            = datadir_path.h

lib_LTLIBRARIES          = libsedflux.la libsedfluxui.la

libsedflux_la_SOURCES    = \
                           sed_cell.c \
                           sed_column.c \
                           sed_cube.c \
                           sed_epoch.c \
                           sed_hydro.c \
                           sed_process.c \
                           sed_property.c \
                           sed_property_file.c \
                           sed_sediment.c \
                           sed_tripod.c \
                           sed_wave.c

include_HEADERS          = \
                           sed_cell.h \
                           sed_column.h \
                           sed_const.h \
                           sed_cube.h \
                           sed_epoch.h \
                           sed_hydro.h \
                           sed_process.h \
                           sed_property.h \
                           sed_property_file.h \
                           sed_sedflux.h \
                           sed_sediment.h \
                           sed_tripod.h \
                           sed_wave.h \
                           etk_addrem.h \
                           etk_keyvalue.h

bin_PROGRAMS             = write_test_file

write_test_file_SOURCES  = sed_column_write_test_file.c
write_test_file_LDADD    = -lsedflux -lgthread-2.0 -lglib-2.0

if ENABLE_CHECK

bin_PROGRAMS            += libsed_unit_test

libsed_unit_test_SOURCES = \
                           unit_test.c \
                           sed_cell_unit_test.c \
                           sed_wave_unit_test.c \
                           sed_column_unit_test.c \
                           sed_sediment_unit_test.c \
                           sed_hydro_unit_test.c

libsed_unit_test_LDADD   = -lsedflux -lgthread-2.0 -lglib-2.0 @CHECK_LIBS@
libsed_unit_test_CFLAGS  = @CHECK_CFLAGS@

endif

if ENABLE_GTK

bin_PROGRAMS            += sedflux_ui select_bathy_ui

libsedfluxui_la_SOURCES  = etk_addrem.c etk_keyvalue.c stk_select_bathy.c

sedflux_ui_SOURCES       = sed_ui.c
sedflux_ui_CFLAGS        = @GTK_CFLAGS@
sedflux_ui_LDADD         = @GTK_LIBS@ -lsedfluxui

select_bathy_ui_SOURCES  = stk_select_bathy_main.c
select_bathy_ui_CFLAGS   = @GTK_CFLAGS@
select_bathy_ui_LDADD    = @GTK_LIBS@ -lsedfluxui 

libsedfluxui_la_LIBADD   = -lgthread-2.0 -lglib-2.0 @GTK_LIBS@
libsedfluxui_la_CFLAGS   = @GTK_CFLAGS@

endif

libsedflux_la_LIBADD     = -lgthread-2.0 -lglib-2.0
#libsedflux_la_LIBADD     = \
#                           /usr/local/sparcv9/lib/libgthread-2.0.so \
#                           /usr/local/sparcv9/lib/libglib-2.0.so
libsedflux_la_LDFLAGS    = -version-info 1:1


EXTRA_DIST               = sedflux.pc.in

pkgconfigdir             = $(libdir)/pkgconfig
pkgconfig_DATA           = sedflux.pc

DISTCLEANFILES           = datadir_path.h

datadir_path.h: Makefile
	if test -f $(srcdir)/datadir_path.h ; then :; else \
	   chmod u+w $(srcdir) ; \
	   echo '#if !defined(DATADIR)' > $(srcdir)/datadir_path.h ; \
	   echo '# define DATADIR "$(datadir)"' >> $(srcdir)/datadir_path.h ; \
	   echo '#endif' >> $(srcdir)/datadir_path.h ; \
	fi

