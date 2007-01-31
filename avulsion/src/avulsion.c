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

#include <math.h>
#include <glib.h>
#include "utils.h"
#include "avulsion.h"

GQuark
avulsion_data_struct_quark( void )
{
   return g_quark_from_static_string( "avulsion-data-struct-quark" );
}

Avulsion_st*
avulsion_new( GRand* rand , double std_dev )
{
   Avulsion_st* data = eh_new( Avulsion_st , 1 );

   data->rand    = rand;
   data->std_dev = std_dev;

   return data;
}

Avulsion_st*
avulsion_destroy( Avulsion_st* data )
{
   if ( data )
   {
      g_rand_free( data->rand );
      data->std_dev = 0;
      data->rand    = NULL;
      eh_free( data );
   }
   return NULL;
}

double
get_std_dev_func( double angle , double std_dev )
{
   return std_dev*(1. - .25*exp(-pow(angle-.0*G_PI,2.)) );
}

double
avulsion( GRand* rand , double last_angle , double std_dev )
{
   double new_angle;
//   double d_angle = eh_rand_normal( rand , 0. , std_dev );
   double d_angle = eh_rand_normal( rand , 0. , get_std_dev_func(last_angle,std_dev) );

   new_angle = last_angle+d_angle;

   if      ( new_angle>G_PI )
      new_angle = 2.*G_PI - new_angle;
   else if ( new_angle<-G_PI )
      new_angle = -2.*G_PI - new_angle;

   new_angle = eh_reduce_angle( new_angle );

   return new_angle;
}

double
avulsion_scale_angle_down( double angle , double min_angle , double max_angle )
{
   return   angle*(max_angle-min_angle)/(2.*G_PI) + .5*(min_angle+max_angle);
}

double
avulsion_scale_angle_up( double angle , double min_angle , double max_angle )
{
   return (angle - .5*(min_angle+max_angle))*(2.*G_PI)/(max_angle-min_angle);
}

double
avulsion_scale_std_dev_up( double std_dev , double min_angle , double max_angle )
{
   return std_dev*2.*G_PI/( max_angle-min_angle );
}

Sed_riv
sed_river_set_avulsion( Sed_riv r , Avulsion_st* data )
{
   return sed_river_set_data_full( r , AVULSION_DATA , data , avulsion_destroy );
}

Avulsion_st*
sed_river_avulsion( Sed_riv r )
{
   return sed_river_data( r , AVULSION_DATA );
}

Sed_riv
sed_river_avulse( Sed_riv r )
{
   eh_require( r );

   if ( r )
   {
      double       angle;
      double       last_angle = sed_river_angle( r );
      Avulsion_st* data       = sed_river_data( r , AVULSION_DATA );

      if ( data )
      {
         GRand* rand       = data->rand;
         double std_dev    = data->std_dev;
         double min_angle  = sed_river_min_angle(r);
         double max_angle  = sed_river_max_angle(r);

         std_dev    = avulsion_scale_std_dev_up( std_dev    , min_angle  , max_angle );
         last_angle = avulsion_scale_angle_up  ( last_angle , min_angle  , max_angle );
         angle      = avulsion                 ( rand       , last_angle , std_dev   );
         angle      = avulsion_scale_angle_down( angle      , min_angle  , max_angle );

         sed_river_set_angle( r , angle );
      }
      else
         eh_require_not_reached();
   }

   return r;
}

Sed_cube
sed_cube_avulse_river( Sed_cube c , Sed_riv r )
{

   if ( c )
   {
      sed_river_avulse( r );
      sed_cube_find_river_mouth( c , r );
   }

   return c;
}

void
sed_cube_avulse_river_helper( Sed_riv r , Sed_cube c )
{
   sed_cube_avulse_river( c , r );
}

Sed_cube
sed_cube_avulse_all_rivers( Sed_cube c )
{
   return sed_cube_foreach_river( c , &sed_cube_avulse_river_helper , c );
}


