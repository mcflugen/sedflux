.. _sedflux_river_file:

Description of a sedflux river file
===================================

The river file specifies river characteristics of the river as it enters the
model domain. It can come in two formats: :ref:`key-value file
<sedflux_key_value_file>` (ASCII), or Hydrotrend (binary). Regardless of
format, the river format specifies river width, depth, velocity, and
concentrations of each of the grain types. In both cases, if sedflux runs
through all of the periods of your river file, it will return to the beginning.

Key-value File
--------------

The user can specify river characteristics key-value pairs for an arbitrary
number of time periods. Unlike the Hydrotrend file, these time periods need not
be the same length. The following example defines two periods:

.. code-block:: ini

  [ 'Season 1' ]
  Duration (y):                            3m
  Bedload (kg/s):                          300.0
  Suspended load concentration (kg/m^3):   6,      3,      3,      3
  velocity (m/s):                          1.0
  Width (m):                               100.0
  Depth (m):                               5.0
  [ 'Season 2' ]
  Duration (y):                            9m
  Bedload (kg/s):                          30.0
  Suspended load concentration (kg/m^3):   .6,      .3,      .3,      .3
  velocity (m/s):                          1.0
  Width (m):                               10.0
  Depth (m):                               .5

The first period lasts for 3 months, and the second for 9 months (the periods
do not need to sum to 1 year - they can be whatever length you like).

There are a couple of things to note about this file:
 * The above river file describes five grain types - one bedload type and four
   suspended load types.  It is necessary that the number of grain types given
   here are the same number described in the
   :ref:`sediment file <sedflux_sediment_file>` In addition, the order that
   they are listed here (starting with bedload) corresponds to the order that
   they are listed in the :ref:`sediment file <sedflux_sediment_file>`.
 * The durations that you specify in this file will override the time step
   that you provided in the :ref:`initialization file <sedflux_init_file>`.
   Thus, say you provided a time step of 1 day in the initialization file. If
   using the above river file, this time step would be ignored and would
   instead alternate between 3 months and 9 months. 

Hydrotrend File
---------------

The Hydrotrend model can output daily, monthly, or yearly river data. See the
Hydrotrend web page for a detailed description of the format. One thing to
note though is that it is binary and will be of the byte-order of the machine
that it was created on. It has been a common problem for users to use a
Hydrotrend file created on one machine but then run sedflux with this file on
a machine with the opposite byte-order. sedflux should now be able to notice
this and give an appropriate error message.
