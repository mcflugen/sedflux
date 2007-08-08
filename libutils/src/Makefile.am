lib_LTLIBRARIES        = libutils.la

libutils_la_SOURCES    = eh_input_val.c \
                         eh_data_record.c \
                         eh_symbol_table.c \
                         eh_key_file.c \
                         eh_ndgrid.c \
                         eh_grid.c \
                         eh_opt_context.c \
                         zbsubs.c \
                         eh_rand.c \
                         eh_logging.c \
                         eh_glib.c \
                         eh_scanner.c \
                         eh_get_opt.c \
                         complex.c \
                         eh_num.c \
                         machcon.c \
                         eh_mem.c \
                         eh_project.c \
                         eh_dlm_file.c \
                         eh_str.c \
                         eh_io.c \
                         eh_status_bar.c \
                         eh_polygon.c \
                         eh_sequence.c \
                         eh_misc.c \
                         eh_thread_pool.c \
                         eh_file_utils.c

utilsincludedir=$(includedir)/ew-2.0
utilsinclude_HEADERS = eh_utils.h

utilssubincludedir=$(includedir)/ew-2.0/utils
utilssubinclude_HEADERS = \
                         utils.h \
                         eh_rand.h \
                         eh_logging.h \
                         eh_scanner.h \
                         eh_glib.h \
                         eh_get_opt.h \
                         complex.h \
                         eh_num.h \
                         f2c.h \
                         eh_mem.h \
                         eh_project.h \
                         eh_opt_context.h \
                         eh_ndgrid.h \
                         eh_grid.h \
                         eh_key_file.h \
                         eh_symbol_table.h \
                         eh_data_record.h \
                         eh_types.h \
                         eh_input_val.h \
                         eh_dlm_file.h \
                         eh_str.h \
                         eh_io.h \
                         eh_status_bar.h \
                         eh_sequence.h \
                         eh_polygon.h \
                         eh_misc.h \
                         eh_thread_pool.h \
                         eh_file_utils.h \
                         eh_messages.h \
                         eh_macros.h

#include_HEADERS        = utils.h \
#                         eh_rand.h \
#                         eh_logging.h \
#                         eh_scanner.h \
#                         eh_glib.h \
#                         eh_get_opt.h \
#                         complex.h \
#                         eh_num.h \
#                         f2c.h \
#                         eh_mem.h \
#                         eh_project.h \
#                         eh_opt_context.h \
#                         eh_ndgrid.h \
#                         eh_grid.h \
#                         eh_key_file.h \
#                         eh_symbol_table.h \
#                         eh_data_record.h \
#                         eh_types.h \
#                         eh_input_val.h \
#                         eh_dlm_file.h \
#                         eh_str.h

bin_PROGRAMS           = test
test_SOURCES           = test.c

if ENABLE_CHECK

bin_PROGRAMS          += unit_test
unit_test_SOURCES      = \
                         unit_test.c \
                         eh_utils_unit_test.c \
                         eh_grid_unit_test.c \
                         eh_project_unit_test.c \
                         eh_num_unit_test.c \
                         eh_symbol_table_unit_test.c \
                         eh_input_val_unit_test.c \
                         eh_io_unit_test.c

unit_test_LDADD        = -lutils @GLIB_LIBS@ @CHECK_LIBS@ -lm

unit_test_CFLAGS       = @CHECK_CFLAGS@

endif

libutils_la_LIBADD     = @GLIB_LIBS@
libutils_la_LDFLAGS    = -version-info 1:1

if ENABLE_BLAS
libutils_la_LIBADD   += @BLAS_LIBS@
libutils_la_CFLAGS    = @BLAS_CFLAGS@
endif

test_LDADD             = -lutils @GLIB_LIBS@ -lutils -lm

EXTRA_DIST             = utils.pc.in
pkgconfigdir           = $(libdir)/pkgconfig
pkgconfig_DATA         = utils.pc

PURIFY = /home/huttone/local/bin/purify

PURE_LINK = $(PURIFY) $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
            $(AM_LDFLAGS) $(LDFLAGS) -o $@

pure: $(test_OBJECTS) $(test_DEPENDENCIES)
	@rm -f test$(EXEEXT)
	$(PURE_LINK) $(test_LDFLAGS) $(test_OBJECTS) $(test_LDADD) $(LIBS)

