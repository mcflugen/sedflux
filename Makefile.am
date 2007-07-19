#CFLAGS              += @GLIB_CFLAGS@ -I$(top_srcdir)/libutils/src

#libutils_dir         = $(top_builddir)/libutils/src
#libsed_dir           = $(top_builddir)/libsed/src

#LIBS                += -L$(libutils_dir) -L$(libsed_dir)

ACLOCAL_AMFLAGS      = -I m4

EXTRA_DIST           = bootstrap

SUBDIRS              = rpm \
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

#libutils: force_rebuild
#	@cd libutils; make install

#libsed: force_rebuild
#	@cd libsed/src && make datadir.h && cd .. &&  make install

#avulsion: force_rebuild
#	@cd avulsion; make install

#bing: force_rebuild
#	@cd bing; make install

#compact: force_rebuild
#	@cd compact; make install

#diffusion: force_rebuild
#	@cd diffusion; make install

#failure: force_rebuild
#	@cd failure; make install

#flow: force_rebuild
#	@cd flow; make install

#hydrotrend: force_rebuild
#	@cd hydrotrend; make install

#inflow: force_rebuild
#	@cd inflow; make install

#muds: force_rebuild
#	@cd muds; make install

#plume: force_rebuild
#	@cd plume; make install

#quake: force_rebuild
#	@cd quake; make install

#sakura: force_rebuild
#	@cd sakura; make install

#sedflux: force_rebuild
#	@cd sedflux; make install

#sedutils: force_rebuild
#	@cd sedutils; make install

#squall: force_rebuild
#	@cd squall; make install

#subside: force_rebuild
#	@cd subside; make install

#xshore: force_rebuild
#	@cd xshore; make install

#message: force_rebuild
#	@echo "---------------------------------------"
#	@echo "--- Making package $@"
#	@echo "---------------------------------------"

#docs: force_rebuild
#	@cd docs; make install

force_rebuild:

#dist-hook:
#	cd docs && make all
#	mkdir -p $(distdir)/docs && \
#	cp -r $(top_builddir)/docs/html $(distdir)/docs && \
#	cp -r $(top_builddir)/docs/latex $(distdir)/docs


