
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
   gchar*           name;
   gchar*           tag;

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

typedef struct
{
   Sed_process p;
   GList*      obj_list;
}
__Sed_process_link;

#define SED_PROCESS( ptr )      ( (Sed_process)(ptr) )
#define SED_PROCESS_LINK( ptr ) ( (__Sed_process_link*)(ptr) )

CLASS( Sed_process_queue )
{
   GList* l; //< A list of processes
};

__Sed_process_link* sed_process_link_new           ( gchar* name       ,
                                                     gssize data_size  ,
                                                     init_func f_init  ,
                                                     run_func f_run    ,
                                                     Eh_key_file file );
__Sed_process_link* sed_process_link_destroy       ( __Sed_process_link* link );
Sed_process_queue  sed_process_queue_append       ( Sed_process_queue q ,
                                                    __Sed_process_link* p );
Sed_process_queue  sed_process_queue_prepend      ( Sed_process_queue q ,
                                                    __Sed_process_link* p );
Sed_process_queue  sed_process_queue_insert       ( Sed_process_queue q     ,
                                                    __Sed_process_link* link ,
                                                   gint position );
gint               sed_process_queue_find_position( Sed_process_queue q ,
                                                    const gchar* name );
GList*             sed_process_queue_find         ( Sed_process_queue q ,
                                                    const gchar* name );

Sed_process_queue
sed_process_queue_new( void )
{
   Sed_process_queue q;

   NEW_OBJECT( Sed_process_queue , q );

   q->l = NULL;

   return q;
}

__Sed_process_link*
sed_process_link_new( gchar* name       , gssize data_size ,
                      init_func f_init  , run_func f_run   ,
                      Eh_key_file file )
{
   __Sed_process_link* link = eh_new( __Sed_process_link , 1 );

   eh_require( name );
   eh_require( data_size>0 );
   eh_require( f_init );
   eh_require( f_run );
   eh_require( file );

   if ( link )
   {
      link->p        = sed_process_create( name , data_size , f_init , f_run );
      link->obj_list = sed_process_scan  ( file , link->p );
   }

   return link;
}

Sed_process_queue
sed_process_queue_destroy( Sed_process_queue q )
{
   if ( q )
   {
      GList* link;

      for ( link=q->l ; link ; link=link->next )
      {
         sed_process_link_destroy( link->data );
      }

      g_list_free( q->l );

      eh_free( q );
   }

   return NULL;
}

__Sed_process_link*
sed_process_link_destroy( __Sed_process_link* link )
{
   if ( link )
   {
      g_list_foreach( link->obj_list , (GFunc)sed_process_destroy , NULL );
      sed_process_destroy( link->p );
      eh_free( link );
   }
   return NULL;
}

Sed_process_queue
sed_process_queue_scan( Sed_process_queue q , Eh_key_file file )
{
   if ( q && file )
   {
      GList* link;
      __Sed_process_link* data;

      for ( link=q->l ; link ; link=link->next )
      {
         data = link->data;

         data->obj_list = g_list_concat( data->obj_list , sed_process_scan( file , data->p ) );
      }

   }

   return q;
}

Sed_process_queue
sed_process_queue_append( Sed_process_queue q , __Sed_process_link* p )
{
   if ( q && p )
   {
      q->l = g_list_append( q->l , p );
   }

   return q;
}

Sed_process_queue
sed_process_queue_prepend( Sed_process_queue q , __Sed_process_link* p )
{
   if ( q && p )
   {
      q->l = g_list_prepend( q->l , p );
   }

   return q;
}

Sed_process_queue
sed_process_queue_insert( Sed_process_queue q , __Sed_process_link* link , gint position )
{
   if ( q && link )
   {
      q->l = g_list_insert( q->l , link , position );
   }

   return q;
}

gint
sed_process_queue_find_position( Sed_process_queue q , const gchar* name )
{
   gint position = -1;

   if ( q && name )
   {
      GList* link = sed_process_queue_find( q , name );

      position = g_list_position( q->l , link );
   }

   return position;
}

gint __sed_process_compare_name( const __Sed_process_link* link , const gchar* name )
{
   return g_ascii_strcasecmp( link->p->name , name );
}

GList*
sed_process_queue_find( Sed_process_queue q , const gchar* name )
{
   GList* link = NULL;

   if ( q && name )
   {
      link = g_list_find_custom( q->l , name , __sed_process_compare_name );
   }

   return link;
}

Sed_process_queue
sed_process_queue_push( Sed_process_queue q , Sed_process_init_t init )
{
   if ( q )
   {
      __Sed_process_link* new_link = eh_new( __Sed_process_link , 1 );

      new_link->p = sed_process_create( init.name , init.data_size , init.init_f , init.run_f );
      new_link->obj_list = NULL;

      sed_process_queue_append( q , new_link );
   }

   return q;
}

Sed_process
sed_process_queue_find_nth_obj( Sed_process_queue q , const gchar* name , gssize n )
{
   Sed_process obj = NULL;

   if ( n>=0 && name && q )
   {
      GList* link = sed_process_queue_find( q , name );
      obj = g_list_nth_data( SED_PROCESS_LINK(link->data)->obj_list , n );
   }

   return obj;
}

gpointer*
sed_process_queue_obj_data( Sed_process_queue q , const char* name )
{
   gpointer* data = NULL;

   if ( q && name )
   {
      GList* link     = sed_process_queue_find( q , name );
      GList* obj_list = SED_PROCESS_LINK(link->data)->obj_list;
      GList* this_obj = obj_list;
      gssize len      = g_list_length( obj_list );
      gssize i;

      data = eh_new( gpointer , len+1 );

      for ( i=0,this_obj=obj_list ; this_obj ; i++,this_obj=this_obj->next )
         data[i] = SED_PROCESS(this_obj->data)->data;
      data[i] = NULL;
   }

   return data;
}

Sed_process_queue
sed_process_queue_delete( Sed_process_queue q , const gchar* name )
{
   if ( q && name )
   {
      GList* link = sed_process_queue_find( q , name );

      link->data = sed_process_link_destroy( link->data );

      q->l = g_list_delete_link( q->l , link );
   }

   return q;
}

Sed_process_queue
sed_process_queue_run( Sed_process_queue q , Sed_cube p )
{
   if ( q && p )
   {
      GList* link;

      for ( link=q->l ; link ; link=link->next )
      {
         g_list_foreach( SED_PROCESS_LINK(link->data)->obj_list , (GFunc)sed_process_run , p );
      }
   }

   return q;
}

Sed_process_queue
sed_process_queue_run_at_end( Sed_process_queue q , Sed_cube p )
{
   if ( q && p )
   {
      GList* link;

      for ( link=q->l ; link ; link=link->next )
      {
         g_list_foreach( SED_PROCESS_LINK(link->data)->obj_list , (GFunc)sed_process_run_at_end , p );
      }
   }

   return q;
}

Sed_process_queue
sed_process_queue_run_process_now( Sed_process_queue q , const gchar* name , Sed_cube cube )
{
   if ( q )
   {
      GList* link = sed_process_queue_find( q , name );

      g_list_foreach( SED_PROCESS_LINK(link->data)->obj_list , sed_process_run_now , cube );
   }

   return q;
}

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
//   p->data = eh_malloc( data_size , NULL , __FILE__ , __LINE__ );
   p->data = eh_new( gchar , data_size );

   p->log_files    = eh_new( FILE* , SED_MAX_LOG_FILES+1 );
   p->log_files[0] = NULL;

   p->data_size    = data_size;
   p->name         = g_strdup( name );
   p->tag          = NULL;
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

      d->tag = g_strdup( s->tag );

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

      eh_free( p->info      );
      eh_free( p->log_files );
      eh_free( p->name      );
      eh_free( p->tag       );
      eh_free( p->data      );
      eh_free( p            );
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
      if ( !p->active || sed_process_interval_is_at_end(p) )
         is_on = FALSE;
      else if ( sed_process_interval_is_always(p) )
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

gboolean
sed_process_run( Sed_process a , Sed_cube p )
{
   gboolean rtn_val = TRUE;
   if ( sed_process_is_on( a , sed_cube_age(p) ) )
      rtn_val = sed_process_run_now(a,p);
   return rtn_val;
}

gboolean
sed_process_run_at_end( Sed_process a , Sed_cube p )
{
   gboolean rtn_val = TRUE;
   if ( sed_process_is_active(a) && sed_process_interval_is_at_end(a) )
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

   if ( g_getenv("SED_TRACK_MASS") )
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

#define SED_KEY_TAG         "process tag"
#define SED_KEY_ACTIVE      "active"
#define SED_KEY_LOGGING     "logging"
#define SED_KEY_INTERVAL    "repeat interval"

GList *sed_process_scan( Eh_key_file k , Sed_process p )
{
   GList* p_array = NULL;

   eh_require( k );
   eh_require( p );

   if ( !eh_key_file_has_group( k , p->name ) )
   {
      Sed_process new_proc;

      g_warning( "%s: Process not found (disabling)" , p->name );

      new_proc         = sed_process_dup( p );
      new_proc->active = FALSE;
      p_array          = g_list_append( p_array , new_proc );
   }
   else
   {
      Eh_symbol_table* t = eh_key_file_get_symbol_tables( k , p->name );
      gchar**     tag    = eh_key_file_get_str_values   ( k , p->name , SED_KEY_TAG  );
      gboolean*  active  = eh_key_file_get_bool_values  ( k , p->name , SED_KEY_ACTIVE  );
      gboolean* logging  = eh_key_file_get_bool_values  ( k , p->name , SED_KEY_LOGGING );
      gchar**  interval  = eh_key_file_get_all_values   ( k , p->name , SED_KEY_INTERVAL );
      gssize len = eh_key_file_group_size( k , p->name );
      gssize i;
      Sed_process new_proc;

      for ( i=0 ; i<len ; i++ )
      {
         new_proc = sed_process_dup( p );
         new_proc->active = active[i];
         new_proc->tag    = tag[i];

         if ( active[i] )
         {
            new_proc->logging = logging[i];

            if      ( g_strcasecmp( interval[i] , "always" ) == 0 )
               new_proc->interval = -1;
            else if ( g_strcasecmp( interval[i] , "at-end" ) == 0 )
               new_proc->interval = -2;
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

         p_array = g_list_append(p_array,new_proc);

         eh_symbol_table_destroy( t[i] );
      }

      eh_free( t       );
      eh_free( active  );
      eh_free( logging );
      eh_free( tag     );
      g_strfreev( interval );

   }

   return p_array;
}

gssize sed_process_fprint( FILE *fp , Sed_process p )
{
   gssize n = 0;

   eh_return_val_if_fail( fp , 0 );
   eh_return_val_if_fail( p  , 0 );

   if ( fp && p )
   {
      const gchar* active_str  = (p->active)?"yes":"no";
      const gchar* logging_str = (p->logging)?"yes":"no";
      const gchar* tag_str     = (p->tag)?p->tag:"(null)";

      n += fprintf( fp,"[Process info]\n" );
      n += fprintf( fp,"name          = %s\n" , p->name     );
      n += fprintf( fp,"tag           = %s\n" , tag_str     );
      n += fprintf( fp,"active        = %s\n" , active_str  );
      n += fprintf( fp,"interval      = %f\n" , p->interval );
      n += fprintf( fp,"logging       = %s\n" , logging_str );
   }

   return n;
}

gssize sed_process_queue_fprint( FILE* fp , Sed_process_queue q )
{
   gssize n = 0;
   if ( q )
   {
      __Sed_process_link* link;
      GList* this_link;
      GList* this_obj;

      n += fprintf( fp , "[Process queue info]\n" );
      n += fprintf( fp , "No. of processes = %d\n" , sed_process_queue_size(q)       );
      n += fprintf( fp , "No. active       = %d\n" , sed_process_queue_n_active(q)   );
      n += fprintf( fp , "No. inactive     = %d\n" , sed_process_queue_n_inactive(q) );
      n += fprintf( fp , "No. absent       = %d\n" , sed_process_queue_n_absent(q)   );

      for ( this_link=q->l ; this_link ; this_link=this_link->next )
      {
         link = this_link->data;
         for ( this_obj=link->obj_list ; this_obj ; this_obj=this_obj->next )
            n += sed_process_fprint( fp , this_obj->data );
      }
   }
   return n;
}

gssize
sed_process_queue_size( Sed_process_queue q )
{
   return g_list_length( q->l );
}

gssize
sed_process_queue_n_active( Sed_process_queue q )
{
   gssize n = 0;
   if ( q )
   {
      GList* list;
      GList* obj_list;
      GList* this_obj;
      gboolean is_active;

      /* Cycle through each process in the queue */
      for ( list=q->l ; list ; list=list->next )
      {
         obj_list = SED_PROCESS_LINK(list->data)->obj_list;

         is_active = FALSE;

         /* Cycle through each object of this process */
         for ( this_obj=obj_list ; this_obj && !is_active ; this_obj=this_obj->next )
            is_active = sed_process_is_active( this_obj->data );

         if ( is_active )
            n += 1;
      }
   }

   return n;
}

gssize
sed_process_queue_n_absent( Sed_process_queue q )
{
   gssize n = 0;

   if ( q )
   {
      GList* list;
      GList* obj_list;

      /* Cycle through each process in the queue */
      for ( list=q->l ; list ; list=list->next )
      {
         obj_list = SED_PROCESS_LINK(list->data)->obj_list;

         /* If there is no object list, the process was absent from the input file. */
         if ( !obj_list )
            n += 1;
      }
   }

   return n;
}

gssize
sed_process_queue_n_inactive( Sed_process_queue q )
{
   gssize n = 0;

   if ( q )
   {
      gssize n_active = sed_process_queue_n_active( q );
      gssize n_absent = sed_process_queue_n_absent( q );
      gssize len      = sed_process_queue_size    ( q );
   
      n = len - n_active - n_active;

      eh_require( n>=0   );
      eh_require( n<=len );
   }

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

/** Compare the names of two processes

\param a    A Sed_process
\param b    A Sed_process

\return     TRUE if the process names are the same, FALSE otherwise.
*/
gboolean
sed_process_name_is_same( Sed_process a , Sed_process b )
{
   gboolean same_name = FALSE;

   if ( a && b )
   {
      same_name = (g_ascii_strcasecmp( a->name , b->name )==0)?TRUE:FALSE;
   }

   return same_name;
}

/** Get the time interval of a Sed_process

@param p A Sed_process

@return The process interval in years
*/
double
sed_process_interval( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , 0. );
   return p->interval;
}

/** Is the time interval set to 'always'

If the interval of a process is 'always', it will be run for
every sedflux time step.

\param p A Sed_process

\return Returns true if the interval is 'always'
*/
gboolean
sed_process_interval_is_always( Sed_process p )
{
   eh_return_val_if_fail( p , FALSE );
   return p->interval>-1.5 && p->interval<-.5;
}

/** Is the time interval set to 'at-end'

If the interval of a process is 'at-end', it will only
be run for at the end of an epoch.

\param p A Sed_process

\return Returns true if the interval is 'at-end'
*/
gboolean
sed_process_interval_is_at_end( Sed_process p )
{
   eh_return_val_if_fail( p , FALSE );
   return p->interval>-2.5 && p->interval<-1.5;
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
      }
   }

   return error;
}

int
sed_process_queue_check( Sed_process_queue q , const gchar* p_name )
{
   int error = 0;
   GList* link;

   link = sed_process_queue_find( q , p_name );

   if ( link )
   {
      gssize i, len;
      gboolean is_active;
      Sed_process this_proc;
      GList* list;

      list = SED_PROCESS_LINK(link->data)->obj_list;

      /* Check if there are any instances of this process */
      len = g_list_length( list );
      if ( len>1 )
         error |= SED_ERROR_MULTIPLE_PROCS;

      /* Check if the time step of each instance is set to always */
      for ( i=0,is_active=FALSE ; i<len ; i++ )
      {
         this_proc = g_list_nth_data( list , i );

         is_active |= sed_process_is_active(this_proc);
         if ( sed_process_interval(this_proc) > 0 )
            error |= SED_ERROR_NOT_ALWAYS;
      }
      
      /* Check if any of the instances are turned on */
      if ( !is_active )
         error |= SED_ERROR_INACTIVE;
   }
   else
      error |= SED_ERROR_PROC_ABSENT;

   return error;
}

int
sed_process_queue_check_family( Sed_process_queue q ,
                                const gchar* parent ,
                                const gchar* child  ,
                                ... )
{
   int error = 0;
   gboolean parent_active, child_active;
   double parent_dt, child_dt;

   eh_require( q      );
   eh_require( parent );
   eh_require( child  );

   /* There should be a parent. */
   if ( parent )
   {
      GList* parent_link = sed_process_queue_find( q , parent );

      if ( parent_link )
      {
         __Sed_process_link* parent_proc = parent_link->data;

         /* There should only be one parent. */
         if ( g_list_length( parent_proc->obj_list )>1 )
            error |= SED_ERROR_MULTIPLE_PARENTS;

         /* There should be an active parent. */
         if ( g_list_length( parent_proc->obj_list )==0 )
            error |= SED_ERROR_ABSENT_PARENT;
         else
         {
            parent_active = sed_process_is_active( parent_proc->obj_list->data );
            parent_dt     = sed_process_interval ( parent_proc->obj_list->data );

            if ( !parent_active )
               error |= SED_ERROR_INACTIVE_PARENT;
         }
      }
      else
         error |= SED_ERROR_ABSENT_PARENT;
   }
   else
      error |= SED_ERROR_ABSENT_PARENT;

   if ( child )
   {
      __Sed_process_link* child_proc;
      GList* this_child;
      va_list ap;

      va_start( ap , child );

      error |= SED_ERROR_INACTIVE_CHILDREN;

      /* Check each of the child processes. */
      for ( ; child ; child=va_arg( ap , const gchar* ) )
      {
         child_proc = sed_process_queue_find( q , child )->data;

         /* Check each object of this child process. */
         for ( this_child=child_proc->obj_list ; this_child ; this_child=this_child->next )
         {
            child_active = sed_process_is_active( this_child->data );
            child_dt     = sed_process_interval ( this_child->data );

            /* If there is one active child, clear the inactive children error. */
            if ( child_active )
               error &= ~SED_ERROR_INACTIVE_CHILDREN;

            /* The dt of parent and child should match */
            if ( child_active && parent_active )
            {
               if ( !eh_compare_dbl( parent_dt , child_dt , 1e-12 ) )
                  error |= SED_ERROR_DT_MISMATCH;
            }
         }
      }

      va_end( ap );
   }

   /* If the children are inactive, then there is no error if the parent is inactive/absent. */
   if ( error&SED_ERROR_INACTIVE_CHILDREN )
      error &= ~(SED_ERROR_INACTIVE_PARENT|SED_ERROR_ABSENT_PARENT);

   return error;
}

Sed_process sed_process_set_active( Sed_process p , gboolean val )
{
   if ( p )
   {
      p->active = val;
   }
   return p;
}

Sed_process sed_process_activate( Sed_process p )
{
   if ( p )
   {
      p->active = TRUE;
   }
   return p;
}

Sed_process sed_process_deactivate( Sed_process p )
{
   if ( p )
   {
      p->active = FALSE;
   }
   return p;
}

Sed_process_queue sed_process_queue_activate( Sed_process_queue q ,
                                              const gchar* name )
{
   return sed_process_queue_set_active( q , name , TRUE );
}

Sed_process_queue sed_process_queue_deactivate( Sed_process_queue q ,
                                                const gchar* name )
{
   return sed_process_queue_set_active( q , name , FALSE );
}

Sed_process_queue sed_process_queue_set_active( Sed_process_queue q ,
                                                const gchar* name   ,
                                                gboolean val )
{
   if ( q )
   {
      if ( g_ascii_strncasecmp( name , "<all>" , 5 )==0 )
      {
         /* set all processes */
         GList* list;

         for ( list=q->l ; list ; list=list->next )
         {
            sed_process_queue_set_active( q ,
                                          SED_PROCESS_LINK( list->data )->p->name ,
                                          val );
         }
      }
      else if ( g_ascii_strncasecmp( name , "<just>" , 6 )==0 )
      {
         /* set just this process */
         sed_process_queue_set_active( q , "<all>" , !val );
         sed_process_queue_set_active( q , name+6  ,  val );
      }
      else
      {
         /* It's just a name */
         GList* list = sed_process_queue_find( q , name );

         if ( list )
         {
            if ( val )
               g_list_foreach( SED_PROCESS_LINK(list->data)->obj_list , sed_process_activate , NULL );
            else
               g_list_foreach( SED_PROCESS_LINK(list->data)->obj_list , sed_process_deactivate , NULL );
         }
      }
   }

   return q;
}

