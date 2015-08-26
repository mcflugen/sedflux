.. _sedflux_key_value_file:

The sedflux key-value file
==========================

Many of the (ASCII) input files for sedflux are of a common format. I'll
often refer to them as sedflux key-files. They are plain text files that
define a series of groups. Each group contains a series of key-value pairs.
A group begins with its name surrounded by square brackets and ends at the
begining of the next group.

The key-value pairs follow, one per line. They consist of a parameter name
(or key), followed by a colon and then the value. As an example, a valid
group looks like,

.. code-block:: ini

  [ global ]
  vertical resolution (m):   0.05
  x resolution (m):          10000
  y resolution (m):          100

In this example, we've defined a group called global. For this group, we've
set the values for three parameters. There can be any number of groups or
key-value pairs per file. In fact, the same group name can appear in the
same file more than once. The data are not overwritten but a new instance
of the group created. However, in some instances it may not make sense to
have multiple occurances of a group.

Examples of sedflux input files that are key-value files:
 * :ref:`Sediment description file <sedflux_sediment_file>`
 * :ref:`River conditions <sedflux_river_file>`
 * :ref:`Initialization file <sedflux_init_file>`
