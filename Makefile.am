ACLOCAL_AMFLAGS      = -I m4

EXTRA_DIST           = bootstrap \
                       CMakeLists.txt \
											 ew/CMakeLists.txt \
											 ew/avulsion/CMakeLists.txt \
											 ew/bing/CMakeLists.txt \
											 ew/bio/CMakeLists.txt \
											 ew/compact/CMakeLists.txt \
											 ew/diffusion/CMakeLists.txt \
											 ew/failure/CMakeLists.txt \
											 ew/flow/CMakeLists.txt \
											 ew/hydrotrend/CMakeLists.txt \
											 ew/inflow/CMakeLists.txt \
											 ew/muds/CMakeLists.txt \
											 ew/plume/CMakeLists.txt \
											 ew/quake/CMakeLists.txt \
											 ew/sakura/CMakeLists.txt \
											 ew/sed/CMakeLists.txt \
											 ew/sedflux/CMakeLists.txt \
											 ew/sedutils/CMakeLists.txt \
											 ew/squall/CMakeLists.txt \
											 ew/subside/CMakeLists.txt \
											 ew/utils/CMakeLists.txt \
											 ew/xshore/CMakeLists.txt

SUBDIRS              = deb  \
                       rpm  \
                       docs \
                       ew

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config-h.in \
                       stamp-h.in
#all: package

#package: $(SUBDIRS)

#libutils: force_rebuild
#	@cd libutils; make

#libsed: libutils
#	@cd libsed/src && make datadir.h && cd .. &&  make

utils: force_rebuild
	@cd utils; make

sed: utils
	@cd sed && make datadir.h &&  make

avulsion: utils sed
	@cd avulsion; make 

bing: utils sed
	@cd bing; make

compact: utils sed
	@cd compact; make

diffusion: utils sed
	@cd diffusion; make

failure: utils sed
	@cd failure; make

flow: utils sed
	@cd flow; make

hydrotrend: utils sed
	@cd hydrotrend; make

inflow: utils sed
	@cd inflow; make

muds: utils sed
	@cd muds; make

plume: utils sed
	@cd plume; make

quake: utils sed
	@cd quake; make

sakura: utils sed
	@cd sakura; make

sedflux: utils sed
	@cd sedflux; make

sedutils: utils sed
	@cd sedutils; make

squall: utils sed
	@cd squall; make

subside: utils sed
	@cd subside; make

xshore:  utils sed
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
	/Users/huttone/Desktop/googlecode_upload.py --summary="Nightly build (`date +%d/%m/%C%y`)" -P sedflux -u mcflugen -p f5e8p5y3 $(distdir).tar.gz

