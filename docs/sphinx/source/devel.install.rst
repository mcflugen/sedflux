.. _installing_sedflux:

Compiling and installing sedflux
================================

If you are interested in the TLDR version of this process, I've
created a script that does all this (without any description) in a
:download:`single file <build_sedflux.sh>`.

Please note that sedflux now uses `cmake <http://www.cmake.org/>`_ rather than
autotools as its build system.

Step 1: Install prerequisites
-----------------------------

Before attempting to compile sedflux make sure that you have all of the
:ref:`prerequisite packages <sedflux_requires>` You may not need the
particular versions; those are only the versions that I use.

Step 2: configure the distribution
----------------------------------

:ref:`Run cmake <sedflux_cmake>` to create the appropriate makefiles that
will then be used to compile sedflux.

Step 3: compile and install sedflux
-----------------------------------

:ref:`Build and install sedflux <sedflux_build_install>` with the generated
makefiles (assuming step 2 went OK).
