.. _sedflux_module_bed_load:

Bed Load Dumping
================

The bed load dumping module deposits bedload from a river to the ocean.  In
2D mode, the bed load is deposited evenly over a fixed length, while in 3D it
is deposited over a cone of fixed size.  The bed load dumping module is
configured within the sedflux
:ref:`process description file <sedflux_process_file>` as a group named
``bedload dumping``.  Following is an example
:ref:`key-value group <sedflux_key_value_file>` that defines the bed load
dumping module.

.. code-block:: ini

  [ 'bedload dumping' ]
  active:                                 yes
  logging:                                no
  repeat interval:                        always
  distance to dump bedload (m):           5000
  ratio of flood plain to bedload rate:   0.
  fraction of bedload retained in the delta plain: 0.
  river name:                             po

The first three parameters are general module parameters that must be
specified for every group within a :ref:`process file <sedflux_process_file>`.
The remaining parameters are specific to the bed load dumping module:

 * ``distance to dump bedload``: Distance over which bed load is deposited.
   In 3D mode this is the radius of the cone. 
 * ``ratio of flood plain to bedload rate``: Specify flood plain deposition
   rate as a fraction of marine deposition rate.  A value of 0 indicates no
   subaerial deposition, while 1 is equal deposition rates.
 * ``fraction of bedload retained in the delta plain``: Specify the fraction
   of the total bed load entering the system that is retained by the delta
   plain.  A value of 0 means everything reaches the ocean, while 1 means
   nothing makes it to the ocean. 
 * ``river name``: Name of the river to associate this process with.
