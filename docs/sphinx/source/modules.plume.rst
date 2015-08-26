.. _sedflux_module_plume:

Plume
=====

If you have successfully [wiki:SedfluxDevelInstall compiled and installed sedflux]
you will have also installed the program, plume.  You can find this in you
sedflux installation directory under ``/bin``.  Thus, if you've installed
sedflux under ``/usr/local``, you can find the plume program at
``/usr/local/bin/plume``.  The following descriptions assume plume is in you
path.

To run the plume model you will need two input files.  I've create an example
of each to get you started:

 * :download:`plume_config.kvf`: describes some physical and numerical
   constants used by the model.
 * :download:`plume_flood.kvf`: describes input conditions to the model.

To run plume with the above input files, download them to some directory on
your machine and execute plume as follows,

.. code-block:: bash

  > plume --in-file=plume_config.kvf --flood-file=plume_flood.kvf --data-file=output

This will create a series of output files of the form ``output-#.csv`` where
``#`` will be a number from 0 to the number of grain types that you are
modeling minus one.  In this example there are 4 grain types.  The output
files contain comma-separated data that you can easily view in MATLAB or a
similar program.

View Output with MATLAB
-----------------------

The plume program writes output files as a text file of comma-separated
values.  If you are using MATLAB, the easiest way to read a plume output
files is with the ``dlmread`` function.  For instance, if your plume output
files is called ``output-0.csv``,

.. code-block:: matlab

  >> dz = dlmread ('output-0.csv');

Values are deposition rates for this grain size (in m/day).  Since these
values usually fall off exponentially from the river mouth, oftentimes it
is best to display them as log values,

.. code-block:: matlab

  >> imagesc (real (log (dz))

Note that land values are given a value of -1.  Because of this, it is
necessary to take the real part of the log.
