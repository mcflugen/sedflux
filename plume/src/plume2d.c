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

#include <glib.h>
#include "plumevars.h"
#include "plumeinput.h"
#include <utils.h>

//void init_plume2d(plume_inputs *plume_const,river_type river_passed,int ngrains_passed,sedload_type *sedload_passed, double dx,double **deposit,int depositLen);

gboolean plume2d(Plume_inputs *plume_const,Plume_river river_passed,int ngrains_passed,Plume_sediment *sedload_passed, double dx,double **deposit,int depositLen,Plume_data*);
gboolean plume3d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  Eh_dbl_grid *deposit      , Plume_data *data );

Plume_data *init_plume_data( Plume_data *data )
{
   data->x_len = 0;
   data->y_len = 0;
   data->ccnc  = NULL;
   data->ncnc  = NULL;
   data->deps  = NULL;
   data->dist  = NULL;
   data->ualb  = NULL;
   data->pcent = NULL;
   data->xval  = NULL;
   data->yval  = NULL;

   return data;
}

void destroy_plume_data( Plume_data *grid )
{
   if ( grid )
   {
      eh_free( grid->xval );
      eh_free( grid->yval );

      free_d3tensor( grid->ccnc  );
      free_d3tensor( grid->ncnc  );
      free_d3tensor( grid->deps  );
      free_d3tensor( grid->dist  );
      free_dmatrix ( grid->ualb  );
      free_dmatrix ( grid->pcent );

      eh_free( grid );
   }
}

gboolean plume2d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  double dx                 , double **deposit        ,
                  int deposit_len           , Plume_data *data )
{
   int err;
   Plume_enviro env;
   Plume_ocean ocean;
   Plume_grid *grid = data;
   Plume_options opt;

   ocean.Cw = plume_const->ocean_concentration;
   ocean.So = 1.;
   ocean.Sw = 32.1;
   ocean.cc = .5;
   ocean.vo = 0.;

   grid->ymin    = plume_const->plume_width/2 - plume_const->plume_width;
   grid->ymax    = grid->ymin + plume_const->plume_width;
   grid->ndx     = plume_const->ndx;
   grid->ndy     = plume_const->ndy;
   grid->max_len = dx*deposit_len;

   opt.fjrd = 1;
   opt.strt = 1;
   opt.kwf  = 0;
   opt.o1   = 0;
   opt.o2   = 1;
   opt.o3   = 0;

   env.n_grains = n_grains;
   env.lat      = 45.;
   env.river = &river;
   env.ocean = &ocean;
   env.sed   = sedload;

   err = plume( &env , grid , &opt );
   if ( !err )
   {
      // Output data files.
      plumeout2( &env , grid , dx,deposit,deposit_len,n_grains,plume_const->plume_width);

      return TRUE;
   }
   else
      return FALSE;

}
/*
Plume_river plume_init_plume_river( Sed_hydro river )
{
   Plume_river riv;

   {
      riv.Cs = eh_new( double , sed_sediment_env_size() );
      for ( n=0 ; n<n_grains ; n++ )
         riv.Cs[n] = sed_hydro_nth_conc( river , n );
      riv.u0 = sed_hydro_velocity(river);
      riv.b0 = sed_hydro_width(river);
      riv.d0 = sed_hydro_depth(river);
      riv.Q  = sed_hydro_water_flux(river);

      riv.rdirection = G_PI_2;
      riv.rma        = 0.;
   }

   return riv;
}

Plume_sediment plume_init_plume_sediment( Sed_sediment sed )
{
   Plume_sediment* sedload = eh_new( Plume_sediment , sed_sediment_n_types(sed)-1 );

   {
      Sed_type this_type;

      for ( n=1 ; n<n_grains ; n++ )
      {
         this_type = sed_sediment_type( sed , n );

         sedload[n-1].lambda = sed_type_lambda ( this_type );
         sedload[n-1].rho    = sed_type_rho_sat( this_type );
      }
   }

   return sedload;
}

Eh_dbl_grid* plume_init_plume_grid( Eh_cell_grid deposit )
{
   Eh_dbl_grid* grid;

   {
      gssize n;
      gssize n_grains = sed_sediment_env_size();

      grid = eh_new( Eh_dbl_grid , n_grains-1 );
      for ( n=0 ; n<4 ; n++ )
      {
         grid[n] = eh_grid_new( double , eh_grid_n_x(deposit) , eh_grid_n_y(deposit) );

         eh_grid_set_x_lin( grid[n] , eh_grid_x(deposit)[0] , dx );
         eh_grid_set_y_lin( grid[n] , eh_grid_y(deposit)[0] , dy );
      }
   }

   return grid;
}

gboolean plume_3d( Plume_inputs* plume_const , Sed_hydro river ,
                   Eh_cell_grid deposit      , Plume_data* data )
{
   Plume_river         riv = plume_init_plume_river   ( river              );
   Plume_sediment* sedload = plume_init_plume_sediment( sed_sediment_env() );
   Eh_dbl_grid*       grid = plume_init_plume_grid    ( deposit            );

   plume3d( plume_const , riv , sed_sediment_env_size() , sedload , grid , data );

   plume_fill_cell_grid( deposit , grid );

   return TRUE;
}
*/
gboolean plume3d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  Eh_dbl_grid *deposit      , Plume_data *data )
{
   int err;
   Plume_enviro env;
   Plume_ocean ocean;
   Plume_grid *grid = data;
   Plume_options opt;

   ocean.Cw = plume_const->ocean_concentration;
   ocean.So = 1.;
   ocean.Sw = 32.1;
   ocean.cc = .9;
   ocean.vo = plume_const->current_velocity;

   grid->ymin    = plume_const->plume_width/2 - plume_const->plume_width;
   grid->ymax    = grid->ymin + plume_const->plume_width;
   grid->ndx     = plume_const->ndx;
   grid->ndy     = plume_const->ndy;
   grid->max_len = eh_grid_y(deposit[0])[eh_grid_n_y(deposit[0])-1]
                 - eh_grid_y(deposit[0])[0];

   opt.fjrd = 0;
   opt.strt = 0;
   opt.kwf  = 1;
   opt.o1   = 0;
   opt.o2   = 1;
   opt.o3   = 0;

   if ( fabs(ocean.vo) < 1e-5 )
      opt.strt = 1;

   env.n_grains = n_grains;
   env.lat      = 49.;
   env.river = &river;
   env.ocean = &ocean;
   env.sed   = sedload;

   err = plume( &env , grid , &opt );
   if ( !err )
   {
      // Output data files.
      plumeout3( &env , grid , deposit );

      return TRUE;
   }
   else
      return FALSE;

}

