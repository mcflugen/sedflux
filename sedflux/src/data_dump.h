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

#ifndef _DATA_DUMP_H_
# define _DATA_DUMP_H_

# include "utils.h"
# include "sed_sedflux.h"

#define GRAIN_SIZE_TOKEN        "grain"
#define PLASTIC_INDEX_TOKEN     "pi"
#define BULK_DENSITY_TOKEN      "density"
#define AGE_TOKEN               "age"
#define FACIES_TOKEN            "facies"
#define VELOCITY_TOKEN          "velocity"
#define VOID_RATIO_TOKEN        "void ratio"
#define VOID_RATIO_MIN_TOKEN    "void ratio min"
#define VISCOSITY_TOKEN         "viscosity"
#define FRICTION_ANGLE_TOKEN    "friction angle"
#define PERMEABILITY_TOKEN      "permeability"
#define CONDUCTIVITY_TOKEN      "conductivity"
#define POROSITY_TOKEN          "porosity"
#define RELATIVE_DENSITY_TOKEN  "relative density"
#define MV_TOKEN                "mv"
#define CV_TOKEN                "cv"
#define SHEAR_STRENGTH_TOKEN    "shear"
#define COHESION_TOKEN          "cohesion"
#define EXCESS_PRESSURE_TOKEN   "excess pressure"
#define RELATIVE_PRESSURE_TOKEN "relative pressure"
#define CONSOLIDATION_TOKEN     "consolidation"
#define SAND_TOKEN              "sand"
#define SILT_TOKEN              "silt"
#define CLAY_TOKEN              "clay"
#define MUD_TOKEN               "mud"
#define FRACTION_TOKEN          "fraction"

typedef struct
{
   gboolean initialized;

   double vertical_resolution;
   double horizontal_resolution;
   double y_lim_min, y_lim_max;
   double x_lim_min, x_lim_max;
   char *output_dir;
   GArray *property;
   int count;
}
Data_dump_t;

Sed_process_info run_data_dump(gpointer,Sed_cube);
gboolean init_data_dump(Eh_symbol_table,gpointer);

#endif

