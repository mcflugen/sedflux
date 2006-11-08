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

#if !defined(TURBIDITY_CURRENT_H)
# define TURBIDITY_CURRENT_H

# include "utils.h"
# include "sed_sedflux.h"

typedef struct
{
   gboolean initialized;
   double sua;
   double sub;
   double E_a;
   double E_b;
   double C_d;
   double tan_phi;
   double mu;
   double rhoSW;
   double channel_width;
   double channel_length;
   Sed_cube failure;
   int algorithm;

   guint n_x;
   guint n_y;
   double **deposit;
}
Turbidity_t;

typedef struct
{
   double dx;
   double x;
   double erode_depth;
   double *phe;
}
Sed_phe_query_t;

typedef struct
{
   double dh;
   int i;
}
Sed_remove_query_t;

typedef struct
{
   int i;
   double dh;
   double *phe;
   int n_grains;
}
Sed_add_query_t;

typedef struct
{
   int i;
   double depth;
}
Sed_depth_query_t;

Sed_process_info run_turbidity(gpointer,Sed_cube);
gboolean init_turbidity(Eh_symbol_table,gpointer);

#endif
