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

#if !defined(PROCESSES_H)
# define PROCESSES_H

# include "utils.h"
# include "sed_sedflux.h"

# include "run_constants.h"
# include "cpr.h"
# include "bbl.h"
# include "bedload.h"
# include "compaction.h"
# include "run_flow.h"
# include "data_dump.h"
# include "debris_flow.h"
# include "run_squall.h"
# include "run_diffusion.h"
# include "run_xshore.h"
# include "erosion.h"
# include "failure_proc.h"
# include "quake.h"
# include "tide.h"
# include "isostasy.h"
# include "measuring_station.h"
# include "run_plume.h"
# include "river.h"

# include "run_new_process.h"

# include "sealevel.h"
# include "storms.h"
# include "subsidence.h"
# include "turbidity_current.h"
# include "avulsion.h"

/** Holds all of the processes for sedflux

Each process has two entries in the structure.  The first, a GList is a
list of instances of a process.  sedflux will run each of the processes
in the list every time step.

The second entry is a Sed_process that describes the process.

*/
/*
typedef struct
{
   GSList *constants_l;    Sed_process constants;
   GSList *quake_l;        Sed_process quake;
   GSList *tide_l;         Sed_process tide;
   GSList *sea_level_l;    Sed_process sea_level;
   GSList *storm_l;        Sed_process storm;
   GSList *erosion_l;      Sed_process erosion;
   GSList *avulsion_l;     Sed_process avulsion;
   GSList *river_l;        Sed_process river;

   GSList *new_process_l;  Sed_process new_process;

   GSList *turbidity_l;    Sed_process turbidity;
   GSList *debris_flow_l;  Sed_process debris_flow;
   GSList *slump_l;        Sed_process slump;
   GSList *plume_l;        Sed_process plume;
   GSList *bedload_l;      Sed_process bedload;
   GSList *diffusion_l;    Sed_process diffusion;
   GSList *xshore_l;       Sed_process xshore;
   GSList *squall_l;       Sed_process squall;
   GSList *failure_l;      Sed_process failure;
   GSList *compaction_l;   Sed_process compaction;
   GSList *flow_l;         Sed_process flow;
   GSList *isostasy_l;     Sed_process isostasy;
   GSList *subsidence_l;   Sed_process subsidence;
   GSList *bbl_l;          Sed_process bbl;
   GSList *met_station_l;  Sed_process met_station;
   GSList *data_dump_l;    Sed_process data_dump;
   GSList *final_dump_l;   Sed_process final_dump;
   GSList *cpr_l;          Sed_process cpr;
}
Sed_process_list;
*/

#endif
