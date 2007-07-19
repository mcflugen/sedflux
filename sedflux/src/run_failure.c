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

#define SED_FAIL_PROC_NAME "failure"
#define EH_LOG_DOMAIN SED_FAIL_PROC_NAME

#include <stdio.h>
#include <time.h>

#include "utils.h"
#include "sed_sedflux.h"
#include "failure.h"
#include "my_processes.h"

int get_tsunami_parameters(Sed_cube fail);

gboolean init_failure_data( Sed_process proc , Sed_cube prof , GError** error );

GQuark
failure_profile_data_quark( void )
{
   return g_quark_from_string( "failure-profile-data-quark" );
}

Sed_process_info
run_failure( Sed_process proc , Sed_cube p )
{
   Failure_proc_t*  data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   Sed_cube fail;
   int decision;
   int fail_count=0;
   int fs_min_start, fs_min_length;
   double fs_min;
   Sed_process fail_process;
   Failure_t failure_const;
   GTimer *time;
   Fail_profile *fail_prof=NULL;
   gboolean flow_ok = TRUE;

   if ( sed_process_run_count(proc)==0 )
      init_failure_data( proc , p , NULL );

/*
   prof = sed_create_empty_profile( p->n_y , p->sed );
   for ( i=0 ; i<prof->size ; i++ )
      prof->col[i] = p->col[0][i];
   prof->age = p->age;
   prof->time_step = p->time_step;
   prof->storm_value = p->storm_value;
   prof->quake_value = p->quake_value;
   prof->tidal_range = p->tidal_range;
   prof->tidal_period = p->tidal_period;
   prof->wave[0] = p->wave[0];
   prof->wave[1] = p->wave[1];
   prof->wave[2] = p->wave[2];
   prof->basinWidth = p->dx;
   prof->colWidth   = p->dy;
   prof->cellHeight = p->cell_height;
   prof->sealevel   = p->sea_level;
   prof->constants  = p->constants;
*/

   time = g_timer_new();

   fail_prof                       = data->fail_prof;
   failure_const.consolidation     = data->consolidation;
   failure_const.cohesion          = data->cohesion;
   failure_const.frictionAngle     = data->friction_angle;
   failure_const.gravity           = data->gravity;
   failure_const.density_sea_water = data->density_sea_water;

   fprintf( stderr , "\n" );

   eh_debug( "initializing failure profile" );
   fail_prof = fail_reinit_fail_profile( fail_prof , p , failure_const );

   do
   {

      g_timer_start(time);

      eh_debug( "updating profile for failures" );
      fail_update_fail_profile( fail_prof );

      eh_debug( "examining profile for failures" );
      fail_examine_fail_profile( fail_prof );

      eh_debug( "examination took %f seconds" , g_timer_elapsed(time,NULL) );

      fs_min        = fail_prof->fs_min_val;
      fs_min_start  = fail_prof->fs_min_start;
      fs_min_length = fail_prof->fs_min_len;

      eh_message( "time             : %f" , sed_cube_age_in_years(p) );
      eh_message( "factor of safety : %f" , fs_min        );
      eh_message( "CHOOSING LARGEST FAILURE SURFACE: NO" );
      eh_message( "failure location : %d" , fs_min_start  );
      eh_message( "failure length   : %d" , fs_min_length );
      eh_message( "water depth      : %f" ,
         (fs_min_start>=0)?sed_cube_water_depth(p,0,fs_min_start):-999 );

      // Fail the sediment above the ellipse that has the minimum factor
      // of safety.
      if ( fs_min>0 && fs_min<MIN_FACTOR_OF_SAFETY )
      {

         fail = get_failure_surface( p , fs_min_start , fs_min_length );

         get_tsunami_parameters( fail );

         if ( fail )
         {
            sed_cube_remove(p,fail);

            decision = decider(fail,data->decider_clay_fraction);
/*
            if ( decision == DECIDER_TURBIDITY_CURRENT )
            {
               tc = data->turbidity_current;
               //sed_process_data_val(tc,failure,Turbidity_t) = fail;

               ((Turbidity_t*)sed_process_data(tc))->failure = fail;
               flow_ok = sed_process_run_now(tc,p);
            }
            else if ( decision == DECIDER_DEBRIS_FLOW )
            {
               db = data->debris_flow;
               //sed_process_data_val(db,failure,Debris_flow_t) = fail;
               ((Debris_flow_t*)sed_process_data(db))->failure = fail;
               flow_ok = sed_process_run_now(db,p);
            }
            else if ( decision == DECIDER_SLUMP )
            {
               slump = data->slump;
               //sed_process_data_val(slump,failure,Slump_t) = fail;
               ((Slump_t*)sed_process_data(slump))->failure = fail;
               flow_ok = sed_process_run_now(slump,p);
            }
*/
            if      ( decision == DECIDER_TURBIDITY_CURRENT ) fail_process = data->turbidity_current;
            else if ( decision == DECIDER_DEBRIS_FLOW       ) fail_process = data->debris_flow;
            else if ( decision == DECIDER_SLUMP             ) fail_process = data->slump;

            sed_process_provide( fail_process , FAILURE_PROFILE_DATA , fail );

            flow_ok = sed_process_run_now( fail_process , p );

            sed_process_withhold( fail_process , FAILURE_PROFILE_DATA );

            sed_cube_destroy(fail);

         }

         fail_set_failure_surface_ignore( fail_prof    ,
                                          fs_min_start ,
                                          fs_min_length );

         if ( !flow_ok )
         {
            fail_set_failure_surface_ignore( fail_prof ,
                                             fs_min_start ,
                                             fs_min_length );
            flow_ok = TRUE;
         }
         else
         {
//            flow = data->flow;
//            flow_ok = sed_run_process_now(flow,p);
            fail_count++;
         }
     
      }
      else
         get_tsunami_parameters( NULL );

   }
   while ( fs_min > 0. && fs_min < MIN_FACTOR_OF_SAFETY && flow_ok && fail_count<100 );

/*
   for ( i=0 ; i<prof->size ; i++ )
      sed_destroy_cell( prof->in_suspension[i] );
   eh_free(prof->in_suspension);
   sed_destroy_cell( prof->erode );
   eh_free(prof->col);
*/

   g_timer_destroy(time);

   return info;
}

#define S_KEY_CONSOLIDATION  "coefficient of consolidation"
#define S_KEY_COHESION       "cohesion of sediments"
#define S_KEY_FRICTION_ANGLE "apparent coulomb friction angle"
#define S_KEY_CLAY_FRACTION  "fraction of clay for debris flow"

gboolean
init_failure( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Failure_proc_t* data    = sed_process_new_user_data( p , Failure_proc_t );
   GError*         tmp_err = NULL;
   gchar**         err_s   = NULL;
   gboolean        is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->fail_prof             = NULL;
   data->turbidity_current     = NULL;
   data->debris_flow           = NULL;
   data->slump                 = NULL;
   data->flow                  = NULL;

   data->consolidation         = eh_symbol_table_dbl_value( tab , S_KEY_CONSOLIDATION  );
   data->cohesion              = eh_symbol_table_dbl_value( tab , S_KEY_COHESION       );
   data->friction_angle        = eh_symbol_table_dbl_value( tab , S_KEY_FRICTION_ANGLE );
   data->decider_clay_fraction = eh_symbol_table_dbl_value( tab , S_KEY_CLAY_FRACTION  );

   data->friction_angle        *= S_RADS_PER_DEGREE;
   data->decider_clay_fraction /= 100.;

   data->gravity                = sed_gravity();
   data->density_sea_water      = sed_rho_sea_water();

   eh_check_to_s( data->consolidation>=0         , "Sediment consolidation positive" , &err_s );
   eh_check_to_s( data->cohesion>=0              , "Sediment cohesion positive"      , &err_s );
   eh_check_to_s( data->friction_angle>=0        , "Friction angle positive"         , &err_s );
   eh_check_to_s( data->decider_clay_fraction>=0 , "Clay fraction between 0 and 1"   , &err_s );
   eh_check_to_s( data->decider_clay_fraction<=1 , "Clay fraction between 0 and 1"   , &err_s );

   if ( !tmp_err && err_s )
      eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_failure_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Failure_proc_t* data = sed_process_user_data(proc);

   if ( data )
   {
      Failure_t failure_const;

      failure_const.consolidation     = data->consolidation;
      failure_const.cohesion          = data->cohesion;
      failure_const.frictionAngle     = data->friction_angle;
      failure_const.gravity           = data->gravity;
      failure_const.density_sea_water = data->density_sea_water;

      data->fail_prof = fail_init_fail_profile( prof , failure_const );

      data->turbidity_current = sed_process_child( proc , "TURBIDITY CURRENT" );
      data->debris_flow       = sed_process_child( proc , "DEBRIS FLOW"       );
      data->slump             = sed_process_child( proc , "SLUMP"             );

   }

   return TRUE;
}

gboolean
destroy_failure( Sed_process p )
{
   if ( p )
   {
      Failure_proc_t* data = sed_process_user_data( p );

      if ( data )
      {
         if ( data->fail_prof )
            fail_destroy_failure_profile( data->fail_prof );

         eh_free( data );
      }
   }

   return TRUE;
}

#include <math.h>

int get_tsunami_parameters(Sed_cube fail)
{
   int i;
   double rise;
   double b, d, theta, w, T, A, lambda;
   double s_0, t_0;
   double sin_theta;
   double g=sed_gravity();

   eh_require( fail );
   eh_require( sed_cube_is_1d(fail) );

   if ( fail )
   {
      rise  = fabs(    sed_cube_water_depth(fail,0,0)
                    - sed_cube_water_depth(fail,0,sed_cube_n_y(fail)-1) );
      b     = sed_cube_n_y(fail)*sed_cube_y_res(fail);
      theta = atan(rise/b);
      w     = .25*b;
      d     = sed_cube_water_depth(fail,0,sed_cube_n_y(fail)/2);

      for (i=0,T=0;i<sed_cube_n_y(fail);i++)
         if ( sed_cube_thickness(fail,0,i) > T )
            T = sed_cube_thickness(fail,0,i);

      sin_theta = sin(fabs(theta));

      if ( d < 0 )
      {
         eh_debug( "Failure depth is less than zero." );
         d = 0;
      }

      lambda = 3.87*pow(b*d/sin_theta,.5);
      A      = 0.224*T*(w/(w+lambda))
             * (         pow(sin_theta,1.29)
                 - 0.746*pow(sin_theta,2.29)
                 + 0.170*pow(sin_theta,3.29) )
             * pow(b/d,1.25);
      s_0 = 4.48*b;
      t_0 = 3.87*pow(b/(g*sin_theta),.5);

   }
   else
   {
      A      = -999;
      lambda = -999;
      theta  = -999;
      rise   = -999;
      T      = -999;
      s_0    = -999;
      t_0    = -999;
   }

   eh_message( "tsunami amplitude (m): %f"       , A      );
   eh_message( "tsunami wavelength (m): %f"      , lambda );
   eh_message( "sea floor slope (rads): %f"      , theta  );
   eh_message( "failure relief (m): %f"          , rise   );
   eh_message( "failure thickness (m): %f"       , T      );
   eh_message( "characteristic distance (m): %f" , s_0    );
   eh_message( "characteristic time (s)    : %f" , t_0    );

   return 0;
}
/*
gboolean dump_failure_data( gpointer ptr , FILE *fp )
{
   Failure_proc_t *data = (Failure_proc_t*)ptr;

   fwrite( data , sizeof(Failure_t) , 1 , fp );

   sed_dump_process( fp , data->turbidity_current );
   sed_dump_process( fp , data->debris_flow       );
   sed_dump_process( fp , data->slump             );

   fail_dump_fail_profile( data->fail_prof , fp );

   return TRUE;
}

gboolean load_failure_data( gpointer ptr , FILE *fp )
{
   Failure_proc_t *data = (Failure_proc_t*)ptr;

   fread( data , sizeof(Failure_t) , 1 , fp );

   sed_load_process( data->turbidity_current , fp );
   sed_load_process( data->debris_flow       , fp );
   sed_load_process( data->slump             , fp );

   fail_load_fail_profile( fp );

   return TRUE;
}
*/


