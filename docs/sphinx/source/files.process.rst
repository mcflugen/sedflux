.. _sedflux_process_file:

sedflux Process Description File
================================

Each epoch of a sedflux :ref:`initialization file <sedflux_init_file>` gives
the name of a process description file.  This description file is a
:ref:`key-value file <sedflux_key_value_file>` that describes the various
:ref:`process modules <sedflux_processes>` you wish to run within your
sedflux simulation.  Each process is defined within a key-value group named for
that process.  Every group begins with key-value pairs that are common for all
sedflux processes and then follows with process-specific parameters.  An
example of a process group is::

  [ isostasy ]
  active:                                 yes
  logging:                                yes
  repeat interval:                        100y
  effective elastic thickness (m):        65000
  Youngs modulus (Pa):                    7e10
  relaxation time (years):                2500

In this case we describe the constants for the process, *isostasy*.  The first
three parameters must be specified for all sedflux process groups.  The meaning
of these parameters are:

 * ``active``: Boolean that indicates if sedflux should run this process or not
   (``yes`` or ``no``)
 * ``logging``: Boolean that indicates if sedflux should create a log file for
   this process (``yes`` or ``no``).
 * ``repeat interval``: Duration that sedflux should wait between invocations
   of this process (either the keyword, ``always`` or model time)

The subsequent key-value pairs describe parameters that are specific to this
process.
