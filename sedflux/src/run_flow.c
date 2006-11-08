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

#define SED_FLOW_PROC_NAME "flow"
#define EH_LOG_DOMAIN SED_FLOW_PROC_NAME

#include <stdio.h>
#include "sed_sedflux.h"
#include "run_flow.h"

void run_exponential_flow( Sed_column c , double time_now_in_years );
void run_terzaghi_flow( Sed_column c , double time_now_in_years );
void run_darcy_flow( Sed_column c , double dt_in_years );

Sed_process_info run_flow(gpointer ptr, Sed_cube p)
{
   Flow_t *data=(Flow_t*)ptr;
   double dt_in_years;
   double time_now;
   Sed_process_info info = SED_EMPTY_INFO;
   
   if ( p == NULL )
   {
      if ( data->initialized )
      {
//         eh_free( data->old_load );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
//      data->len       = p->size;
//      data->old_load  = eh_new0( double , p->size );
      data->last_time = 0;

      data->initialized = TRUE;
   }

   time_now    = sed_cube_age_in_years( p );
   dt_in_years = time_now - data->last_time;

   eh_message( "time : %f"       , time_now    );
   eh_message( "dt (years) : %f" , dt_in_years );

   switch ( data->method )
   {
      case FLOW_EXPONENTIAL:
         eh_message( "method : %s" , "EXPONENTIAL" );
         break;
      case FLOW_TERZAGHI:
         eh_message( "method : %s" , "TERZAGHI" );
         break;
      case FLOW_DARCY:
         eh_message( "method : %s" , "DARCY" );
         break;
      default:
         eh_message( "method : %s" , "UNKNOWN" );
         eh_require_not_reached();
   }

   {
      gssize i;
      gssize len = sed_cube_size(p);
      Sed_column this_col;

      for ( i=0 ; i<len ; i++ )
      {
         this_col = sed_cube_col(p,i);

         if ( sed_column_len( this_col )>3 )
         {
            switch ( data->method )
            {
               case FLOW_EXPONENTIAL:
                  run_exponential_flow( this_col , time_now );
                  break;
               case FLOW_TERZAGHI:
                  run_terzaghi_flow( this_col , time_now );
                  break;
               case FLOW_DARCY:
                  run_darcy_flow( this_col , dt_in_years );
                  break;
               default:
                  eh_require_not_reached();
            }
         }
      }
   }

   // the current time is now the old time.
   data->last_time = time_now;
   
   return info;
}

#define S_KEY_METHOD "method"

gboolean init_flow( Eh_symbol_table symbol_table,gpointer ptr)
{
   Flow_t *data=(Flow_t*)ptr;
   char *key;

   if ( symbol_table == NULL )
   {
      data->initialized = FALSE;
      return TRUE;
   }

   key = eh_symbol_table_lookup( symbol_table , S_KEY_METHOD );

   if ( g_ascii_strcasecmp( key , "EXPONENTIAL" )==0 )
      data->method = FLOW_EXPONENTIAL;
   else if ( g_ascii_strcasecmp( key , "DARCY" )==0 )
      data->method = FLOW_DARCY;
   else if ( g_ascii_strcasecmp( key , "TERZAGHI" )==0 )
      data->method = FLOW_TERZAGHI;
   else
   {
      eh_warning( "Unkown keyword for flow: %s" , key );
      return FALSE;
   }

   return TRUE;
}

gboolean dump_flow_data( gpointer ptr , FILE *fp )
{
   Flow_t *data = (Flow_t*)ptr;

   fwrite( data           , sizeof(Flow_t) , 1         , fp );
   fwrite( data->old_load , sizeof(double) , data->len , fp );

   return TRUE;
}

gboolean load_flow_data( gpointer ptr , FILE *fp )
{
   Flow_t *data = (Flow_t*)ptr;

   fread( data           , sizeof(Flow_t) , 1         , fp );
   fread( data->old_load , sizeof(double) , data->len , fp );

   return TRUE;
}

void run_exponential_flow( Sed_column c , double time_now_in_years )
{
   int j, n;
   double *k;
   double dt_in_secs, dt_in_years;
   Sed_cell this_cell;

   n = sed_column_len( c );

   k = eh_new( double , n );

   for ( j=0 ; j<n ; j++ )
      k[j] = sed_cell_cc( sed_column_nth_cell(c,j) );

   for ( j=0 ; j<n ; j++ )
   {
      this_cell = sed_column_nth_cell( c , j );

      dt_in_years = time_now_in_years
                  - sed_cell_age( this_cell );
      dt_in_secs  = dt_in_years * S_SECONDS_PER_YEAR;

      eh_lower_bound( dt_in_secs , 0 );

      sed_cell_set_pressure( this_cell ,
                               sed_cell_pressure( this_cell )
                             * exp( -dt_in_years/k[j] ) );

   }

   eh_free( k );

   return;
}

void run_terzaghi_flow( Sed_column c , double time_now_in_years )
{
   int j, n;
   double *u, *c_v;
   double burial_depth;
   double dt_in_secs, dt_in_years;
   Sed_cell this_cell;

   n = sed_column_len( c );

   u   = eh_new( double , n );
   c_v = eh_new( double , n );

   for ( j=0 ; j<n ; j++ )
      c_v[j] = sed_cell_cv( sed_column_nth_cell(c,j) );

   burial_depth = sed_column_thickness( c );

   for ( j=0 ; j<n ; j++ )
   {
      this_cell = sed_column_nth_cell( c,j );

      dt_in_years = time_now_in_years
                  - sed_cell_age( this_cell );
      dt_in_secs  = dt_in_years * S_SECONDS_PER_YEAR;

      eh_lower_bound( dt_in_secs , 0 );

      u[j] = sed_calculate_consolidation( c_v[j]       ,
                                          burial_depth ,
                                          burial_depth ,
                                          dt_in_secs );

      burial_depth -= sed_cell_size( this_cell );
      sed_cell_set_pressure( this_cell ,
                               sed_cell_pressure( this_cell )
                             * (1-u[j]) );

   }

   eh_free( c_v      );
   eh_free( u        );

   return;
}

void run_darcy_flow( Sed_column col , double dt_in_years )
{
   int j, n;
   double *load, *u, *k, *c;
   double hydro_static, dz, sed_rate;
   double max_k, mean_k, mean_c;
   double time, dt_in_secs;
   double *solve_excess_pore_pressure( double* , double* , double* , int ,
                                       double  , double  , double  , double );
   Sed_cell this_cell;

   dt_in_secs   = years_to_secs( dt_in_years );

   hydro_static = sed_column_water_pressure( col );
   dz           = sed_column_z_res( col );
   load         = sed_column_load( col , 0 , -1 , NULL );
//   sed_rate     = ( load[0] - data->old_load[i] ) / dt_in_secs;

   n = sed_column_len( col );

   u = eh_new( double , n );
   k = eh_new( double , n );
   c = eh_new( double , n );

   for ( j=n-1 ; j>=0 ; j-- )
   {
      this_cell = sed_column_nth_cell( col , j );

      u[j] = sed_cell_pressure( this_cell ) - hydro_static;

      eh_lower_bound( u[j] , 1e-5 );

      k[j] = sed_cell_hydraulic_conductivity( this_cell );
      c[j] = sed_cell_compressibility       ( this_cell )
           * sed_rho_sea_water()*sed_gravity();


//c[j] = 0;
//k[j] = 0;
//k[j] /= 1e6;
//k[j] = 1e-9;
//c[j] = 1e-4;
//c[j] *= 1000000;

      eh_lower_bound( max_k , k[j]/c[j] );

   }

   mean_k = eh_dbl_array_mean( k , n );
   mean_c = eh_dbl_array_mean( c , n );

mean_c = 1.;

   eh_dbl_array_set( k , n , mean_k );
   eh_dbl_array_set( c , n , mean_c );

sed_rate = 0;

   // solve for the excess porewater pressure for this column.
   for ( time=0 ; time<dt_in_secs ; time+=dt_in_secs )
      solve_excess_pore_pressure( u  , k          , c  , n ,
                                  dz , dt_in_secs , 0. , sed_rate );

   // set_the new excess porewater pressures for this column.
   for ( j=0 ; j<n ; j++ )
      sed_cell_set_pressure( sed_column_nth_cell(col,j) ,
                             (u[j]<0)? (hydro_static):(u[j]+hydro_static) );
   
//   data->old_load[i] = load[0];

   eh_free( load );
   eh_free( u    );
   eh_free( k    );
   eh_free( c    );

   return;
}


