Name: LibPlume
Description: Plume library
Version: ${PLUME_VERSION}
Requires: glib-2.0 >= 2.25, utils >= 2.0, sed >= 2.0
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lplume
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0 -I${CMAKE_INSTALL_PREFIX}/include

