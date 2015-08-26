.. _sedflux_input_files:

Sedflux Input Files
===================
To run sedflux you will need to create a project directory where your input
files will be stored and output files can be written to. You will need at least
four input files; depending on the processes you plan to use, you may need
more.

The main input files are:
 * :ref:`Initialization file <sedflux_init_file>`
 * :ref:`Bathymetry file <sedflux_bathy_file>`
 * :ref:`Sediment description file <sedflux_sediment_file>`
 * :ref:`Process definition file <sedflux_process_file>`

The above are all required files. Files that are optionally included are:
 * :ref:`River file <sedflux_river_file>`
 * :ref:`Sea-level file <sedflux_sea_level_file>`
 * :ref:`Subsidence file<sedflux_subsidence_file>`

Most of these files, are in the :ref:`sedflux key/value <sedflux_key_value_file>`
file format.

.. toctree::
  :hidden:
  :maxdepth: 2

  files.keyvalue.rst
  files.bathymetry
  files.init.rst
  files.process.rst
  files.river.rst
  files.sealevel.rst
  files.sediment.rst
  files.subsidence.rst
