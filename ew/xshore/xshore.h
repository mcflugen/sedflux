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

#ifndef _XSHORE_H_
# define _XSHORE_H_
   
# include <utils/utils.h>
# include <sed/sed_sedflux.h>

#define XSHORE_BRUUN_M ( 0.6 )

typedef struct
{
   Sed_cell added;
   Sed_cell lost;
   double bruun_a;
   double bruun_m;
   double bruun_b;
   double bruun_y_0;
   double bruun_y_b;
   double bruun_h_b;
   double z_0;
   gint n_zones;
   double* dt;
}
Xshore_info;

Xshore_info  xshore                      ( Sed_cube p                ,
                                           Sed_cell along_shore_cell ,
                                           double xshore_current     ,
                                           Sed_ocean_storm storm );
double     get_breaking_wave_depth       ( Sed_wave deep_water );
Sed_cube*  get_shelf_zones               ( Sed_cube p               ,
                                           double z_0               ,
                                           gint** shelf_ind );
Sed_cube*  get_bruun_zones               ( Sed_cube p               ,
                                           double y_0 );
Sed_cube* get_zones                      ( Sed_cube p               ,
                                           double* z                ,
                                           gint n_zones           ,
                                           Sed_grid_func f          ,
                                           gint** ind );
gint     get_zone_indices              ( Sed_cube p               ,
                                           double z_0               ,
                                           double z_1               ,
                                           gint i_0               ,
                                           Sed_grid_func get_val    ,
                                           gint* ind );
double     get_diffusion_constant        ( double z                 ,
                                           Sed_wave w              ,
                                           double settling_velocity ,
                                           double breaker_depth );
double     get_advection_constant        ( double z                 ,
                                           Sed_wave w              ,
                                           double u_0               ,
                                           double settling_velocity ,
                                           double breaker_depth );
void       add_suspended_sediment        ( Sed_column* col         , Sed_cell cell );
double     get_bruun_depth               ( double y                , double y_0 ,
                                           double bruun_a          , double bruun_m );
double     get_bruun_a                   ( Sed_cube p              , double bruun_m );
double     get_bruun_y_0                 ( Sed_cube p );
double     get_bruun_y_b                 ( Sed_cube p );
double*    get_bruun_profile             ( double* y               , gint len ,
                                           double bruun_a          , double bruun_m ,
                                           double y_0              , double y_b );
void       fill_to_bruun                 ( Sed_cube p             ,
                                           double* h              ,
                                           Sed_cell fill_cell );
void       fill_to_bruun_profile         ( Sed_cube p             ,
                                           Sed_wave deep_wave     ,
                                           double bruun_m          ,
                                           Sed_cell fill_cell     ,
                                           Sed_cell added_fill_cell );

double*    get_max_erosion_profile       ( Sed_cube p             , Sed_wave w );
double     get_near_bed_velocity         ( double water_depth ,
                                           Sed_wave w        ,
                                           double breaker_depth      );
double     near_bed_velocity_func        ( double water_depth ,
                                           Sed_wave w        ,
                                           double breaker_depth      );
double     near_bed_velocity_func_mean   ( double water_depth ,
                                           Sed_wave w        ,
                                           double breaker_depth      );
double     near_bed_velocity_func_komar  ( double water_depth ,
                                           Sed_wave w        ,
                                           double breaker_depth      );
double     near_bed_velocity_func_stokes ( double water_depth ,
                                           Sed_wave w        ,
                                           double breaker_depth      );
double     get_grain_size_threshold      ( double orbital_velocity ,
                                           double wave_period        );
#endif

