Name: LibSubside
Description: Subside library
Version: ${SUBSIDE_VERSION}
Requires: glib-2.0 >= 2.25, utils, sed
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lbmi_subside
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0 -I${CMAKE_INSTALL_PREFIX}/include

