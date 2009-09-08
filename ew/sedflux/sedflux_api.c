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

#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_sedflux.h"
#include "sedflux_api.h"

struct Sedflux_state
{
  Sed_epoch_queue q;
  Sed_cube p;
  char* init_file;
};

gboolean
sedflux (const gchar* init_file, int dimen)
{
  Sedflux_state* state = sedflux_initialize (init_file, dimen);

  eh_require (state)

  sedflux_run (state);
  sedflux_finalize (state);

  return TRUE;
}

Sedflux_state*
sedflux_initialize (const gchar* file, int dimen)
{
  Sedflux_state* state = NULL;

  { /* Create a new state. */
    state = eh_new (Sedflux_state,1);

    state->q = NULL;
    state->p = NULL;
    state->init_file = g_strdup (file);
  }

  switch (dimen)
  {
    case 2:
      sed_mode_set (SEDFLUX_MODE_2D);
      break;
    case 3:
      sed_mode_set (SEDFLUX_MODE_3D);
      break;
    default:
      eh_require_not_reached ();
  }

  eh_require (sed_mode_is_2d() || sed_mode_is_3d());

  { /* Scan the init file. */
    GError* error = NULL;

    state->p = sed_cube_new_from_file( state->init_file , &error );
    eh_exit_on_error( error , "%s: Error reading initialization file" ,
                              state->init_file );

    state->q = sed_epoch_queue_new_full( state->init_file , my_proc_defs ,
                                         my_proc_family , NULL , &error );
    eh_exit_on_error( error , "%s: Error reading epoch file" ,
                               state->init_file );
  }

  return state;
}

void
sedflux_run_time_step (Sedflux_state* state)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_tic( state->q , state->p );

  return;
}

void
sedflux_run (Sedflux_state* state)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_run (state->q, state->p);

  return;
}

void
sedflux_run_until (Sedflux_state* state, double then)
{
  eh_require( state );
  eh_require( state->q );
  eh_require( state->p );

  sed_epoch_queue_run_until( state->q , state->p , then );
  return;
}

double*
sedflux_get_value (Sedflux_state* state, const char* val_s, int dimen[3])
{
  double* data = NULL;

  eh_return_val_if_fail (state, NULL);
  eh_return_val_if_fail (val_s, NULL);

  {
    Sed_measurement m = sed_measurement_new (val_s);

    eh_require (m);

    { /* Get values at all grid cells */
      gint64 id;
      Eh_ind_2 sub;
      const len = sed_cube_size (state->p);

      data = eh_new (double, sed_cube_size (state->p));
      for (id=0; id<len; id++)
      {
        sub = sed_cube_sub (state->p, id);
        data[id] = sed_measurement_make (m, state->p, sub.i, sub.j);
      }

      dimen[0] = 1;
      dimen[1] = sed_cube_n_x (state->p);
      dimen[2] = sed_cube_n_y (state->p);
    }

    sed_measurement_destroy( m );
   }

   return data;
}

double*
sedflux_get_value_cube (Sedflux_state* state, const char* val_s, int dimen[3])
{
  double* data = NULL;

  eh_return_val_if_fail (state, NULL);
  eh_return_val_if_fail (val_s, NULL);

  {
    Sed_property p = sed_property_new (val_s);

    eh_require (p);

    { /* Get values at all grid cells */
      Eh_ndgrid g;

      g = sed_cube_property_subgrid (state->p, p, NULL, NULL, NULL);
      data = eh_ndgrid_start (g);

      dimen[0] = eh_ndgrid_n (g,0);
      dimen[1] = eh_ndgrid_n (g,1);
      dimen[2] = eh_ndgrid_n (g,2);

      eh_ndgrid_destroy (g,FALSE);
    }

    sed_property_destroy (p);
   }

   return data;
}

void
sedflux_set_basement (Sedflux_state* state, const double* val)
{
  eh_require (state);
  eh_require (state->p);
  eh_require (val);

  {
    const gint len = sed_cube_size (state->p);
    gint i;

    for (i=0; i<len; i++)
      sed_column_set_base_height (sed_cube_col (state->p, i), val[i]);
  }

  return;
}

double
sedflux_get_start_time (Sedflux_state* state)
{
  eh_require (state);

  Sed_epoch e = sed_epoch_queue_first (state->q);
  return sed_epoch_start (e);
}

double
sedflux_get_end_time (Sedflux_state* state)
{
  eh_require (state);

  Sed_epoch e = sed_epoch_queue_last (state->q);
  return sed_epoch_end (e);
}

int
sedflux_get_nx (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_n_x (state->p);
}

int
sedflux_get_ny (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_n_y (state->p);
}

double
sedflux_get_xres (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_x_res (state->p);
}

double
sedflux_get_yres (Sedflux_state* state)
{
  eh_require (state);
  return sed_cube_y_res (state->p);
}

void
sedflux_finalize (Sedflux_state* state)
{
  if (state)
  {
    sed_epoch_queue_destroy (state->q);

    sed_cube_destroy (state->p);

    sed_sediment_unset_env ();
  }

  return;
}

