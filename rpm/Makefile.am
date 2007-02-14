WD=`pwd`
TOPDIRFLAGS=--define "_topdir ${WD}"
RPMFLAGS=
RPM_RELEASE=1

EXTRA_DIST=sedflux.spec sedflux.spec.in

SOURCE_FILE=${PACKAGE}-${VERSION}.tar.gz
RPM_I386=./RPMS/i386/${PACKAGE}-$(VERSION)-$(RPM_RELEASE).i386.rpm
RPM_SRC=./SRPMS/${PACKAGE}-$(VERSION)-$(RPM_RELEASE).src.rpm

RPMDIRS=BUILD SOURCES RPMS RPMS/i386 SRPMS

rpm: sedflux.spec
	mkdir -p ${RPMDIRS}
	cp ../${SOURCE_FILE} SOURCES
	rpmbuild ${TOPDIRFLAGS} ${RPMFLAGS} -ba sedflux.spec
	cp ${RPM_I386} ..
	cp ${RPM_SRC} ..
	rm -rf ${RPMDIRS}

CLEANFILES=*.*~ sedflux.spec
