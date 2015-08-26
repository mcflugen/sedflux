.. _sedflux_module_subsidence:

Subsidence
==========

The sedflux subsidence module allows the user to specify subsidence rates for
a model simulation.  This should not to be confused with the
:ref:`isostasy module <sedflux_module_isostasy>`, which predicts subsidence
rates due to loading.  Subsidence is configured in a
:ref:`sedflux process file <sedflux_process_file>` with the group name
``subsidence``.  Following is an example
:ref:`key-value group <sedflux_key_value_file>` that defines the subsidence
module.

.. code-block:: ini

  [ subsidence ]
  active:                                 yes
  logging:                                yes
  repeat interval:                        always
  subsidence file:                        test.subsidence

The first three parameters are general module parameters that must be
specified for every group within a :ref:`process file <sedflux_process_file>`.
The remaining parameter is specific to the subsidence module:

 * ``subsidence file``: Path to the file that contains the subsidence data

The :ref:`format of the sedflux subsidence file <sedflux_subsidence_file>`
depends on whether sedflux is run in 2D or 3D mode.  In either case this file
defines subsidence rates that are able to vary in both time and space.
