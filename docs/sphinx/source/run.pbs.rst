.. _sedflux_run_pbs:

Running sedflux with PBS/Torque
===============================

If you are running sedflux on a cluster, you will most likely have to submit
your sedflux simulation through a queuing system.  This page describes how
this might be done with PBS/Torque.    The first step is to create a
submission script that will run sedflux.  An
:download:`example <run_sedflux.sh>` of such a script is:

.. literalinclude:: run_sedflux.sh

This script will run sedflux in 3D mode in ``$RUNDIR`` with the initialization
file ``$INIT_FILE``.  The options to sedflux are the same as if
:ref:`running from the command line <sedflux_command_line>`.  Notice that
you must supply a run description message on the command line (the --msg
option).  This is because the script will not be run interactively; without
this option, sedflux will prompt you to provide a description.  To submit
this job to the queue, use ``qsub``.

.. code-block:: bash

  > qsub run_sedflux.sh

In this case we've named the above script, ``run_sedflux.sh``.  The exact
details of how this is done will depend on your particular system.  There are
lots of good resources on the internet that describe queuing systems.  For
example:

 * `Complete Torque guide <http://www.clusterresources.com/torquedocs21/?id=torque:appendix:l_torque_quickstart_guid>`_
 * `CSDMS cluster <http://csdms.colorado.edu/wiki/Help:HPCC_Torque>`_
   (where I run sedflux mostly)
