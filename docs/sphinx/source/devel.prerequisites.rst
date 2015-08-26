.. _sedflux_requires:

Prerequisites needed to build sedflux
=====================================

Prerequisites
-------------

To install *sedflux-2.0*, you need to have certain progams installed on you
machine. A list of these programs follows with the version numbers that I
used. It is possible that older version will work but I have not checked if
this is the case.

**Required**:
 * glib-2.0 (>= 2.16)
 * pkg-config (0.21)
 * gcc (2.95.3)
 * cmake (2.6)

**Optional**:
 * `check <http://check.sourceforge.net>`_ (0.9.4)
 * `doxygen <http://www.stack.nl/~dimitri/doxygen>`_ (1.5.1)

If you do not have some of the optional programs, you can still install
sedflux but with some loss of functionality. The limitations are:

 * *check*: You will not be able to run unit tests on your compiled code.
 * *doxygen*: You will not be able to generate documentation

The configure script will check for these packages and turn them on or off
depending are whether or not they are found.
