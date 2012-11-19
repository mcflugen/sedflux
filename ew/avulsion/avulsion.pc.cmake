Name: LibAvulsion
Description: Avulsion library
Version: ${AVULSION_VERSION}
Requires: glib-2.0 >= 2.25, utils >= 2.0, sed >= 2.0
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lavulsion
Cflags: -I${CMAKE_INSTALL_PREFIX}/include/ew-2.0 -I${CMAKE_INSTALL_PREFIX}/include

