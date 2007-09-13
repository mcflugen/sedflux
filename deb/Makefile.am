EXTRA_DIST=control control.in

DPKG_DEB=dpkg-deb
SYS_NAME=`uname -s`-`uname -m`

all: control

deb:
	mkdir -p debian
	mkdir -p debian/DEBIAN
	mkdir -p debian/usr/local/ew
	cp control debian/DEBIAN
	mkdir -p _build \
	&& cd _build \
	&& DEB_SRCDIR=`(cd ../$(top_srcdir) ; pwd)` \
	&& DEB_PREFIX=`(cd ../debian/usr/local/ew ; pwd)` \
	&& ../$(top_srcdir)/configure --srcdir=$$DEB_SRCDIR --prefix=$$DEB_PREFIX --disable-shared --enable-static $(DISTCHECK_CONFIGURE_FLAGS) \
	&& $(MAKE) $(AM_MAKEFLAGS) \
	&& $(MAKE) $(AM_MAKEFLAGS) install \
	&& cd .. \
	&& $(DPKG_DEB) --build debian ${PACKAGE}_${VERSION}_${SYS_NAME}.deb \
	&& cd _build \
	&& $(MAKE) $(AM_MAKEFLAGS) uninstall \
	&& $(MAKE) $(AM_MAKEFLAGS) clean \
	&& cd ..
	rm -rf debian _build

#CLEANFILES=*.*~ sedflux.spec
