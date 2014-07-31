Name: LibPlume
Description: Plume library
Version: ${PLUME_VERSION}
Requires: glib-2.0 >= 2.25, utils, sed
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lbmiplume
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0 -I${CMAKE_INSTALL_PREFIX}/include

