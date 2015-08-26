.. _sedflux_subsidence_file:

Description of a sedflux subsidence file
========================================

The format for sedflux subsidence files is different for the 2D and 3D
versions of sedflux. In 2D mode, the subsidence file is a series of records
that each contain a time value, and comma delimated data that define the
subsidence curve. In 3D mode, the subsidence file is also a series of records
that defines a time-varying subsidence curve but is binary.

2D subsidence file
------------------

The subsidence file in the following example contains two records that define
the subsidence of the profile at 0 and 100 years. Each record begins with a
header contained in square brackets followed by two columns of data that
define how the profile subsides along its length. The time is given in years,
the position (first column) in meters, and the subsidence rate (second column)
in meters per year. For times between records, sedflux interpolates linearly
between records.

.. code-block:: ini

  [ Time: 0 ]    // Time of this record in years
  0, 0           // Subsidence is zero at the origin
  100000, -.2    //   and increases to -.2 m/y at 100km

  [ Time: 100 ]  // The second record is at 100 years
  0,0
  100000, -.1

If the file contains only one record, then the subsidence rate is held
constant starting from the time indicated in the record header. If the model
runs longer than the time of the last record, the subsidence curve is set to
that of the last record for the remainder of the simulation.

3D subsidence file
------------------

The subsidence file for 3D sedflux, contains a series of binary records
containing similar information as that of the 2D file described above.

The file begins with two 32-bit integers that define the size of the
subsidence grid for each record. The first value is the number of rows and
the second the number of columns.

Records are then listed one after another. Each record begins with a double
value that gives the time in years of the record. The subsidence data then
follows as double values written row by row. Subsidence values are given in
meters per year.

.. note::

  The size of the grid must match exactly that of the grid that is defined in
  the bathymetery file. 

.. note::

  The byte order of the file must be the same as that of the machine that the
  simulation will be run on.

Create a subsidence file for sedflux 3D
+++++++++++++++++++++++++++++++++++++++

The dimensions of the subsidence grid must be the same as the bathymetric
grid. Using MATLAB, you can see what the dimension are of your bathymetric
grid are with the following commands:

.. code-block:: matlab

  >> bathy = dlmread (‘test-bathy.csv’);
  >> [n_rows, n_cols] = size (bathy);

In this case ``n_rows`` and ``n_cols`` will contain the number of rows and
columns of the grid described by your bathymetry file (for this particular
case, ``n_row=32``, and ``n_cols=100``).

In the following example we use MATLAB to create a subsidence file called
subsidence.bin.  The file will describe a scenario where the entire grid is
initially subsiding at .005 m/yr but reduces to .0025 m/yr at 1000 years.

 #. Open the new subsidence file, ``subsidence.bin``.  This
    file will contain your subsidence grids.  On Mac and Linux systems you will
    need the binary byte order to be ``little endian``. Byte orders are machine
    dependent, so it is common that you run into problems if you are using
    the wrong byte order.

    .. code-block:: matlab

      >> fid = fopen ('subsidence.bin', 'w', 'ieee-le');

 #. Specify the dimensions of the grid.  In this case the number
    of rows is 32 and the number of columns is 100.  They must be written as
    32 bit integers.

    .. code-block:: matlab

      >> fwrite (fid, [32, 100], 'int32')

 #. Write the time for which your first input grid is specified
    (in years), generally this would be time=0.

    .. code-block:: matlab

      >> fwrite (fid, 0, 'double')

 #. Create a matrix of subsidence values (in m/yr).  For
    subsidence (rather than uplift) these value should be negative. The
    example shows spatially uniform uplift, but the grid could be spatially
    variable as well.

    .. code-block::  matlab

      >> s0 = ones (32, 100) * 0.005;

 #. Write the matrix to the subsidence file. Note that the matrix
    ``s`` is transposed (by using ``s’`` in the MATLAB statement) to get the
    rows and columns in the correct x and y direction for sedflux.

    .. code-block:: matlab

      >> fwrite (fid, s0', 'double')

 #. Define the time (in years) when you want to update your
    subsidence or uplift rates. This can be any time after the previous grid.
    sedflux will linearly interpolate between the two grids for the time steps
    in between. In our example the uplift rate decreases over time.

    .. code-block:: matlab

      >> fwrite (fid, 1000, 'double');
      >> s1000 = ones (32,100) * 0.0025;
      >> fwrite (fid, s1000', 'double');

    Repeat steps 3 through 6 as many time steps as you like.  Subsidence or
    uplift rates should ideally be specified until the last time step of the
    simulation to avoid ambiguity.

 #. Close the file ``subsidence.bin``. The file is now ready to be read in
    sedflux 3D.

    .. code-block:: matlab

      > fclose (fid)
