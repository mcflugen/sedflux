# Install script for directory: /home/runner/work/sedflux/sedflux/ew/utils

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/utils/libutils.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libutils.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/utils/CMakeFiles/utils.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ew-2.0" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/utils/eh_utils.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ew-2.0/utils" TYPE FILE FILES
    "/home/runner/work/sedflux/sedflux/ew/utils/utils.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_rand.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_logging.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_scanner.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_glib.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_get_opt.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/complex.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_num.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/f2c.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_mem.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_project.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_opt_context.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_ndgrid.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_grid.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_key_file.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_symbol_table.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_data_record.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_types.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_input_val.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_dlm_file.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_str.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_io.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_status_bar.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_sequence.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_polygon.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_misc.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_thread_pool.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_file_utils.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_messages.h"
    "/home/runner/work/sedflux/sedflux/ew/utils/eh_macros.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/utils/utils.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/sedflux/sedflux/_build/ew/utils/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
