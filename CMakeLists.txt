cmake_minimum_required(VERSION 2.8)

project(sedflux)
include(CTest)

set(CMAKE_MACOSX_RPATH 1)


#enable_testing()

if (BUILD_TESTING)
  add_test (Help ${CMAKE_CURRENT_BINARY_DIR}/ew/sedflux/run_sedflux --help)
  add_test (SedCell gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-cell)
  add_test (SedColumn gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-column)
  add_test (SedCube gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-cube)
  add_test (SedHydro gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-hydro)
  add_test (SedRiver gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-river)
  add_test (SedWave gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/sed/sed-test-wave)
  add_test (UtilsGrid gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/utils/utils-test-grid)
  add_test (UtilsIO gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/utils/utils-test-io)
  add_test (UtilsKeyFile gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/utils/utils-test-key-file)
  add_test (UtilsNum gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/utils/utils-test-num)
  add_test (UtilsSymbolTable gtester ${CMAKE_CURRENT_BINARY_DIR}/ew/utils/utils-test-symbol-table)
endif()

set (BUILD_SHARED_LIBS ON)

# if (APPLE)
#   include_directories (/Developer/Headers/FlatCarbon)
#   find_library (CARBON_LIBRARY Carbon)
#   mark_as_advanced (CARBON_LIBRARY)
#   set (EXTRA_LIBS ${CARBON_LIBRARY})
# endif (APPLE)

########### Look for glib-2.0 ###############

if (DEFINED CMAKE_GLIB_DIR)
  set (ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_GLIB_DIR}/lib/pkgconfig")
endif (DEFINED CMAKE_GLIB_DIR)

include( FindPkgConfig )
pkg_check_modules( GLIB2 glib-2.0 )
pkg_check_modules( GTHREAD2 gthread-2.0 )
include_directories( ${GLIB2_INCLUDE_DIRS} ${GTHREAD2_INCLUDE_DIRS} )
link_directories( ${GLIB2_LIBRARY_DIRS} ${GTHREAD2_LIBRARY_DIRS} )

########### Look for check ###############

option( WITH_CHECK   "Enable unit testing with check" OFF )
if ( WITH_CHECK )
  pkg_check_modules( CHECK check )
  include_directories( ${CHECK_INCLUDE_DIRS} )
  link_directories( ${CHECK_LIBRARY_DIRS} )
endif ( WITH_CHECK )

########### Look for Doxygen ###############

option( WITH_DOXYGEN "Build API docs using Doxygen" OFF )
if ( WITH_DOXYGEN )
  find_program( DOXYGEN doxygen )
  include( FindDoxygen )
  if (DOXYGEN)
    add_subdirectory(doxygen)
  endif (DOXYGEN)
endif ( WITH_DOXYGEN )

########### Look for BLAS ###############

option( WITH_BLAS   "Enable BLAS libraries" OFF )
if (WITH_BLAS)
  enable_language( Fortran )
  include( FindBLAS )
endif (WITH_BLAS)

########### Path to libintl  ###############
find_library(LIBINTL intl
             HINTS ${CMAKE_LIBINTL_DIR})
if (NOT LIBINTL)
  message(STATUS
          "Looking for libintl... - NOT found.  Try setting CMAKE_LIBINTL_DIR") 
endif (NOT LIBINTL)

get_filename_component( LIBINTL_DIR ${LIBINTL} PATH )
message(STATUS "Looking for libintl... - found ${LIBINTL_DIR}") 
link_directories( ${LIBINTL_DIR} )

########### Version number ###############
#set (TARBALL_VERSION ${CMAKE_SOURCE_DIR}/build-aux/tarball-version)
#set (PACKAGE_VERSION ${CMAKE_SOURCE_DIR}/build-aux/package-version)
#set (VERSION_GEN ${CMAKE_SOURCE_DIR}/build-aux/version-gen)
#execute_process (
#  COMMAND ${VERSION_GEN} ${TARBALL_VERSION} ${PACKAGE_VERSION}
#  OUTPUT_FILE ${CMAKE_SOURCE_DIR}/.version
#  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#file (STRINGS .version version LIMIT_COUNT 1)
#string (REPLACE "." ";" v "${version}")
#list (GET v 0 MAJOR_VERSION)
#list (GET v 1 MINOR_VERSION)
#list (LENGTH v len)
#if (len GREATER 2)
#  list (GET v 2 PATCH_VERSION)
#else ()
#  set (PATCH_VERSION 0)
#endif ()
#string (REGEX MATCH "([0-9]+)" RELEASE_VERSION ${PATCH_VERSION})

#set (SEDFLUX_VERSION ${MAJOR_VERSION}.${MINOR_VERSION})

########### Create a distribution ###############
set (CPACK_PACKAGE_NAME sedflux)
set (CPACK_PACKAGE_INSTALL_DIRECTORY /usr/local/csdms/sedflux)
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "sedflux is a basin-filling model")
set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/License.txt)
set (CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${RELEASE_VERSION})
set (CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set (CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set (CPACK_SOURCE_IGNORE_FILES "/_build/;/_inst/;/.git/")

include( CPack )

add_custom_target (dist
  COMMAND ${CMAKE_SOURCE_DIR}/build-aux/dist-version-get ${CMAKE_SOURCE_DIR}/.version > ${CMAKE_SOURCE_DIR}/build-aux/tarball-version
  COMMAND ${CMAKE_MAKE_PROGRAM} package_source
  COMMAND rm ${CMAKE_SOURCE_DIR}/build-aux/tarball-version)

########### Run unit tests ###############
find_program (GTESTER gtester HINTS ${CMAKE_GLIB_DIR}/bin)
if (NOT GTESTER)
  message(STATUS
          "Looking for gtester... - NOT found.  Try setting CMAKE_GLIB_DIR") 
else (NOT GTESTER)
  message(STATUS "Looking for gtester... - found ${GTESTER}") 
endif (NOT GTESTER)
add_custom_target (unit_tests
  COMMAND ${GTESTER} ew/utils/utils-test-* -m=quick
  COMMAND ${GTESTER} ew/sed/sed-test-* -m=quick)
add_custom_target (unit_tests_slow
  COMMAND ${GTESTER} ew/utils/utils-test-* -m=slow
  COMMAND ${GTESTER} ew/sed/sed-test-* -m=slow)

########### Build in subdirectories ###############
add_subdirectory(ew)

########### install files ###############


########### Configuration Information ###############

if ( CMAKE_BUILD_TYPE MATCHES Release )
  set( cflags ${CMAKE_C_FLAGS_RELEASE} )
elseif ( CMAKE_BUILD_TYPE MATCHES Debug )
  set( cflags ${CMAKE_C_FLAGS_DEBUG} )
else ( )
  set( cflags ${CMAKE_C_FLAGS} )
endif ( CMAKE_BUILD_TYPE MATCHES Release )

message("------------------------------------------------------------------------")
message("Configuration:")
message("")
message("  sedflux version:            ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
message("  Source code location:       ${CMAKE_SOURCE_DIR}")
message("  Build type:                 ${CMAKE_BUILD_TYPE}")
message("  Compiler:                   ${CMAKE_C_COMPILER}")
message("  Compiler flags:             ${cflags}")
message("  Host System Type:           ${CMAKE_HOST_SYSTEM}")
message("  Installation architecture:  ${CMAKE_SYSTEM}")
message("  Install path:               ${CMAKE_INSTALL_PREFIX}")
message("  With gtk+-2.0:              ${GTK_VERSION}")
message("  With check:                 ${CHECK_VERSION}")
message("  With doxygen:               ${DOXYGEN}")
message("  With flex:                  ${FLEX_VERSION}")
message("  HTML path:                  ${HTML_DIR}")
message("  BLAS path:                  ${with_blas_dir}")
message("  BLAS LIBS:                  ${BLAS_LIBS}")
message("  BLAS CFLAGS:                ${BLAS_CFLAGS}")
message("")
message("  See config.h for further configuration information.")
message("------------------------------------------------------------------------")

