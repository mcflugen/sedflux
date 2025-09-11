# Install script for directory: /home/runner/work/sedflux/sedflux/ew/sed

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sed/libsedflux.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so"
         OLD_RPATH "/home/runner/work/sedflux/sedflux/_build/ew/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sed/CMakeFiles/sedflux.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/runner/work/sedflux/sedflux/_build/ew/sed/write_test_file")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/write_test_file")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sed/CMakeFiles/write_test_file.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-print-input")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/sed-print-input")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sed/CMakeFiles/sed-print-input.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ew-2.0" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sed/sed.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ew-2.0/sed" TYPE FILE FILES
    "/home/runner/work/sedflux/sedflux/ew/sed/csdms.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_cell.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_column.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_const.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_cube.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_diag.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_epoch.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_hydro.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_hydrotrend.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_process.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_property.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_property_file.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_river.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_sediment.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_signal.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_tripod.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/sed_wave.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/etk_addrem.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/etk_keyvalue.h"
    "/home/runner/work/sedflux/sedflux/ew/sed/datadir_path.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sed/sed.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/sedflux/sedflux/_build/ew/sed/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
