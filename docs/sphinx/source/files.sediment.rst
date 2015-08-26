.. _sedflux_sediment_file:

The sedflux sediment file
=========================

The characteristics of the sediment used by a sedflux simulation is described
within a :ref:`key-value file <sedflux_key_value_file>`.  The characteristics
of a sediment type is defined through key-value pairs within a single group.
Sediment types are listed one after another. The order that they occur here is
important. sedflux assumes that the order they are listed in this file
corresponds to the same order they are listed in the river input files.  An
example of one record that describes a singe sediment type is::

  [ 'Grain 1 (bedload)' ]
  grain size (microns):                200
  grain density (kg/m^3):              2625
  saturated density (kg/m^3):          1850
  minimum void ratio (-):              .30
  diffusion coefficient (-):           .25
  removal rate (1/day):                50
  consolidation coefficient (m^2/yr):  100000
  compaction coefficient (-):          0.0000000368

For a sediment file, the required parameters are:
 * ``grain size``: `Grain size <http://en.wikipedia.org/wiki/Grain_size>`_ (in
   microns) of the sediment type.
 * ``grain density``: `Particle Density <http://en.wikipedia.org/wiki/Particle_density>`_
   (in kg/m^3^) of the sediment grains.
 * ``saturated density``: `Bulk density <http://en.wikipedia.org/wiki/Bulk_density>`_
   (in kg/m^3^) with voids completely filled with water.
 * ``minimum void ratio``: `Void ratio <http://en.wikipedia.org/wiki/Void_ratio>`_
   (-) of the sediment in its closest packed state (void ratio is closely related to `porosity <http://en.wikipedia.org/wiki/Porosity>`_).
 * ``diffusion coefficient``: Diffusion index between 0 (unable to be moved)
   and 1 (easily moved).
 * ``removal rate``: Removal rate constant (in 1/day)
   ([#syvitski_1988 Syvitski et al., 1988]).  This is closely related to
   particle settling velocity as described in [#bursik_1995 Bursik, 1995].
 * ``consolidation coefficient``: How quickly the sediment consolidates.
 * ``compaction coefficient``: Rate that sediment is compacted under load
   ([#bahr_2001 Bahr et al., 2001]).

.. note::

  The first grain type listed in the sediment file is assumed by sedflux to be
  bedload! That is, it is not carried by the [SedfluxModulePlume plume process]
  but by the [SedfluxModuleBedLoadDumping bed-load dumping process].

Example Sediment Files
----------------------

An example of a typical sediment file:
 * :download:`sediment.kvf`: 5 sediment types.

References
----------

 1. [=#syvitski_1988]Syvitski, J. P. M., Smith, J. N., Calabrese, E. A. and Boudreau, B. P., 1988. Basin sedimentation and the growth of prograding deltas. Journal of Geophysical Research 93 C6, pp. 6895–6908.
 2. [=#bursik_1995]Bursik, M., 1995. Theory of the sedimentation of suspended particles from fluvial plumes. Sedimentology 42 6, pp. 831–838.
 3. [=#bahr_2001]David B. Bahr, Eric W.H. Hutton, James P.M. Syvitski, Lincoln F. Pratson, Exponential approximations to compacted sediment porosity profiles, Computers & Geosciences, Volume 27, Issue 6, Numerical Models of Marine Sediment Transport and Deposition, July 2001, Pages 691-700, ISSN 0098-3004, DOI: 10.1016/S0098-3004(00)00140-0.
