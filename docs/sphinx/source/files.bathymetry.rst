.. _sedflux_bathy_file:

The sedflux Bathymetry file
===========================

The :ref:`sedflux initialization file <sedflux_init_file>` lists the file that
defines the initial bathymetry for a sedflux simulation. The format of the file
depends on whether sedflux will be run in 2D or 3D mode. In either case,
though, all elevations are measured in meters.

2D mode
-------

When run in 2D mode, sedflux only requires bathymetric elevations along a
profile. This is provided as comma-separated columns with each row being a
point along the profile. The first column is horizontal position of the point
(in meters), and the second elevation (in meters). As an example::

  0.00, 5.00
  300.00, 3.00
  500.00, 1.00
  1329.4, -10.9
  5200.0, -20.4
  11282.4, -29.6
  15540.0, -38.0
  22894.1, -48.0
  25890.0, -69.0
  27317.6, -77.0
  27732.4, -79.0
  28976.5, -89.0
  30082.4, -99.0
  30911.8, -109.0
  32017.6, -113.0

In this case the profile is just over 32 km long and drops 118 meters.
The points need not be evenly spaced; sedflux will use linear interpolation to
estimate elevations between points.

3D mode
-------

The contents of the bathymetry is a matrix of elevations (in meters). Unlike
the 2D version, sedflux does not interpolate between points. The points are
spaced uniformly with the spacing specified by the x and y resolutions in the
:ref:`sedflux initialization file <sedflux_init_file>`. As an example::

  33,-8   ,-8.1 ,-8.2 ,-8.3 ,-8.4 ,-8.5 ,-8.6 ,-8.7 ,-8.8
  33,-11  ,-11.1,-11.2,-11.3,-11.4,-11.5,-11.6,-11.7,-11.8
  33,-13  ,-13.1,-13.2,-13.3,-13.4,-13.5,-13.6,-13.7,-13.8
  33,-14  ,-14.1,-14.2,-14.3,-14.4,-14.5,-14.6,-14.7,-14.8
  33,-14  ,-14.1,-14.2,-14.3,-14.4,-14.5,-14.6,-14.7,-14.8
  33,-14.5,-14.6,-14.7,-14.8,-14.9,-15  ,-15.1,-15.2,-15.3
  33,-14.7,-14.8,-14.9,-15  ,-15.1,-15.2,-15.3,-15.4,-15.5
  33,-15  ,-15.1,-15.2,-15.3,-15.4,-15.5,-15.6,-15.7,-15.8
  33,-15.2,-15.3,-15.4,-15.5,-15.6,-15.7,-15.8,-15.9,-16
  33,-15.2,-15.3,-15.4,-15.5,-15.6,-15.7,-15.8,-15.9,-16

In this case, sedflux will use a 10 by 10 model grid. The spacing of the rows
is the x-resolution and the spacing of the columns is the y-resolution.  If
this grid were put into sedflux's coordinate system, the origin would be in
the upper left corner with the positive x-axis pointing down and the positive
y-axis pointing to the right.  This is of particular importance when
:ref:`setting avulsion parameters <sedflux_module_avulsion>`.
