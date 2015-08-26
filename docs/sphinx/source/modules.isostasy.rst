.. _sedflux_module_isostasy:

Isostasy
========

The isostasy module simulates crustal deflection due to sediment and water
loading.  This should not be confused with the
:ref:`subsidence module <sedflux_module_subsidence>`, which subsides a basin
based on user-specified subsidence rates.  The subsidence module is configured
within the sedflux :ref:`process description file <sedflux_process_file>` as
a group named ``isostasy``.  Following is an example
:ref:`key-value group <sedflux_key_value_file>` that defines the isostasy
module.

.. code-block:: ini

  [ isostasy ]
  active:                                 yes
  logging:                                no
  repeat interval:                        100y
  effective elastic thickness (m):        65000
  Youngs modulus (Pa):                    7e10
  relaxation time (years):                2500

The first three parameters are general module parameters that must be
specified for every group within a :ref:`process file <sedflux_process_file>`.
The remaining parameters are specific to the subsidence module:

 * ``Effective elastic thickness``: The portion of the lithosphere that
   behaves elastically on geological time scales .
 * ``Youngs modulus``:
   `Young's Modulus <http://en.wikipedia.org/wiki/Young%27s_modulus>`_ of the
   crust and mantle lithosphere (typically around :math:`7x10^{10}`
   :math:`N/m^2`)
 * ``Relaxation time``: Time for the crust to reach :math:`1/e` of its
   equilibrium deflection.
