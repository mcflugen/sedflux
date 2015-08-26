.. _sedflux_cmake:

Configuring the build with CMake
================================

At this point you should have already downloaded and unpacked a source
distribution of sedflux.  This could be either from a tarball or from the
source repository.

Create a build directory
------------------------

Although not necessary, it is usually best if your build directory is
separate from the source directory.  If you're just installing sedflux from a
distribution, it's probably not that big of a deal; if you are doing any
development you'll definitely want them to be separated.  I usually just
create a build directory within the source directory (I like to call it
_build but you can call it whatever you like).

.. code-block:: bash

  > mkdir _build
  > cd _build

Configure the build
-------------------

You're now ready to run cmake.  Assuming that cmake is in your path, the
following will generate the necessary makefiles,

.. code-block:: bash

  > cmake .. -DBUILD_SHARED_LIBS=ON

With the latest version of sedflux you must define ``BUILD_SHARED_LIBS`` to be
``ON``.  It will not compile otherwise.  This will set up the build to
install sedflux in ``/usr/local/``.  If you would like to have sedflux
installed elsewhere, you need to define ``CMAKE_INSTALL_PREFIX`` to your
preferred install path.  A couple of examples,

.. code-block:: bash

  > cmake .. -DCMAKE_INSTALL_PREFIX=`pwd`/../_inst -DBUILD_SHARED_LIBS=ON

.. code-block:: bash

  > cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/sedflux -DBUILD_SHARED_LIBS=ON
