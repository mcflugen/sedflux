Name: LibSed
Description: Sedflux library
Version: ${SEDFLUX_VERSION}
Requires: glib-2.0 >= 2.25, utils, sed
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lbmi_sedflux2d
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0 -I${CMAKE_INSTALL_PREFIX}/include

