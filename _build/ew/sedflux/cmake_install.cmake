# Install script for directory: /home/runner/work/sedflux/sedflux/ew/sedflux

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "sedflux" FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/run_sedflux")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libsedflux-2.0.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so"
         OLD_RPATH "/home/runner/work/sedflux/sedflux/_build/ew/sakura:/home/runner/work/sedflux/sedflux/_build/ew/avulsion:/home/runner/work/sedflux/sedflux/_build/ew/bio:/home/runner/work/sedflux/sedflux/_build/ew/diffusion:/home/runner/work/sedflux/sedflux/_build/ew/subside:/home/runner/work/sedflux/sedflux/_build/ew/xshore:/home/runner/work/sedflux/sedflux/_build/ew/squall:/home/runner/work/sedflux/sedflux/_build/ew/flow:/home/runner/work/sedflux/sedflux/_build/ew/muds:/home/runner/work/sedflux/sedflux/_build/ew/bing:/home/runner/work/sedflux/sedflux/_build/ew/failure:/home/runner/work/sedflux/sedflux/_build/ew/quake:/home/runner/work/sedflux/sedflux/_build/ew/compact:/home/runner/work/sedflux/sedflux/_build/ew/inflow:/home/runner/work/sedflux/sedflux/_build/ew/plume:/home/runner/work/sedflux/sedflux/_build/ew/sed:/home/runner/work/sedflux/sedflux/_build/ew/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsedflux-2.0.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/sedflux-2.0.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/sedflux.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/sedflux2d.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/sedflux3d.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/sedgrid.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedflux2d.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so"
         OLD_RPATH "/home/runner/work/sedflux/sedflux/_build/ew/sakura:/home/runner/work/sedflux/sedflux/_build/ew/avulsion:/home/runner/work/sedflux/sedflux/_build/ew/bio:/home/runner/work/sedflux/sedflux/_build/ew/diffusion:/home/runner/work/sedflux/sedflux/_build/ew/subside:/home/runner/work/sedflux/sedflux/_build/ew/xshore:/home/runner/work/sedflux/sedflux/_build/ew/squall:/home/runner/work/sedflux/sedflux/_build/ew/flow:/home/runner/work/sedflux/sedflux/_build/ew/muds:/home/runner/work/sedflux/sedflux/_build/ew/bing:/home/runner/work/sedflux/sedflux/_build/ew/failure:/home/runner/work/sedflux/sedflux/_build/ew/quake:/home/runner/work/sedflux/sedflux/_build/ew/compact:/home/runner/work/sedflux/sedflux/_build/ew/inflow:/home/runner/work/sedflux/sedflux/_build/ew/plume:/home/runner/work/sedflux/sedflux/_build/ew/sed:/home/runner/work/sedflux/sedflux/_build/ew/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux2d.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedflux2d.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedflux2d.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedflux2d-static.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedflux2d" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi_sedflux2d.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedflux2d" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedflux3d.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so"
         OLD_RPATH "/home/runner/work/sedflux/sedflux/_build/ew/sakura:/home/runner/work/sedflux/sedflux/_build/ew/avulsion:/home/runner/work/sedflux/sedflux/_build/ew/bio:/home/runner/work/sedflux/sedflux/_build/ew/diffusion:/home/runner/work/sedflux/sedflux/_build/ew/subside:/home/runner/work/sedflux/sedflux/_build/ew/xshore:/home/runner/work/sedflux/sedflux/_build/ew/squall:/home/runner/work/sedflux/sedflux/_build/ew/flow:/home/runner/work/sedflux/sedflux/_build/ew/muds:/home/runner/work/sedflux/sedflux/_build/ew/bing:/home/runner/work/sedflux/sedflux/_build/ew/failure:/home/runner/work/sedflux/sedflux/_build/ew/quake:/home/runner/work/sedflux/sedflux/_build/ew/compact:/home/runner/work/sedflux/sedflux/_build/ew/inflow:/home/runner/work/sedflux/sedflux/_build/ew/plume:/home/runner/work/sedflux/sedflux/_build/ew/sed:/home/runner/work/sedflux/sedflux/_build/ew/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedflux3d.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedflux3d.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedflux3d.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedflux3d-static.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedflux3d" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi_sedflux3d.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedflux3d" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedgrid.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so"
         OLD_RPATH "/home/runner/work/sedflux/sedflux/_build/ew/sakura:/home/runner/work/sedflux/sedflux/_build/ew/avulsion:/home/runner/work/sedflux/sedflux/_build/ew/bio:/home/runner/work/sedflux/sedflux/_build/ew/diffusion:/home/runner/work/sedflux/sedflux/_build/ew/subside:/home/runner/work/sedflux/sedflux/_build/ew/xshore:/home/runner/work/sedflux/sedflux/_build/ew/squall:/home/runner/work/sedflux/sedflux/_build/ew/flow:/home/runner/work/sedflux/sedflux/_build/ew/muds:/home/runner/work/sedflux/sedflux/_build/ew/bing:/home/runner/work/sedflux/sedflux/_build/ew/failure:/home/runner/work/sedflux/sedflux/_build/ew/quake:/home/runner/work/sedflux/sedflux/_build/ew/compact:/home/runner/work/sedflux/sedflux/_build/ew/inflow:/home/runner/work/sedflux/sedflux/_build/ew/plume:/home/runner/work/sedflux/sedflux/_build/ew/sed:/home/runner/work/sedflux/sedflux/_build/ew/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libbmi_sedgrid.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedgrid.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/libbmi_sedgrid.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/runner/work/sedflux/sedflux/_build/ew/sedflux/CMakeFiles/bmi_sedgrid-static.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedgrid" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi_sedgrid.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sedgrid" TYPE FILE FILES "/home/runner/work/sedflux/sedflux/ew/sedflux/bmi.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "sedflux" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ew-2.0" TYPE FILE FILES
    "/home/runner/work/sedflux/sedflux/ew/sedflux/sedflux.h"
    "/home/runner/work/sedflux/sedflux/ew/sedflux/sedflux_api.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
