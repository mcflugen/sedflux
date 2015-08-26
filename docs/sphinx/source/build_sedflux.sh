#!/bin/bash

# This is a script that lists the commands to download, compile, and install
# sedflux.  This is not intended to be robust in any way.  It probably
# WILL NOT work as-is for your system.  It's intent is only to provide a
# guide of what needs to be done to compile sedflux.  Most likely, you will
# have to make changes to this script for it to work on your system.

# The version of sedflux that you want to use
VERSION=2.0.204-r231

# Fetch the source
wget http://csdms.colorado.edu/pub/models/sedflux/sedflux-$VERSION.tar.gz

# Unpack the source tarball
tar xvfz sedflux-$VERSION.tar.gz 
cd sedflux-$VERSION

# Set up a build directory
mkdir _build && cd _build

# Configure, build, and install
cmake .. -DCMAKE_INSTALL_PREFIX=`pwd`/../_inst -DBUILD_SHARED_LIBS=ON
make -j8 && make install

