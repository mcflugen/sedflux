.. _sedflux_module_avulsion:

Avulsion
========

Within sedflux, the avulsion process is configured in the
:ref:`sedflux process file <sedflux_process_file>`.  A snippet from a sedflux
process file that shows how avulsion might be configured.

.. code-block:: ini

  [ avulsion ]
  process name:                            avulsion
  active:                                  yes
  logging:                                 yes
  repeat interval:                         always
  standard deviation:                      1
  minimum angle:                           0
  maximum angle:                           180
  hinge point:                             40,0
  fraction of sediment remaining in plane: 1.
  river can branch?:                       no
  river name:                              Mississippi
  seed for random number generator:        1945

The first four parameters are general module parameters for sedflux and must
be given for every :ref:`group in the process file <sedflux_process_file>`.
The remaining parameters are specific to the avulsion module.

 * ``standard deviation``: The change in angle of the river is drawn from a
   normal distribution with this standard deviation (in radians)
 * ``minimum angle``: The lower limit of the river angle with respect to the
   positive x-axis (in degrees)
 * ``maximum angle``: The upper limit of the river angle with respect to the
   positive x-axis (in degrees)
 * ``hinge point``: The location of the hinge point - the point where the
   river is fixed and rotates about.  This is given as row and column indices
   of the input :ref:`bathymetry file <sedflux_bathy_file>`.
 * ``fraction of sediment remaining in plane``: Amount of river sediment that
   makes it to the river mouth (as a fraction between 0 and 1).  Adjust this
   value if you don't want all of the river sediment to reach the ocean
   (otherwise, set it to 1).
 * ``river can branch?``: For now set this to ``no``.  River branching is
   still in development and may not work properly (or at all).
 * ``river name``: The name of the river.  Used to associate this avulsion
   process to a particular river that was specified by another process module.
 * ``seed for random number generator``: An integer used as an initial seed
   for the random number generator that drives the avulsion process.

The troublesome part of this configuration is setting the angle limits and
the location of the hinge point.  To understand how to set these parameters
you first need to understand how the coordinate system that sedflux uses
relates to your [wiki:SedfluxFileBathymetry bathymetry file].  Some key facts:

 #. The bathymetry file lists elevations row-by-row, where each row is
    constant x (and so, each column is constant y).
 #. Angles are measured with respect to the positive x-axis.
 #. Row and columns are indexed starting from 0.

As an example, let's take a simple bathymetry file that defines a plane
dipping in the positive y-direction.  Such a file might look like::

  0, -10, -20, -30, -40
  0, -10, -20, -30, -40
  0, -10, -20, -30, -40

In this case, we would probably want the hinge point to be at the first row
and the zeroth column (in the middle of the left edge).  This means setting
the hinge point to ``1, 0`` in the above configuration file.  Given this hinge
point, to limit the river to positions that point to the right a good choice
for angle limits might be from 0 degrees to 180 degrees (the half-plane
containing all positive y).
