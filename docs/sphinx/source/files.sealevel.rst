.. _sedflux_sea_level_file:

Description of a sedflux sea-level file
=======================================

A sedflux sea-level file consists of two comma-separated columns that specify
sea-level with time. The first column is time (in years), and the second is
elevation (in meters). sedflux uses linear interpolation to find sea-level
between points. A simple sea-level file looks like this::

  0   , 0
  3000, -10
  6000, 5

Here sea level falls 10 meters over 3000 years, and then rises 15 meters over
the next 3000 years.  The name of the sea-level file that sedflux uses is
configured in the :ref:`process description file <sedflux_process_file>` as the
:ref:`sea-level module <sedflux_module_sea_level>`.

Note that time is measured from the start of the sedflux simulation (not, say,
years before present).  For example, if you are using a sea-level curve for
the last 20ky, time 0 would correspond to 20ky ago and time 20ky to present
day.
