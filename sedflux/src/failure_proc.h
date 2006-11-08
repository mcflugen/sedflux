//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#if !defined(FAILURE_PROC_H)
#define FAILURE_PROC_H

#include "utils.h"
#include "sed_sedflux.h"

#include "failure.h"
#include "debris_flow.h"
#include "turbidity_current.h"
#include "flow.h"
#include "slump.h"

typedef struct
{
   gboolean initialized;

   double decider_clay_fraction;
   double consolidation;
   double cohesion;
   double friction_angle;
   double gravity;
   double density_sea_water;
   Sed_process turbidity_current;
   Sed_process debris_flow;
   Sed_process slump;
   Sed_process flow;

   Fail_profile *fail_prof;
}
Failure_proc_t;

Sed_process_info run_failure(gpointer,Sed_cube);
gboolean init_failure(Eh_symbol_table,gpointer);

#endif
