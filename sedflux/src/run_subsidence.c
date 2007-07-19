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

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define SED_SUBSIDENCE_PROC_NAME "subsidence"
#define EH_LOG_DOMAIN SED_SUBSIDENCE_PROC_NAME

#include "utils.h"
#include "sed_sedflux.h"
#include "my_processes.h"

typedef struct
{
   double time;
   double *rate;
}
Subsidence_record_t;

//GArray *read_tectonic_curve(const char *filename, double *x_resample, int len);
int get_tectonics(GArray *rate_array, double year, double *rate_resample, int len);

gboolean init_subsidence_data( Sed_process proc , Sed_cube prof , GError** error );

Sed_process_info
run_subsidence( Sed_process proc , Sed_cube prof )
{
   Subsidence_t*    data = sed_process_user_data(proc);
   Sed_process_info info = SED_EMPTY_INFO;
   gsize i, j, n;
   double year;
   double start_year, end_year;
   double upper_edge, lower_edge;
   double time_step, total_time=0., total_subsidence=0.;

   if ( sed_process_run_count(proc)==0 )
      init_subsidence_data( proc , prof , NULL );

   start_year      = data->last_year;
   end_year        = sed_cube_age_in_years( prof );
   year            = sed_cube_age_in_years( prof );
   time_step       = year - data->last_year;
   data->last_year = year;

   eh_message( "subsidence time step (years): %f" , time_step );

   //---
   // Subside the basin.  If there is only one subsidence record, assume that 
   // subsidence is constant with time.  If there are multiple records
   // integrate the subsidence over the time step.
   //
   // Note that the time step will be 0 at the beginning of an epoch, in
   // this case, don't do anything.
   //---
   if ( data->subsidence_seq->len == 1 && time_step>1e-6 )
   {
      double** sub_grid = eh_dbl_grid_data(data->subsidence_seq->data[0]);
      for ( i=0 ; i<sed_cube_n_x(prof) ; i++ )
         for ( j=0 ; j<sed_cube_n_y(prof) ; j++ )
            sed_cube_adjust_base_height( prof , i , j , sub_grid[i][j]*time_step );
   }
   else if ( time_step>1e-6 )
   {
      for ( n=0,total_time=0 ; n<data->subsidence_seq->len ; n++ )
      {
         lower_edge = data->subsidence_seq->t[n];
         if ( n<data->subsidence_seq->len-1 )
            upper_edge = data->subsidence_seq->t[n+1];
         else
            upper_edge = G_MAXDOUBLE;

         if ( start_year >= upper_edge )
            time_step = -1;
         else if ( start_year >= lower_edge && end_year <  upper_edge )
            time_step = end_year - start_year;
         else if ( start_year < lower_edge && end_year >= upper_edge )
            time_step = upper_edge - lower_edge;
         else if ( start_year >= lower_edge && end_year >= upper_edge )
            time_step = upper_edge - start_year;
         else if ( start_year <  lower_edge && end_year <  upper_edge )
            time_step = end_year - lower_edge;
         else
            time_step = -1;

         if ( time_step > 0 )
         {
            double dz;
            double** sub_grid_l = eh_dbl_grid_data(data->subsidence_seq->data[n]  );
            double** sub_grid_h = eh_dbl_grid_data(data->subsidence_seq->data[n+1]);
            
            total_time += time_step;
            for ( i=0 ; i<sed_cube_n_x(prof) ; i++ )
               for ( j=0 ; j<sed_cube_n_y(prof) ; j++ )
               {
                  dz = (   (sub_grid_h[i][j] - sub_grid_l[i][j] )
                         / (upper_edge - lower_edge ) )
                     * (start_year - lower_edge)
                     + sub_grid_l[i][j];
                  sed_cube_adjust_base_height( prof , i , j , dz*time_step );
                  total_subsidence += dz*time_step;
               }
         }
      }

      if ( fabs( total_time - (end_year-start_year) ) > 1e-5 )
      {
         eh_warning( "The current time interval is not completely contained "
                     " within the subsidence curve." );
         eh_warning( "Start of this time interval: %f" , start_year );
         eh_warning( "End of this time interval: %f" , end_year );
         eh_warning( "Total time: %f" , total_time );
      }
   }

   eh_message( "Time: %f"                 , sed_cube_age( prof ) );
   eh_message( "Total subsidence (m): %f" , total_subsidence );

   return info;
}

#define SUBSIDENCE_KEY_FILENAME "subsidence file"

static gchar* subsidence_req_labels[] =
{
   SUBSIDENCE_KEY_FILENAME ,
   NULL
};

gboolean
init_subsidence( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Subsidence_t* data    = sed_process_new_user_data( p , Subsidence_t );
   GError*       tmp_err = NULL;
   gboolean      is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->last_year      = 0.;
   data->subsidence_seq = NULL;

   eh_symbol_table_require_labels( tab , subsidence_req_labels , &tmp_err );

   if ( !tmp_err )
   {
      data->filename = eh_symbol_table_value( tab , SUBSIDENCE_KEY_FILENAME );

      eh_touch_file( data->filename , O_RDONLY , &tmp_err );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_subsidence_data( Sed_process proc , Sed_cube prof , GError** error )
{
   gboolean      is_ok = TRUE;
   Subsidence_t* data  = sed_process_user_data( proc );

   if ( data )
   {
      GError* tmp_err = NULL;
      double* y       = sed_cube_y( prof , NULL );

      data->last_year = sed_cube_age_in_years(prof);

      if ( sed_mode_is_3d() )
         data->subsidence_seq  = sed_get_floor_sequence_3(
                                    data->filename ,
                                    sed_cube_x_res( prof ) ,
                                    sed_cube_y_res( prof ) ,
                                    &tmp_err );
      else
         data->subsidence_seq  = sed_get_floor_sequence_2(
                                    data->filename     ,
                                    y                  ,
                                    sed_cube_n_y(prof) ,
                                    &tmp_err );



      eh_free( y );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         is_ok = FALSE;
      }
   }

   return is_ok;
}

gboolean
destroy_subsidence( Sed_process p )
{
   if ( p )
   {
      Subsidence_t* data = sed_process_user_data( p );
      
      if ( data )
      {
         gint i;

         for ( i=0 ; i<data->subsidence_seq->len ; i++ )
            eh_grid_destroy( data->subsidence_seq->data[i] , TRUE );
         eh_destroy_sequence( data->subsidence_seq , FALSE );

         eh_free( data->filename );
         eh_free( data           );
      }
   }

   return TRUE;
}

gboolean dump_subsidence_data( gpointer ptr , FILE *fp )
{
   Subsidence_t *data = (Subsidence_t*)ptr;
   guint len;

   len = strlen( data->filename )+1;
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( data->filename , sizeof(char) , len , fp );

   fwrite( &(data->last_year) , sizeof(double) , 1 , fp );

   len = data->tectonic_curve->len;
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( data->tectonic_curve->data , sizeof(double) , len , fp );

   return TRUE;
}

gboolean load_subsidence_data( gpointer ptr , FILE *fp )
{
   Subsidence_t *data = (Subsidence_t*)ptr;
   guint len;
   double *tectonic_data;

   fread( &len , sizeof(guint) , 1 , fp );
   fread( data->filename , sizeof(char) , len , fp );

   fread( &(data->last_year) , sizeof(double) , 1 , fp );

   fread( &len , sizeof(guint) , 1 , fp );
   tectonic_data = eh_new( double , len );
   fread( tectonic_data , sizeof(double) , len , fp );

   data->tectonic_curve = g_array_new( FALSE , FALSE , sizeof(double) );
   g_array_append_vals( data->tectonic_curve , tectonic_data , len );

   return TRUE;
}

int get_tectonics(GArray *rate_array, double year, double *rate_resample, int len)
{
   int i, j;
   Subsidence_record_t r1, r2;
   double time[2], rate[2], *rate1, *rate2;

// Determine which times we are between.
   for ( i=0 ;
            i<rate_array->len 
         && g_array_index(rate_array,Subsidence_record_t,i).time <= year ;
         i++ );

// Interpolate for the rates at the present time.  If we are at a time outside
// of the data, use the last rate given.
   if ( i >= rate_array->len )
   {
      g_warning( "current time is outside of bounds in subsidence file" );
      g_warning( "using the rates from the last subsidence record"      );
      i = rate_array->len-1;
      year = g_array_index( rate_array , Subsidence_record_t , i ).time;
   }
   if ( i == 0 )
   {
      g_warning( "current time is outside of bounds in subsidence file" );
      g_warning( "using the rates from the first subsidence record"     );
      i = 1;
      year = g_array_index( rate_array , Subsidence_record_t , 0 ).time;
   }

   r1 = g_array_index( rate_array , Subsidence_record_t , i-1 );
   r2 = g_array_index( rate_array , Subsidence_record_t , i   );

   rate1 = r1.rate;
   rate2 = r2.rate;

   time[0] = r1.time;
   time[1] = r2.time;

   for ( j=0 ; j<len ; j++ )
   {
      rate[0] = rate1[j];
      rate[1] = rate2[j];

      interpolate( time , rate , 2 , &year , &rate_resample[j] , 1 );
   }

   return 0;

}
/*
#define S_KEY_SUBSIDENCE_TIME "time"

GArray *read_tectonic_curve(const char *filename, double *x_resample, int len)
{
   int i;
   GPtrArray *data;
   GArray *rate_array = g_array_new( FALSE ,
                                     FALSE ,
                                     sizeof(Subsidence_record_t) );
   double *y_resample = eh_new0( double , len );
   double time;
   GPtrArray *x, *y;
   Subsidence_record_t rate;
   Eh_data_record *entry;

   data = eh_scan_data_file( filename , "," , FALSE , TRUE );

   if ( data->len < 2 )
      g_error("less than two subsidence records was found in %s." , filename );

   for ( i=0 ; i<data->len ; i++ )
   {
      y_resample = eh_new0( double , len );

      entry = g_ptr_array_index( data , i );
      time = strtotime( eh_symbol_table_lookup( entry->symbol_table ,
                                                S_KEY_SUBSIDENCE_TIME ) );
      x = g_ptr_array_index( entry->data , 0 );
      y = g_ptr_array_index( entry->data , 1 );

      if (    x_resample[len-1] > ((double*)x->pdata)[x->len-1]
           || x_resample[0]     < ((double*)x->pdata)[0] )
         g_error( "subsidence rates not given for the entire model domain" );

      interpolate( (double*)x->pdata , (double*)y->pdata , x->len ,
                    x_resample       , y_resample        , len );

      rate.time = time;
      rate.rate = y_resample;

      g_array_append_val( rate_array , rate );
   }

   return rate_array;
}
*/

