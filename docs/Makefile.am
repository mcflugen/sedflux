AUTOMAKE_OPTIONS = 1.6

TARGET_DIR=$(HTML_DIR)

RUN_DOXYGEN      = @DOXYGEN@

EXTRA_DIST       = main_page.txt sedflux-doxyfile.in
pkgdata_DATA     = 

DOC_STAMPS=html-build.stamp
CLEANFILES=$(DOC_STAMPS)

if ENABLE_DOXYGEN
all-local: html-build.stamp
else
all-local:
endif

docs: html-build.stamp

html-build.stamp:
	@echo "Building HTML Documentation..."
	@-chmod -R u+w $(srcdir)
	rm -rf $(srcdir)/html
	mkdir $(srcdir)/html
	@${RUN_DOXYGEN} sedflux-doxyfile > /dev/null
	touch html-build.stamp

install-data-local:
	installfiles=`echo $(srcdir)/html/*`; \
	if test "$$installfiles" = '$(srcdir)/html/*'; then \
	   echo '--- Nothing to install' ; \
	else \
	   $(mkinstalldirs) $(DESTDIR)$(TARGET_DIR); \
	   for i in $$installfiles; do \
	      echo '--- Installing '$$i ; \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	   done; \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(TARGET_DIR)/*

dist-hook:
	mkdir $(distdir)/html
	-cp $(srcdir)/html/* $(distdir)/html


