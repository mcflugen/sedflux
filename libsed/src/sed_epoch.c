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
#include <glib.h>
#include "utils.h"

#include "sed_epoch.h"
#include "sed_signal.h"

CLASS( Sed_epoch )
{
   char*             name;        //< Name that identifies the epoch
   double            number;      //< ID that identifies the epoch

   double            start;       //< Start time for the epoch
   double            duration;    //< Duration of the epoch
   double            time_step;   //< Time step of the epoch

   char*             filename;    //< Initialization file
   Sed_process_queue proc_q;      //< List of processes to run during the epoch
};

CLASS( Sed_epoch_queue )
{
   GList* l; //< A list of Sed_epoch
};

GQuark
sed_epoch_error_quark( void )
{
   return g_quark_from_static_string( "sed-epoch-error-quark" );
}

Sed_epoch
sed_epoch_new( void )
{
   Sed_epoch e;

   NEW_OBJECT( Sed_epoch , e );

   e->name      = NULL;
   e->number    = -1;

   e->start     =  0;
   e->duration  =  0;
   e->time_step =  0;

   e->filename  = NULL;
   e->proc_q    = NULL;

   return e;
}

Sed_epoch
sed_epoch_copy( Sed_epoch d , const Sed_epoch s )
{
   if ( s )
   {
      if ( !d )
         d = sed_epoch_new( );

      d->name      = g_strdup( s->name );
      d->number    = s->number;

      d->start     = s->start;
      d->duration  = s->duration;
      d->time_step = s->time_step;

      d->filename  = g_strdup( s->filename );

      if ( d->proc_q )
         sed_process_queue_destroy( d->proc_q );

      d->proc_q = sed_process_queue_dup( s->proc_q );
   }

   return d;
}

Sed_epoch
sed_epoch_dup( const Sed_epoch s )
{
   Sed_epoch d = NULL;

   if ( s ) d = sed_epoch_copy( NULL , s );

   return d;
}

#define SED_KEY_EPOCH_NUMBER       "number"
#define SED_KEY_EPOCH_DURATION     "duration"
#define SED_KEY_EPOCH_ACTIVE       "active"
#define SED_KEY_EPOCH_TIME_STEP    "time step"
#define SED_KEY_EPOCH_FILE         "process file"

static gchar* old_required_labels[] =
{
   SED_KEY_EPOCH_NUMBER       ,
   SED_KEY_EPOCH_DURATION     ,
   SED_KEY_EPOCH_TIME_STEP    ,
   SED_KEY_EPOCH_FILE         ,
   NULL
};

static gchar* required_labels[] =
{
   SED_KEY_EPOCH_ACTIVE       ,
   SED_KEY_EPOCH_TIME_STEP    ,
   SED_KEY_EPOCH_FILE         ,
   NULL
};

Sed_epoch_queue
sed_epoch_new_from_table( Eh_symbol_table t , GError** error )
{
   Sed_epoch_queue q = NULL;

   eh_require( t );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( t )
   {
      GError* tmp_err     = NULL;
      gchar*  time_s      = eh_symbol_table_value( t , SED_KEY_EPOCH_ACTIVE    );
      gchar*  number_s    = eh_symbol_table_value( t , SED_KEY_EPOCH_NUMBER    );
      gchar*  duration_s  = eh_symbol_table_value( t , SED_KEY_EPOCH_DURATION  );
      gchar*  file_s      = eh_symbol_table_value( t , SED_KEY_EPOCH_FILE      );
      gchar*  time_step_s = eh_symbol_table_value( t , SED_KEY_EPOCH_TIME_STEP );

      /* Still support the old way of specifying epoch durations and times */
      if (    !eh_symbol_table_has_labels( t , required_labels     )
           && !eh_symbol_table_has_labels( t , old_required_labels ) )
      {
         gchar* err_s;
         eh_symbol_table_require_labels( t , required_labels , &tmp_err );

         err_s = tmp_err->message;
         tmp_err->message = g_strconcat( "Missing required labels in epoch group\n" , err_s , NULL );
         eh_free( err_s );
         /*
         g_set_error( &tmp_err ,
                      SED_EPOCH_ERROR ,
                      SED_EPOCH_ERROR_MISSING_LABEL ,
                      "Missing required labels in epoch group" );
         */
      }

      /* Scan the active times the new way */
      if ( !tmp_err && time_s   ) q = sed_epoch_queue_new_sscan    ( time_s   , time_step_s , file_s , &tmp_err );
      if ( !tmp_err && number_s ) q = sed_epoch_queue_new_sscan_old( number_s , time_step_s , file_s , duration_s , &tmp_err );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = sed_epoch_queue_destroy( q );
      }

      eh_free( number_s    );
      eh_free( duration_s  );
      eh_free( time_s      );
      eh_free( file_s      );
      eh_free( time_step_s );
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_new_sscan_old( const gchar* number_s    ,
                               const gchar* time_step_s ,
                               const gchar* file_s      ,
                               const gchar* duration_s  ,
                               GError** error )
{
   Sed_epoch_queue q = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   {
      GError*   tmp_err = NULL;
      Sed_epoch e       = sed_epoch_new();

      if ( !tmp_err ) sed_epoch_sscan_number   ( e , number_s    , &tmp_err );
      if ( !tmp_err ) sed_epoch_sscan_duration ( e , duration_s  , &tmp_err );
      if ( !tmp_err ) sed_epoch_sscan_time_step( e , time_step_s , &tmp_err );
      if ( !tmp_err ) sed_epoch_sscan_filename ( e , file_s      , &tmp_err );
      if ( !tmp_err )
      {
         NEW_OBJECT( Sed_epoch_queue , q );
         q->l = NULL;

         sed_epoch_queue_push_tail( q , e );
      }
      
      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = NULL;
      }
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_new_sscan( const gchar* time_s      ,
                           const gchar* time_step_s ,
                           const gchar* file_s      ,
                           GError** error )
{
   Sed_epoch_queue q = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( time_s && time_step_s && file_s )
   {
      GError*   tmp_err   = NULL;
      gchar**   time_str  = g_strsplit   ( time_s      , ";" , 0 );
      gchar**   dt_str    = g_strsplit   ( time_step_s , ";" , 0 );
      gint      len_time  = g_strv_length( time_str );
      gint      len_dt    = g_strv_length( dt_str   );
      gint      i;
      Sed_epoch e;

      NEW_OBJECT( Sed_epoch_queue , q );
      q->l = NULL;

      if ( len_time==len_dt )
      {
         for ( i=0 ; !tmp_err && i<len_time ; i++ )
         {
            e = sed_epoch_new();

            if ( !tmp_err ) sed_epoch_sscan_time      ( e , time_str[i] , &tmp_err );
            if ( !tmp_err ) sed_epoch_sscan_time_step ( e , dt_str[i]   , &tmp_err );
            if ( !tmp_err ) sed_epoch_sscan_filename  ( e , file_s      , &tmp_err );

            sed_epoch_queue_push_tail( q , e );
            
         }
      }
      else if ( len_dt==1 )
      {
         for ( i=0 ; !tmp_err && i<len_time ; i++ )
         {
            e = sed_epoch_new();

            if ( !tmp_err ) sed_epoch_sscan_time      ( e , time_str[i] , &tmp_err );
            if ( !tmp_err ) sed_epoch_sscan_time_step ( e , dt_str[0]   , &tmp_err );
            if ( !tmp_err ) sed_epoch_sscan_filename  ( e , file_s      , &tmp_err );

            sed_epoch_queue_push_tail( q , e );
         }
      }
      else
         g_set_error( &tmp_err ,
                      SED_EPOCH_ERROR ,
                      SED_EPOCH_ERROR_BAD_TIME_STEP ,
                      "Number of time steps does not match active periods (or scalar)" );

      g_strfreev( time_str );
      g_strfreev( dt_str   );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = sed_epoch_queue_destroy( q );
      }
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_new( const gchar* file , GError** error )
{
   Sed_epoch_queue q = NULL;

   eh_require( file );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file )
   {
      GError*     tmp_err  = NULL;
      Eh_key_file key_file = NULL;

      key_file = eh_key_file_scan( file , &tmp_err );

      if ( key_file )
      {
         Eh_symbol_table* tables     = eh_key_file_get_symbol_tables( key_file , "epoch" );
         Eh_symbol_table* this_table;
         Sed_epoch_queue  new_q;

         NEW_OBJECT( Sed_epoch_queue , q );
         q->l = NULL;

         for ( this_table=tables ; !tmp_err && *this_table ; this_table++ )
         {
            new_q = sed_epoch_new_from_table( *this_table , &tmp_err );

            q = sed_epoch_queue_concat( q , new_q );

            sed_epoch_queue_destroy( new_q );
         }

         sed_epoch_queue_order( q );

         for ( this_table=tables ; *this_table ; this_table++ )
            eh_symbol_table_destroy( *this_table );
         eh_free( tables );
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = sed_epoch_queue_destroy( q );
      }

      eh_key_file_destroy( key_file );
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_new_full( const gchar*       file          ,
                          Sed_process_init_t proc_defs[]   ,
                          Sed_process_family proc_family[] ,
                          Sed_process_check  proc_checks[] ,
                          GError**           error )
{
   Sed_epoch_queue q = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file )
   {
      GError* tmp_err = NULL;

      q = sed_epoch_queue_new( file , &tmp_err );

      if ( !tmp_err ) sed_epoch_queue_set_processes( q , proc_defs , proc_family , proc_checks , &tmp_err );
      if (  tmp_err ) g_propagate_error( error , tmp_err );
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_dup( const Sed_epoch_queue s )
{
   Sed_epoch_queue d = NULL;

   if ( s )
   {
      GList* l;

      NEW_OBJECT( Sed_epoch_queue , d );
      d->l = NULL;

      for ( l=s->l ; l ; l=l->next )
         d->l = g_list_prepend( d->l , sed_epoch_dup( l->data ) );
      d->l = g_list_reverse( d->l );
   }

   return d;
}

Sed_epoch_queue
sed_epoch_queue_concat( Sed_epoch_queue q_1 , Sed_epoch_queue q_2 )
{
   if ( q_1 && q_2 )
   {
      GList* l;
      GList* new_l = NULL;

      for ( l=q_2->l ; l ; l=l->next )
         new_l = g_list_prepend( new_l , sed_epoch_dup( l->data ) );
      q_1->l = g_list_concat( q_1->l , g_list_reverse(new_l) );
   }
   else if ( !q_1       )
   {
      q_1 = sed_epoch_queue_dup( q_2 );
   }

   return q_1;
}

gint
sed_epoch_start_cmp( Sed_epoch a , Sed_epoch b )
{
   gint val = 0;

   if ( a && b )
   {
      if ( a->start > b->start )
         val =  1;
      else if ( a->start < b->start )
         val = -1;
   }

   return val;
}

gint
sed_epoch_number_cmp( Sed_epoch a , Sed_epoch b )
{
   gint val = 0;

   if ( a && b )
   {
      if ( a->number > b->number )
         val =  1;
      else if ( a->number < b->number )
         val = -1;
   }

   return val;
}

Sed_epoch
sed_epoch_destroy( Sed_epoch e )
{
   if ( e )
   {
      sed_process_queue_destroy( e->proc_q );

      eh_free( e->name      );
      eh_free( e->filename  );
      eh_free( e            );
   }
   return NULL;
}

Sed_epoch_queue
sed_epoch_queue_destroy( Sed_epoch_queue list )
{
   if ( list )
   {
      GList* link;

      for ( link=list->l ; link ; link = link->next )
         sed_epoch_destroy( link->data );

      g_list_free( list->l );
      eh_free( list );
   }

   return NULL;
}

Sed_epoch
epoch_set_name( Sed_epoch e , const gchar* name )
{
   if ( e )
   {
      if ( e->name ) eh_free( e->name );

      e->name = g_strdup( name );
   }

   return e;
}

Sed_epoch
sed_epoch_set_number( Sed_epoch e , gssize n )
{
   eh_require( n>=0 );

   if ( e )
   {
      e->number = n;
   }

   return e;
}

Sed_epoch
sed_epoch_sscan_filename( Sed_epoch e , const gchar* file_s , GError** error )
{
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;

      if ( !eh_try_open( file_s ) )
      {
         g_set_error( &tmp_err ,
                      SED_EPOCH_ERROR ,
                      SED_EPOCH_ERROR_OPEN_FILE ,
                      "Could not open epoch file: %s" , file_s );
      }
      else
         sed_epoch_set_filename( e , file_s );
   }

   return e;
}

Sed_epoch
sed_epoch_sscan_number( Sed_epoch e , const gchar* number_s , GError** error )
{
   eh_require( number_s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;
      double  id = -1;

      id = eh_str_to_dbl( number_s , &tmp_err );

      if ( !tmp_err ) sed_epoch_set_number( e , id );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         e = NULL;
      }
   }

   return e;
}

Sed_epoch
sed_epoch_sscan_time( Sed_epoch e , const gchar* time_s , GError** error )
{
   eh_require( time_s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;
      double* time    = NULL;

      time = eh_str_to_time_range( time_s , &tmp_err );

      if ( !tmp_err ) sed_epoch_set_active_time( e , time );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         e = NULL;
      }

      eh_free( time );
   }

   return e;
}

Sed_epoch
sed_epoch_sscan_duration( Sed_epoch e , const gchar* duration_s , GError** error )
{
   eh_require( duration_s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;
      double  duration;

      duration = eh_str_to_time_in_years( duration_s , &tmp_err );

      /* Look for negative duration */
      if ( !tmp_err && duration < 0 )
         g_set_error( &tmp_err , 
                      SED_EPOCH_ERROR ,
                      SED_EPOCH_ERROR_NEGATIVE_DURATION ,
                      "Negative time step" );

      if ( !tmp_err ) sed_epoch_set_duration( e , duration );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         e = NULL;
      }
   }

   return e;
}

Sed_epoch
sed_epoch_sscan_time_step( Sed_epoch e , const gchar* dt_s , GError** error )
{
   eh_require( dt_s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;
      double  dt;

      dt = eh_str_to_time_in_years( dt_s , &tmp_err );

      /* Look for negative time step */
      if ( !tmp_err && dt < 0 )
         g_set_error( &tmp_err , 
                      SED_EPOCH_ERROR ,
                      SED_EPOCH_ERROR_NEGATIVE_TIME_STEP ,
                      "Negative time step" );

      if ( !tmp_err ) sed_epoch_set_time_step( e , dt );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         e = NULL;
      }
   }

   return e;
}

Sed_epoch
sed_epoch_set_active_time( Sed_epoch e , double* time )
{
   eh_require( e    );
   eh_require( time );

   if ( e && time )
   {
      e->start     = time[0];
      e->duration  = time[1] - time[0];
   }

   return e;
}

Sed_epoch
sed_epoch_set_duration( Sed_epoch e , double duration )
{
   if ( e )
   {
      e->duration = duration;
   }

   return e;
}

Sed_epoch
sed_epoch_set_time_step( Sed_epoch e , double time_step )
{
   if ( e )
      e->time_step = time_step;
   return e;
}

Sed_epoch
sed_epoch_set_filename( Sed_epoch e , const gchar* filename )
{
   if ( e )
   {
      if ( e->filename ) eh_free( e->filename );

      e->filename = g_strdup( filename );
   }

   return e;
}

const gchar*
sed_epoch_name( Sed_epoch e )
{
   eh_return_val_if_fail( e , NULL );

   return e->name;
}

gssize
sed_epoch_number( Sed_epoch e )
{
   eh_return_val_if_fail( e , G_MAXINT );

   return e->number;
}

double
sed_epoch_start( Sed_epoch e )
{
   eh_return_val_if_fail( e , G_MAXDOUBLE );

   return e->start;
}

double
sed_epoch_end( Sed_epoch e )
{
   eh_return_val_if_fail( e , G_MAXDOUBLE );

   return e->start + e->duration;
}

double
sed_epoch_duration( Sed_epoch e )
{
   eh_return_val_if_fail( e , 0 );

   return e->duration;
}

double
sed_epoch_time_step( Sed_epoch e )
{
   eh_return_val_if_fail( e , 0 );

   return e->time_step;
}

const gchar*
sed_epoch_filename( Sed_epoch e )
{
   eh_return_val_if_fail( e , NULL );

   return e->filename;
}

Sed_process_queue
sed_epoch_proc_queue( Sed_epoch e )
{
   eh_return_val_if_fail( e , NULL );

   return e->proc_q;
}

gssize
sed_epoch_queue_length( Sed_epoch_queue q )
{
   eh_return_val_if_fail( q , 0 );

   return g_list_length( q->l );
}

Sed_epoch_queue
sed_epoch_queue_order( Sed_epoch_queue q )
{
   if ( q )
   {
      /* If the first epoch was given in the new format, sort by start time */
      if ( q->l && sed_epoch_number( q->l->data ) < 0 )
         q->l = g_list_sort( q->l , (GCompareFunc)sed_epoch_start_cmp );
      else
         q->l = g_list_sort( q->l , (GCompareFunc)sed_epoch_number_cmp );
   }
   return q;
}

Sed_epoch_queue
sed_epoch_queue_push_tail( Sed_epoch_queue q , Sed_epoch e )
{
   if ( q && e )
      q->l = g_list_append( q->l , e );

   return q;
}

Sed_epoch
sed_epoch_queue_pop( Sed_epoch_queue q )
{
   Sed_epoch e = NULL;

   if ( q && q->l )
   {
      e = (q->l)->data;

      q->l = g_list_delete_link( q->l , q->l );
   }

   return e;
}

Sed_epoch
sed_epoch_queue_nth( Sed_epoch_queue q , gssize n )
{
   Sed_epoch e;

   if ( q && q->l )
   {
      e = g_list_nth_data( q->l , n );
   }

   return e;
}

gint
sed_epoch_is_in_range( Sed_epoch e , double* t )
{
   gint is_in_range = 1;
   if ( e && t )
   {
      if ( (*t) < sed_epoch_end(e) && (*t) >= sed_epoch_start(e) )
         is_in_range = 0;
   }
   return is_in_range;
}

Sed_epoch
sed_epoch_queue_find( Sed_epoch_queue q , double t )
{
   Sed_epoch e = NULL;

   if ( q )
   {
      GList* l = g_list_find_custom( q->l , &t , (GCompareFunc)sed_epoch_is_in_range );

      if ( l )
         e = l->data;
   }

   return e;
}

gssize
sed_epoch_fprint( FILE* fp , Sed_epoch e )
{
   gssize n = 0;

   if ( e )
   {
      gchar* file = (e->filename)?e->filename:"(null)";

      n += fprintf( fp , "[Epoch Info]\n" );

      n += fprintf( fp , "Id           = %f\n" , e->number    );
      n += fprintf( fp , "Start        = %f\n" , e->start     );
      n += fprintf( fp , "Duration     = %f\n" , e->duration  );

      n += fprintf( fp , "Time step    = %f  " , e->time_step );
      n += fprintf( fp , "# NOTE: A river file may override this time step!\n" );

      n += fprintf( fp , "Process file = %s\n" , file         );


   }
   return n;
}

gssize
sed_epoch_queue_fprint( FILE* fp , Sed_epoch_queue q )
{
   gssize n = 0;

   if ( q )
   {
      GList* link;

      for ( link=q->l ; link ; link=link->next )
      {
         n += sed_epoch_fprint( fp , link->data );
      }

   }

   return n;
}

Sed_epoch
sed_epoch_scan_proc_queue( Sed_epoch          e          ,
                           Sed_process_init_t p_list[]   ,
                           Sed_process_family p_family[] ,
                           Sed_process_check  p_check[]  ,
                           GError** error )
{
   eh_require( e );

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( e )
   {
      GError* tmp_err = NULL;

      e->proc_q  = sed_process_queue_init( sed_epoch_filename(e) , p_list , p_family , p_check , &tmp_err );
      if ( tmp_err )
         g_propagate_error( error , tmp_err );
   }

   return e;
}

Sed_epoch_queue
sed_epoch_queue_set_processes( Sed_epoch_queue    q          ,
                               Sed_process_init_t p_list[]   ,
                               Sed_process_family p_family[] ,
                               Sed_process_check  p_check[]  ,
                               GError** error )
{
   eh_require( q );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( q )
   {
      GList* l;
      GError* tmp_err = NULL;

      for ( l=q->l ; !tmp_err && l ; l=l->next )
         sed_epoch_scan_proc_queue( l->data , p_list , p_family , p_check , &tmp_err );

      if ( tmp_err ) g_propagate_error( error , tmp_err );
   } 
   return q;
}

gboolean
sed_epoch_queue_test_run( const Sed_epoch_queue q          ,
                          Sed_process_init_t    p_list[]   ,
                          Sed_process_family    p_family[] ,
                          Sed_process_check     p_check[]  ,
                          GError**              error )
{
   gboolean is_ok = FALSE;

   eh_require( q      );
   eh_require( p_list );
   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( q && p_list )
   {
      GError*           tmp_err = NULL;
      Sed_epoch_queue   epoch_q = sed_epoch_queue_dup( q );
      Sed_process_queue proc_q;
      Sed_epoch         epoch;

      for ( epoch = sed_epoch_queue_pop( epoch_q ) ;
            epoch && !tmp_err ;
            epoch = sed_epoch_queue_pop( epoch_q ) )
         proc_q  = sed_process_queue_init( sed_epoch_filename(epoch) , p_list , p_family , p_check , &tmp_err );

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         is_ok = FALSE;
      }
      else
         is_ok = TRUE;

      sed_epoch_queue_destroy( epoch_q );
   }

   return is_ok;
}

Sed_epoch_queue
sed_epoch_queue_run( Sed_epoch_queue q , Sed_cube p )
{
   eh_require( q );
   eh_require( p );

   if ( q && p )
   {
      Sed_process_queue proc_q;
      Sed_epoch         epoch;

      for ( epoch = sed_epoch_queue_pop( q ) ;
            epoch && !sed_signal_is_pending( SED_SIG_QUIT ) ;
            epoch = sed_epoch_queue_pop( q ) )
      {
         proc_q = sed_epoch_proc_queue( epoch );

         if ( proc_q )
         {
            sed_cube_set_time_step( p , sed_epoch_time_step( epoch ) );

            sed_process_queue_run_until ( proc_q , p , sed_epoch_end( epoch ) );
            sed_process_queue_run_at_end( proc_q , p );

            sed_process_queue_summary( stdout , proc_q );

            sed_epoch_destroy        ( epoch  );
            sed_cube_free_river      ( p      );
         }
         else
            eh_require_not_reached();
      }
   }

   return q;
}

Sed_epoch_queue
sed_epoch_queue_tic( Sed_epoch_queue   epoch_q ,
                     Sed_cube          p       )
{
   eh_require( epoch_q );
   eh_require( p       );

   if ( epoch_q && p )
   {
      double            t_start = sed_cube_age( p );
      Sed_epoch         epoch   = sed_epoch_queue_find( epoch_q , t_start );
      Sed_process_queue proc_q  = sed_epoch_proc_queue( epoch );
      double            t_stop  = t_start + sed_epoch_duration( epoch );

      sed_cube_set_time_step( p , sed_epoch_time_step( epoch ) );

      sed_process_queue_run_until( proc_q , p , t_stop );

      if ( t_stop > sed_epoch_end( epoch ) )
      {
         sed_process_queue_run_at_end( proc_q , p );

         sed_process_queue_summary( stdout , proc_q );

         sed_cube_free_river      ( p      );
      }
   }

   return epoch_q;
}

