.. _sedflux_command_line:

Run From the Command Line
=========================

If you have set up the sedflux working directory with the necessary
:ref:`input files <sedflux_input_files>`, you are now ready to run it.  If your
sedflux working directory is ``<project-dir>``, you can execute sedflux from
the command line with something like the following, 

To run sedflux from the command line,

.. code-block:: bash

  > cd <project-dir>
  > sedflux --init-file=<init-file>

where ``<init-file>`` is the name of your initialization file.  By default,
sedflux will run in 2D mode. To run sedflux in 3D mode, use the ``-3`` option,

.. code-block:: bash

  > sedflux -3 --init-file=<init-file>

To get a brief description of the available options (there aren't many), use
the ``--help`` option,

.. code-block:: bash

  > sedflux --help

If you are running sedflux on a cluster, you will most likely have to submit
your sedflux job to a queuing system.  Because sedflux is written to run in
serial, creating a
:ref:`submission script that will run sedflux <sedflux_run_pbs>` is fairly
straight forward.
