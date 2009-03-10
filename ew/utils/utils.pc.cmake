Name: LibUtils
Description: My utility library
Version: ${LIBUTILS_VERSION}
Requires: glib-2.0 >= 2.0
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lutils -lm
Cflags: -I${CMAKE_INSTALL_PREFIX}/include

