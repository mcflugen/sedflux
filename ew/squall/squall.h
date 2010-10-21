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

#ifndef _SQUALL_H_
# define _SQUALL_H_
   
# include <utils/utils.h>
# include <sed/sed_sedflux.h>
   
# define SQUALL_DEFAULT_C_E       (.2)
//# define SQUALL_DEFAULT_C_V       (.05)
# define SQUALL_DEFAULT_C_V       (0.05)
# define SQUALL_DEFAULT_M         (3.)
# define SQUALL_DEFAULT_A         (2.)
# define SQUALL_DEFAULT_Z_L       (1)
# define SQUALL_DEFAULT_C_H       (1.)
# define SQUALL_DEFAULT_ALPHA_REF (.005)

G_BEGIN_DECLS

void write_output_file( const char *file , Sed_cube p );
gboolean squall( Sed_cube , double );
double get_erosion_rate_from_profile( Sed_cube p , int i ,
                                      double h_c  , double h_w );
//                                      int i_c        , int i_w );
double get_shoreface_slope( Sed_cube p , int i_c , int i_w );
double get_erosion_rate( double h        , double h_w    , double h_c ,
                         double alpha_sf , double h_wave , double h_wave_fair );
double get_erosion_efficiency( double h , double h_w , double h_c );
double get_coastal_dissipation( double alpha_sf );
double get_coastal_wave_energy( double wave_height_actual ,
                                double wave_height_fair );
double get_wave_base( double wave_length );
double get_breaking_wave_base( double wave_height );
double get_deep_water_wave_base( double wave_height );
int *get_zone_boundaries( Sed_cube p , double h_w , double h_c );
double get_travel_dist( double grain_size_in_m , double depth , double dx );
double get_non_dim_travel_dist( double grain_size_in_m );
double *get_moveable_grains( double water_depth ,
                             double wave_height ,
                             double wave_period ,
                             Sed_sediment sed   ,
                             double *is_moveable );

G_END_DECLS

#endif

