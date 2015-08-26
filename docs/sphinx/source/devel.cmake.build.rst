.. _sedflux_build_install:

Compile and Install sedflux
===========================

Compile
-------

To compile sedflux run make within the same directory that you ran cmake.

.. code-block:: bash

  > make

Depending on your machine this might take a little bit of time.  If you have
multiple processors available you can run parallel make to speed up the
process.  For instance, if you want to run make with 8 processes::

  > make -j8

Install
-------

Once the build has finished, you can install the package.  Note that if you
just want to run sedflux from the build directory, you can do that and skip
this step.  However, if you want to install sedflux::

  > make install

This will copy all of the sedflux installation files to the location you
specified during the configuration process. Note that you will have to have
write permission for the directory that you are installing into.  sedflux
installation files will be installed under ``<prefix>/bin``,
``<prefix>/include``, and ``<prefix>/lib``. 
