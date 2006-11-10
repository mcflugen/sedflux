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

#include "sed_process.h"

typedef struct
{
// Public
   double mass_added;
   double mass_lost;
   gboolean error;

// Private
   double mass_before;
   double mass_after;
   double mass_total_added;
   double mass_total_lost;
}
Sed_real_process_info;


CLASS ( Sed_process )
{
   char*            name;
   gpointer         data;
   gint             data_size;
   gboolean         active;
   gboolean         logging;
   FILE**           log_files;
   double           interval;
   GArray*          next_event;
   Sed_real_process_info* info;
   init_func        f_init;
   run_func         f_run;
   dump_func        f_dump;
   load_func        f_load;
};

#define TRACK_MASS_BALANCE TRUE

static FILE*     info_fp        = NULL;
static gboolean  info_fp_is_set = FALSE;

Sed_process sed_process_create( const char *name ,
                                size_t data_size ,
                                init_func f_init ,
                                run_func f_run   )
{
   Sed_process p;

   NEW_OBJECT( Sed_process , p );

//   p->data = g_malloc0(data_size);
   p->data = eh_malloc( data_size , NULL , __FILE__ , __LINE__ );

   p->log_files    = eh_new( FILE* , SED_MAX_LOG_FILES+1 );
   p->log_files[0] = NULL;

   p->data_size    = data_size;
   p->name         = g_strdup( name );
   p->interval     = -1;
   p->next_event   = g_array_new(TRUE,TRUE,sizeof(double));
   p->f_run        = f_run;
   p->f_init       = f_init;
   p->f_load       = NULL;
   p->f_dump       = NULL;

   p->info                   = eh_new0( Sed_real_process_info , 1 );
   p->info->mass_total_added = 0.;
   p->info->mass_total_lost  = 0.;
   p->info->error            = FALSE;

   if ( TRACK_MASS_BALANCE && !info_fp_is_set )
   {
      info_fp_is_set = TRUE;
      info_fp = eh_fopen( "mass_balance.txt" , "w" );
   }

   (*f_init)(NULL,p->data);
   (*f_run)(p->data,NULL);

   return p;
}

Sed_process sed_process_copy(Sed_process d , Sed_process s)
{
   eh_return_val_if_fail( s , NULL );

   {
      gssize i;

      if ( !d )
         d = sed_process_create( s->name , s->data_size , s->f_init , s->f_run );

      d->next_event = g_array_new(TRUE,TRUE,sizeof(double));

      for ( i=0 ; i<s->next_event->len ; i++ )
         g_array_append_val(d->next_event,g_array_index(s->next_event,double,i));

      g_memmove( d->data , s->data , s->data_size );
      g_memmove( d->log_files , 
                 s->log_files ,
                 (SED_MAX_LOG_FILES+1)*sizeof( FILE* ) );
      g_memmove( d->info , s->info , sizeof(Sed_real_process_info) );

      d->active   = s->active;
      d->logging  = s->logging;
      d->interval = s->interval;
   }

   return d;
}

Sed_process sed_process_dup( Sed_process s )
{
   return sed_process_copy( NULL , s );
}

Sed_process sed_process_destroy( Sed_process p )
{
   if ( p )
   {
      sed_process_clean( p );
      g_array_free(p->next_event,TRUE);
      eh_free( p->info );
      eh_free( p->log_files );
      eh_free( p->name );
      eh_free( p->data );
      eh_free( p );
   }

   return NULL;
}

void sed_process_clean( Sed_process p )
{
   eh_return_if_fail( p );

   p->f_run ( p->data , NULL    );
   p->f_init( NULL    , p->data );
}

double sed_process_next_event( Sed_process p )
{
   return g_array_index(p->next_event,double,p->next_event->len-1);
}

Sed_process sed_process_set_next_event( Sed_process p , double new_next_event )
{
   g_array_index( p->next_event , double , p->next_event->len-1 ) = new_next_event ;
   return p;
}

gboolean sed_process_is_on( Sed_process p , double time )
{
   gboolean is_on = FALSE;

   eh_return_val_if_fail( p , FALSE );

   {
      double last_event;
      if ( !p->active )
         is_on = FALSE;
      else if ( p->interval < 0 )
         is_on = TRUE;
      else if ( time >= sed_process_next_event(p) )
      {
         last_event = sed_process_next_event(p);
         g_array_index( p->next_event , double , 0 ) = eh_round(time,p->interval) + p->interval;
         is_on = TRUE;
      }
      else
         is_on = FALSE;
   }

   return is_on;
}

gboolean sed_process_array_run( GPtrArray *a , Sed_cube p )
{
   gssize i;
   gboolean rtn_val = TRUE;
   for (i=0;i<a->len;i++)
      rtn_val = rtn_val && sed_process_run( (Sed_process)(a->pdata)[i] , p );
   return rtn_val;
}

gboolean sed_process_run( Sed_process a , Sed_cube p )
{
   gboolean rtn_val = TRUE;
   if ( sed_process_is_on( a , sed_cube_age(p) ) )
      rtn_val = sed_process_run_now(a,p);
   return rtn_val;
}

gboolean sed_process_run_now( Sed_process a , Sed_cube p )
{
   char *log_name;
   gboolean rtn_val=TRUE;

   if ( a->logging )
      log_name = a->name;
   else
      log_name = "/dev/null";
   eh_redirect_log(log_name,DEFAULT_LOG);

   if ( TRACK_MASS_BALANCE )
   {
      double mass_before = sed_cube_mass(p) + sed_cube_mass_in_suspension(p);
      Sed_process_info info;

      info = a->f_run(a->data,p);

      a->info->mass_added        = info.mass_added;
      a->info->mass_lost         = info.mass_lost;
      a->info->error             = info.error;

      a->info->mass_before       = mass_before;
      a->info->mass_after        = sed_cube_mass(p) + sed_cube_mass_in_suspension(p);
      a->info->mass_total_added += info.mass_added;
      a->info->mass_total_lost  += info.mass_lost;

      sed_process_fprint_info( info_fp , a );

      if ( sed_process_error( a ) )
      {
         eh_warning( "A mass balance error was detected (%s)." , a->name );
         rtn_val = FALSE;
      }
   }
   else
      a->f_run(a->data,p);

   eh_reset_log(DEFAULT_LOG);

   return rtn_val;
}

void sed_process_init( Sed_process a , Eh_symbol_table symbol_table )
{
   a->f_init( symbol_table , a->data );
}

#define G_KEY_PROCESS_NAME "process name"
#define G_KEY_ACTIVE       "active"
#define G_KEY_LOGGING      "logging"
#define G_KEY_INTERVAL     "repeat interval"
#define G_KEY_NOISY        "noisy repeat interval"

GSList *sed_process_scan( Eh_key_file k , Sed_process p )
{
   GSList* p_array = NULL;

   eh_require( k );
   eh_require( p );

   if ( !eh_key_file_has_group( k , p->name ) )
   {
      Sed_process new_proc;

      g_warning( "%s: Process not found (disabling)" , p->name );

      new_proc         = sed_process_dup( p );
      new_proc->active = FALSE;
      p_array          = g_slist_append( p_array , new_proc );
   }
   else
   {
      Eh_symbol_table* t = eh_key_file_get_symbol_tables( k , p->name );
      gboolean*  active  = eh_key_file_get_bool_values( k , p->name , G_KEY_ACTIVE  );
      gboolean* logging  = eh_key_file_get_bool_values( k , p->name , G_KEY_LOGGING );
      gchar**  interval  = eh_key_file_get_all_values( k , p->name , G_KEY_INTERVAL );
      gssize len = eh_key_file_group_size( k , p->name );
      gssize i;
      Sed_process new_proc;

      for ( i=0 ; i<len ; i++ )
      {
         new_proc = sed_process_dup( p );
         new_proc->active = active[i];

         if ( active[i] )
         {
            new_proc->logging = logging[i];

            if ( g_strcasecmp( interval[i] ,"always") == 0 )
               new_proc->interval = -1;
            else
               new_proc->interval = strtotime( interval[i] );

            g_array_append_val(new_proc->next_event,new_proc->interval);

            sed_process_init( new_proc , t[i] );

            if ( new_proc->logging )
            {
               gchar* log_file_name = g_strconcat( new_proc->name , ".log" , NULL );

               new_proc->log_files[0] = eh_open_log_file( new_proc->name );
               new_proc->log_files[1] = stderr;
               new_proc->log_files[2] = NULL;

               eh_free( log_file_name );
            }
            else
               new_proc->log_files[0] = NULL;

            g_log_set_handler( new_proc->name ,
                                 G_LOG_LEVEL_MASK 
                               | G_LOG_FLAG_FATAL 
                               | G_LOG_FLAG_RECURSION ,
                               eh_logger ,
                               new_proc->log_files );
         }

         p_array = g_slist_append(p_array,new_proc);

         eh_symbol_table_destroy( t[i] );
      }

      eh_free( t       );
      eh_free( active  );
      eh_free( logging );
      g_strfreev( interval );

   }

   return p_array;
}

gssize sed_process_fprint( FILE *fp , Sed_process p )
{
   gssize n = 0;

   eh_return_val_if_fail( fp , 0 );
   eh_return_val_if_fail( p  , 0 );

   n += fprintf( fp,"name          : %s\n" , p->name     );
   n += fprintf( fp,"active        : %d\n" , p->active   );
   n += fprintf( fp,"interval      : %f\n" , p->interval );
   n += fprintf( fp,"logging       : %d\n" , p->logging  );

   return n;
}

/** Get a pointer to the user data of a Sed_process

Return a pointer to the data member of a Sed_process.  Note that
this function returns a pointer to the actual data (rather than a 
copy of the data).

@return A pointer to the data
*/
gpointer sed_process_data( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , NULL );
   return p->data;
}

/** Get the name of a process

Obtain a copy of the name of a process.  The pointer should be freed after use.

@param p A Sed_process

@return A newly-allocated string containing the name of the process
*/
gchar* sed_process_name( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , NULL );
   return g_strdup( p->name );
}

/** Get the time interval of a Sed_process

@param p A Sed_process

@return The process interval in years
*/
double sed_process_interval( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , 0. );
   return p->interval;
}

/** Is the Sed_process turned on

@param p A Sed_process

@return TRUE if the process is turned on.
*/

gboolean sed_process_is_active( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , FALSE );
   return p->active;
}

gssize sed_process_fprint_info( FILE* fp , Sed_process p )
{
   gssize n = 0;

   eh_require( fp );

   if ( fp )
   {
      Sed_real_process_info* info = p->info;
      double mass_bal = info->mass_after - ( info->mass_before - info->mass_lost + info->mass_added );
      double mass_f   = eh_safe_dbl_division( fabs(mass_bal ) ,
                                              info->mass_after );

      n += fprintf( fp , "Process name                             : %s\n" , p->name                );
      n += fprintf( fp , "Mass of cube before process (kg)         : %g\n" , info->mass_before      );
      n += fprintf( fp , "Mass of sediment added to cube (kg)      : %g\n" , info->mass_added       );
      n += fprintf( fp , "Mass of sediment lost by process (kg)    : %g\n" , info->mass_lost        );
      n += fprintf( fp , "Mass of cube after process (kg)          : %g\n" , info->mass_after       );
      n += fprintf( fp , "Mass balance (kg)                        : %g\n" , mass_bal               );
      n += fprintf( fp , "Mass fraction (-)                        : %f\n" , mass_f                 );
      n += fprintf( fp , "Mass error                               : %s\n" , mass_f>.01?"YES":"NO"  );
      n += fprintf( fp , "Total mass of sediment added (kg)        : %g\n" , info->mass_total_added );
      n += fprintf( fp , "Total mass of sediment lost (kg)         : %g\n" , info->mass_total_lost  );
      n += fprintf( fp , "Error                                    : %d\n" , info->error            );
   }

   return n;
}

gboolean sed_process_error( Sed_process p )
{
   gboolean error = FALSE;

   if ( p )
   {
      if ( p->info->error )
         error = TRUE;
      else
      {
         double mass_bal = p->info->mass_after - (   p->info->mass_before
                                                   - p->info->mass_lost
                                                   + p->info->mass_added );
         if ( eh_safe_dbl_division( fabs(mass_bal) ,
                                    p->info->mass_after  ) > .01 )
               error = TRUE;
/*
         if ( p->info->mass_added > 0 )
         {
            if ( eh_safe_dbl_division( fabs(mass_bal) , p->info->mass_added  ) > .01 )
               error = TRUE;
         }
         else
         {
            if ( eh_safe_dbl_division( fabs(mass_bal) , p->info->mass_after  ) > .01 )
               error = TRUE;
         }
*/
      }
   }

   return error;
}

