cmake_minimum_required(VERSION 2.6)

#include_directories(${CMAKE_SOURCE_DIR}/ew )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. )
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
#include_directories(${CMAKE_SOURCE_DIR}/ew/utils )
#include_directories( . )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/utils.pc.cmake  ${CMAKE_CURRENT_SOURCE_DIR}/utils.pc )

include (CheckFunctionExists)
Check_Function_Exists (getline HAVE_GETLINE)

if (HAVE_GETLINE)
  add_definitions (-DHAVE_GETLINE)
endif (HAVE_GETLINE)

########### next target ###############

SET(utils_LIB_SRCS
   eh_input_val.c
   eh_data_record.c
   eh_symbol_table.c
   eh_key_file.c
   eh_ndgrid.c
   eh_grid.c
   eh_opt_context.c
   zbsubs.c
   eh_rand.c
   eh_logging.c
   eh_glib.c
   eh_scanner.c
   eh_get_opt.c
   complex.c
   eh_num.c
   machcon.c
   eh_mem.c
   eh_project.c
   eh_dlm_file.c
   eh_str.c
   eh_io.c
   eh_status_bar.c
   eh_polygon.c
   eh_sequence.c
   eh_misc.c
   eh_thread_pool.c
   eh_file_utils.c
)

set_source_files_properties (${utils_LIB_SRCS} PROPERTIES LANGUAGE CXX)


add_library(utils ${utils_LIB_SRCS})
add_library(utils-static STATIC ${utils_LIB_SRCS})

target_link_libraries(utils m ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${EXTRA_LIBS})
#target_link_libraries(utils_STATIC m ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${EXTRA_LIBS})

install(TARGETS utils DESTINATION lib COMPONENT sedflux)
#install(TARGETS utils_STATIC DESTINATION lib COMPONENT sedflux)


########### next target ###############

#SET(test_scanner_SRCS test_scanner.c)

#set_source_files_properties (${test_scanner_SRCS} PROPERTIES LANGUAGE CXX)
#add_executable(test_scanner ${test_scanner_SRCS})

#target_link_libraries(test_scanner utils m)

#install(TARGETS test_scanner DESTINATION bin COMPONENT sedflux)

########### Optional unit tests ###############

if ( WITH_CHECK )
  set( unit_test_SRCS unit_test.c
                      eh_utils_unit_test.c
                      eh_grid_unit_test.c
                      eh_project_unit_test.c
                      eh_num_unit_test.c
                      eh_symbol_table_unit_test.c
                      eh_input_val_unit_test.c
                      eh_io_unit_test.c)

  add_executable( unit_test ${unit_test_SRCS} )

  target_link_libraries(unit_test utils check)
endif (WITH_CHECK)

set (key_file_tests_SRCS test_key_file.c)
add_executable (utils-test-key-file ${key_file_tests_SRCS})
target_link_libraries (utils-test-key-file utils)

set (input_val_tests_SRCS test_input_val.c)
add_executable (utils-test-input-val ${input_val_tests_SRCS})
target_link_libraries (utils-test-input-val utils)

set (io_tests_SRCS test_io.c)
add_executable (utils-test-io ${io_tests_SRCS})
target_link_libraries (utils-test-io utils)

set (symbol_table_tests_SRCS test_symbol_table.c)
add_executable (utils-test-symbol-table ${symbol_table_tests_SRCS})
target_link_libraries (utils-test-symbol-table utils)

set (grid_tests_SRCS test_grid.c)
set_source_files_properties (${grid_test_SRCS} PROPERTIES LANGUAGE CXX)
add_executable (utils-test-grid ${grid_tests_SRCS})
target_link_libraries (utils-test-grid utils)
set_target_properties (utils-test-grid PROPERTIES LINKER_LANGUAGE CXX)

set (num_tests_SRCS test_num.c)
add_executable (utils-test-num ${num_tests_SRCS})
target_link_libraries (utils-test-num utils)

########### install files ###############

install(FILES  eh_utils.h DESTINATION include/ew-2.0 COMPONENT sedflux)

install(FILES  utils.h                           eh_rand.h                           eh_logging.h                           eh_scanner.h                           eh_glib.h                           eh_get_opt.h                           complex.h                           eh_num.h                           f2c.h                           eh_mem.h                           eh_project.h                           eh_opt_context.h                           eh_ndgrid.h                           eh_grid.h                           eh_key_file.h                           eh_symbol_table.h                           eh_data_record.h                           eh_types.h                           eh_input_val.h                           eh_dlm_file.h                           eh_str.h                           eh_io.h                           eh_status_bar.h                           eh_sequence.h                           eh_polygon.h                           eh_misc.h                           eh_thread_pool.h                           eh_file_utils.h                           eh_messages.h                           eh_macros.h DESTINATION include/ew-2.0/utils COMPONENT sedflux)
install(FILES  utils.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#lib_LTLIBRARIES        = libutils.la
#
#libutils_la_SOURCES    = eh_input_val.c \
#                         eh_data_record.c \
#                         eh_symbol_table.c \
#                         eh_key_file.c \
#                         eh_ndgrid.c \
#                         eh_grid.c \
#                         eh_opt_context.c \
#                         zbsubs.c \
#                         eh_rand.c \
#                         eh_logging.c \
#                         eh_glib.c \
#                         eh_scanner.c \
#                         eh_get_opt.c \
#                         complex.c \
#                         eh_num.c \
#                         machcon.c \
#                         eh_mem.c \
#                         eh_project.c \
#                         eh_dlm_file.c \
#                         eh_str.c \
#                         eh_io.c \
#                         eh_status_bar.c \
#                         eh_polygon.c \
#                         eh_sequence.c \
#                         eh_misc.c \
#                         eh_thread_pool.c \
#                         eh_file_utils.c
#
#utilsincludedir=$(includedir)/ew-2.0
#utilsinclude_HEADERS = eh_utils.h
#
#utilssubincludedir=$(includedir)/ew-2.0/utils
#utilssubinclude_HEADERS = \
#                         utils.h \
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
#                         eh_str.h \
#                         eh_io.h \
#                         eh_status_bar.h \
#                         eh_sequence.h \
#                         eh_polygon.h \
#                         eh_misc.h \
#                         eh_thread_pool.h \
#                         eh_file_utils.h \
#                         eh_messages.h \
#                         eh_macros.h
#
#if ENABLE_CHECK
#
#bin_PROGRAMS           = test
#test_SOURCES           = test.c
#test_DEPENDENCIES      = libutils.la
#
#bin_PROGRAMS          += unit_test
#unit_test_SOURCES      = \
#                         unit_test.c \
#                         eh_utils_unit_test.c \
#                         eh_grid_unit_test.c \
#                         eh_project_unit_test.c \
#                         eh_num_unit_test.c \
#                         eh_symbol_table_unit_test.c \
#                         eh_input_val_unit_test.c \
#                         eh_io_unit_test.c
#
#unit_test_DEPENDENCIES = libutils.la
#
#unit_test_LDADD        = -lutils @GLIB_LIBS@ @CHECK_LIBS@ -lm
#
#unit_test_CFLAGS       = @CHECK_CFLAGS@
#
#endif
#
#libutils_la_LIBADD     = @GLIB_LIBS@
#libutils_la_LDFLAGS    = -version-info 1:1
#
#if ENABLE_BLAS
#libutils_la_LIBADD   += @BLAS_LIBS@
#libutils_la_CFLAGS    = @BLAS_CFLAGS@
#endif
#
#test_LDADD             = -lutils @GLIB_LIBS@ -lutils -lm
#
#EXTRA_DIST             = utils.pc.in
#pkgconfigdir           = $(libdir)/pkgconfig
#pkgconfig_DATA         = utils.pc
#
#PURIFY = /home/huttone/local/bin/purify
#
#PURE_LINK = $(PURIFY) $(CCLD) $(AM_CFLAGS) $(CFLAGS) \
#            $(AM_LDFLAGS) $(LDFLAGS) -o $@
#
#pure: $(test_OBJECTS) $(test_DEPENDENCIES)
#	@rm -f test$(EXEEXT)
#	$(PURE_LINK) $(test_LDFLAGS) $(test_OBJECTS) $(test_LDADD) $(LIBS)
#
