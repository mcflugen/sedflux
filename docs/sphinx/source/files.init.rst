.. _sedflux_init_file:

The sedflux Initialization File
===============================

A sedflux initialization file is divided into two parts. The first part defines
some global parameters while the second divides the model run into one or more
"epochs". Each epoch defines a time step, duration, and process environment.

Global Parameters
-----------------

The following is an example of a group that defines the global parameters for a sedflux simulation.

.. code-block:: ini

  [ global ]
  margin name:            poverty_bay
  vertical resolution:    0.05
  x resolution:           30000
  y resolution:           100
  bathymetry file:        poverty_bay_bathy.csv
  sediment file:          poverty_bay_sediment.kvf

Each of these parameter/value pairs must be specified whether run in 2D or 3D
mode. A brief description of the parameters:

 * ``margin name``: The name of the sedflux simulation. This name will appear
   in the name of sedflux output files
 * ``vertical resolution``: The vertical resolution (in meters) of the model
   run. Sediment deposits will be averaged over this amount.
 * ``x resolution``: Resolution of the model run in the x-direction. For a 2D
   simulation this will be the along shore width over which sediment deposited. For 3D simulations, this refers to the spacing of rows in the bathymetry file.
 * ``y resolution``: Resolution of the model run in the y-direction. For a 2D
   simulation this will be the resolution of the simulation in the cross-shore.
   For 3D simulations, this refers to the spacing of columns in the bathymetry
   file.
 * :ref:`bathymetry file <sedflux_bathy_file>`: The name of the file that
   specifies the initial bathymetry of the simulation.
 * :ref:`sediment file <sedflux_sediment_file>`: The name of the file that
   defines the type of sediment that is used by the model.

Epoch Parameters
----------------

The following is an example of a series of groups that define epoch parameters
for a simulation.

.. code-block:: ini

  [ epoch ]
  number:           1
  duration:         490y
  time step:        1d
  process file:     poverty_bay1.epoch

  [ epoch ]
  number:           2
  duration:         60y
  time step:        1d
  process file:     poverty_bay2.epoch

  [ epoch ]
  number:           3
  duration:         300y
  time step:        1d
  process file:     poverty_bay3.epoch

Here three epochs are specified. Each has its own duration and process file.
Thus, in this example, the total simulation models 850 years. A brief
description of the parameters,

 * ``number``: Number that specifies the order that sedflux will run the
   epochs. sedflux runs epochs in ascending order.
 * ``duration``: The length of the epoch (``y`` for years, ``m`` for months,
   and ``d`` for days).
 * ``time step``: The time step of the epoch (``y`` for years, ``m`` for
   months, and ``d`` for days). **IMPORTANT**: Note that this parameter may be
   ignored (see NOTE at the bottom of this page).
 * :ref:`process file <sedflux_process_file>`: The name of the file that
   defines the processes for the epoch.

.. note::

  If within the epoch process file the river values are specified as
  'season', the time step given in that file will override that specified in the
  initialization file.
