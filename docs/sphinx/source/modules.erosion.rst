.. _sedflux_module_erosion:

Erosion
=======

The erosion module erodes subaerial sediments on the delta plain.  In 2D
mode, these are sediments that are landward of the current river mouth.
In 3D mode, these are sediment within the river as defined as the line
joining the hinge point and the river mouth.  Erosion rates are calculated
by one of two methods.  This first erodes all sediment above a user-specified
line drawn landward of the river mouth.  The second method erodes sediment
as a diffusive process.  In either case, eroded sediment is added to the
river.  The erosion module is configured within the sedflux
:ref:`process description file <sedflux_process_file>` as a group named
``erosion``.  Following is an example
:ref:`key-value group <sedflux_key_value_file>` that defines the erosion
module:

.. code:: ini

  [ erosion ]
  active:                                 yes
  logging:                                no
  repeat interval:                        always
  reach of highest order stream (m):      10000
  relief of highest order stream (m):     1
  method (diffusion|slope):               diffusion

The first three parameters are general module parameters that must be
specified for every group within a :ref:`process file <sedflux_process_file>`.
The remaining parameters are specific to the bed load dumping module:

 * ``reach of highest order stream``: Denominator in gradient calculation of
   delta slope. 
 * ``relief of highest order stream``: Numerator in gradient calculation of
   delta slope.
 * ``method``: Specify the method used to erode sediment.  This can be either
   ``slope``, of ``diffusion``.
