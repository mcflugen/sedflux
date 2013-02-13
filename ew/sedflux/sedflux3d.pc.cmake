Name: LibSed
Description: Sedflux library
Version: ${SEDFLUX_VERSION}
Requires: glib-2.0 >= 2.25, utils >= 2.0, sed >= 2.0
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lbmisedflux3d
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0

