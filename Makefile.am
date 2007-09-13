ACLOCAL_AMFLAGS      = -I m4

EXTRA_DIST           = bootstrap

SUBDIRS              = deb \
                       rpm \
                       docs \
                       libutils \
                       libsed  \
                       avulsion  \
                       bing  \
                       bio  \
                       compact  \
                       diffusion  \
                       failure  \
                       flow  \
                       inflow  \
                       muds  \
                       plume  \
                       quake  \
                       sakura  \
                       squall  \
                       subside  \
                       xshore  \
                       sedflux  \
                       hydrotrend  \
                       sedutils 

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config-h.in \
                       stamp-h.in
#all: package

#package: $(SUBDIRS)

libutils: force_rebuild
	@cd libutils; make

libsed: libutils
	@cd libsed/src && make datadir.h && cd .. &&  make

avulsion: libutils libsed
	@cd avulsion; make 

bing: libutils libsed
	@cd bing; make

compact: libutils libsed
	@cd compact; make

diffusion: libutils libsed
	@cd diffusion; make

failure: libutils libsed
	@cd failure; make

flow: libutils libsed
	@cd flow; make

hydrotrend: libutils libsed
	@cd hydrotrend; make

inflow: libutils libsed
	@cd inflow; make

muds: libutils libsed
	@cd muds; make

plume: libutils libsed
	@cd plume; make

quake: libutils libsed
	@cd quake; make

sakura: libutils libsed
	@cd sakura; make

sedflux: libutils libsed
	@cd sedflux; make

sedutils: libutils libsed
	@cd sedutils; make

squall: libutils libsed
	@cd squall; make

subside: libutils libsed
	@cd subside; make

xshore:  libutils libsed
	@cd xshore; make

message: force_rebuild
	@echo "---------------------------------------"
	@echo "--- Making package $@"
	@echo "---------------------------------------"

docs: force_rebuild
	@cd docs; make install

dist-deb: force_rebuild
	cd deb \
	&& make deb

dist-rpm: force_rebuild
	cd rpm \
	&& make rpm

force_rebuild:

#dist-hook:
#	cd docs && make all
#	mkdir -p $(distdir)/docs && \
#	cp -r $(top_builddir)/docs/html $(distdir)/docs && \
#	cp -r $(top_builddir)/docs/latex $(distdir)/docs

dist-bin: distdir
	mkdir -p $(distdir)/_inst
	mkdir -p $(distdir)/_build
	db_install_base=`$(am__cd) $(distdir)/_inst && pwd` \
	   && cd $(distdir)/_build \
	   && ../configure --srcdir=.. --prefix="$$db_install_base" --disable-shared --enable-static $(DISTCHECK_CONFIGURE_FLAGS) \
	   && $(MAKE) $(AM_MAKEFLAGS) \
	   && $(MAKE) $(AM_MAKEFLAGS) install \
	   && cd .. \
	   && mv _inst ${PACKAGE}-${VERSION} \
	   && tar cvfz ../$(top_builddir)/${PACKAGE}_${VERSION}_@DPKG_ARCHITECTURE@.tar.gz ${PACKAGE}-${VERSION} \
	   && rm -r _build ${PACKAGE}-${VERSION}
	$(am__remove_distdir)

nightly: distcheck
	/Users/huttone/googlecode_upload.py --summary="Nightly build (`date +%d/%m/%C%y`)" -p sedflux -u mcflugen $(distdir).tar.gz

