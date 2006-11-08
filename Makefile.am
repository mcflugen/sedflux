SUBDIRS = @SUBDIRS@

EXTRA_DIST = acsed.m4

package: libutils libsed subside xshore squall diffusion flow avulsion bing compact muds failure inflow sakura plume quake sedflux sedutils hydrotrend

libutils: force_rebuild
	cd libutils; make install

libsed: force_rebuild
	cd libsed; make install

subside: force_rebuild
	cd subside; make install

xshore: force_rebuild
	cd xshore; make install

squall: force_rebuild
	cd squall; make install

flow: force_rebuild
	cd flow; make install

xpat: force_rebuild
	cd xpat; make install

avulsion: force_rebuild
	cd avulsion; make install

bing: force_rebuild
	cd bing; make install

compact: force_rebuild
	cd compact; make install

diffusion: force_rebuild
	cd diffusion; make install

muds: force_rebuild
	cd muds; make install

failure: force_rebuild
	cd failure; make install

inflow: force_rebuild
	cd inflow; make install

sakura: force_rebuild
	cd sakura; make install

plume: force_rebuild
	cd plume; make install

quake: force_rebuild
	cd quake; make install

sedflux: force_rebuild
	cd sedflux; make install

sedutils: force_rebuild
	cd sedutils; make install

hydrotrend: force_rebuild
	cd hydrotrend; make install

force_rebuild:


#TAR = /usr/local/gnu/bin/tar

#dist-hook:
##	mkdir $(distdir)/mfiles
#	mkdir $(distdir)/demo
#	mkdir $(distdir)/demo/vail
##	cp -p $(srcdir)/mfiles/* $(distdir)/mfiles/*
#	for file in $(DEMO_FILES) ; do \
#	   cp -p $(srcdir)/demo/vail/$$file $(distdir)/demo/vail; \
#	done

