.. _sedflux_module_sea_level:

Sea Level
=========

The sea-level module changes base level with time for a sedflux simulation.
sedflux reads a [wiki:SedfluxFileSeaLevel sea-level file] that gives a time
series of sea level over the period of the simulation.  The sea-level module
is configured within the sedflux :ref:`process description file <sedflux_process_file>`
as a group named *sea level*.  Following is an example
[wiki:SedfluxKeyValueFile key-value group] that defines the sea-level module::

  [ 'sea level' ]
  active:                                 yes
  logging:                                no
  repeat interval:                        always
  sea level file:                         sea_level.csv

The first three parameters are general module parameters that must be
specified for every group within a :ref:`process description file <sedflux_process_file>`.
The remaining parameter is specific to the sea-level module:

 * ``sea level file``: Name of the file that contains the sea-level time series
