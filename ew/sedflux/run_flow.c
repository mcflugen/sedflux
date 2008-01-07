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

#define EH_LOG_DOMAIN FLOW_PROCESS_NAME_S

#include <stdio.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

#include "sedflux.h"

void run_exponential_flow( Sed_column c , double time_now_in_years );
void run_terzaghi_flow( Sed_column c , double time_now_in_years );
void run_darcy_flow( Sed_column c , double dt_in_years );

Sed_process_info
run_flow( Sed_process proc , Sed_cube p )
{
   Flow_t*          data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   double           dt_in_years;
   double           time_now;
   
   time_now    = sed_cube_age_in_years( p );
   dt_in_years = time_now - data->last_time;

   eh_message( "time : %f"       , time_now    );
   eh_message( "dt (years) : %f" , dt_in_years );

   switch ( data->method )
   {
      case FLOW_ALGORITHM_EXPONENTIAL: eh_message( "method : %s" , "EXPONENTIAL" ); break;
      case FLOW_ALGORITHM_TERZAGHI:    eh_message( "method : %s" , "TERZAGHI" );    break;
      case FLOW_ALGORITHM_DARCY:       eh_message( "method : %s" , "DARCY" );       break;
      default:                         eh_message( "method : %s" , "UNKNOWN" ); eh_require_not_reached();
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
               case FLOW_ALGORITHM_EXPONENTIAL:
                  run_exponential_flow( this_col , time_now );
                  break;
               case FLOW_ALGORITHM_TERZAGHI:
                  run_terzaghi_flow( this_col , time_now );
                  break;
               case FLOW_ALGORITHM_DARCY:
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

gboolean
init_flow( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Flow_t*  data    = sed_process_new_user_data( p , Flow_t );
   GError*  tmp_err = NULL;
   gboolean is_ok   = TRUE;
   gchar*   key;

   data->last_time = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   key = eh_symbol_table_lookup( tab , FLOW_KEY_METHOD );

   if      ( g_ascii_strcasecmp( key , "EXPONENTIAL" )==0 ) data->method = FLOW_ALGORITHM_EXPONENTIAL;
   else if ( g_ascii_strcasecmp( key , "DARCY"       )==0 ) data->method = FLOW_ALGORITHM_DARCY;
   else if ( g_ascii_strcasecmp( key , "TERZAGHI"    )==0 ) data->method = FLOW_ALGORITHM_TERZAGHI;
   else
      g_set_error( &tmp_err ,
                   SEDFLUX_ERROR ,
                   SEDFLUX_ERROR_BAD_ALGORITHM ,
                   "Invalid fluid flow algorithm (exponential, darcy, or terzaghi): %s" , key );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
destroy_flow( Sed_process p )
{
   if ( p )
   {
      Flow_t* data = sed_process_user_data( p );

      if ( data ) eh_free( data );
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


