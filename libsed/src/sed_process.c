
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
#include "sed_signal.h"

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

   GTimer* timer;
   double  secs;
   gulong  u_secs;
}
Sed_real_process_info;

CLASS ( Sed_process )
{
   gchar*           name;
   gchar*           tag;

   gpointer         data;

   gboolean         active;
   double           start;
   double           stop;

   gint             run_count;
   gboolean         is_set;

   Sed_process*     parent;
   Sed_process*     child;

   gboolean         is_child;

   gboolean         logging;
   FILE**           log_files;
   double           interval;
   GArray*          next_event;
   Sed_real_process_info* info;

   init_func        f_init;
   run_func         f_run;
   destroy_func     f_destroy;

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
                                                     init_func f_init  ,
                                                     run_func f_run    ,
                                                     destroy_func f_destroy    ,
                                                     Eh_key_file file  ,
                                                     GError** error );
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

GQuark
sed_process_error_quark( void )
{
   return g_quark_from_static_string( "sed-process-error-quark" );
}

Sed_process_queue
sed_process_queue_new( void )
{
   Sed_process_queue q;

   NEW_OBJECT( Sed_process_queue , q );

   q->l = NULL;

   return q;
}

Sed_process_queue
sed_process_queue_dup( Sed_process_queue s )
{
   return sed_process_queue_copy( NULL , s );
}

Sed_process_queue
sed_process_queue_copy( Sed_process_queue d , Sed_process_queue s )
{
   if ( s )
   {
      GList* l;

      if ( d ) d = sed_process_queue_destroy( d );

      d = sed_process_queue_new( );

      for ( l=s->l ; l ; l=l->next )
         d->l = g_list_append( d->l , sed_process_dup( l->data ) );
      
   }
   return d;
}

__Sed_process_link*
sed_process_link_new( gchar*       name      ,
                      init_func    f_init    ,
                      run_func     f_run     ,
                      destroy_func f_destroy ,
                      Eh_key_file  file      ,
                      GError**     error )
{
   __Sed_process_link* link = eh_new( __Sed_process_link , 1 );

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   eh_require( name );
   eh_require( file );

   if ( link )
   {
      GError* tmp_e = NULL;

      /* Create an empty process that will be used as a template for future objects */
      link->p        = sed_process_create( name , f_init , f_run , f_destroy );

      /* Scan object based on the template */
      link->obj_list = sed_process_scan  ( file , link->p , &tmp_e );

      if ( tmp_e && tmp_e->code == SED_PROC_ERROR_NOT_FOUND )
         g_clear_error( &tmp_e );

      if ( tmp_e )
      {
         link = sed_process_link_destroy( link );
         g_propagate_error( error , tmp_e );
      }
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
         sed_process_link_destroy( link->data );

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
      g_list_free   ( link->obj_list );
      sed_process_destroy( link->p );
      eh_free( link );
   }
   return NULL;
}

Sed_process_queue
sed_process_queue_init( const gchar*       file       ,
                        Sed_process_init_t p_list[]   ,
                        Sed_process_family p_family[] ,
                        Sed_process_check  p_check[]  ,
                        GError**           error )
{
   Sed_process_queue q = NULL;

   eh_require( file   );
   eh_require( p_list );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file && p_list )
   {
      GError*     tmp_err  = NULL;
      Eh_key_file key_file = NULL;

      key_file = eh_key_file_scan( file , &tmp_err );

      if ( key_file )
      {
         gssize i;

         q = sed_process_queue_new( );
         for ( i=0 ; p_list[i].name ; i++ )
            sed_process_queue_push( q , p_list[i] );

         sed_process_queue_scan( q , key_file , &tmp_err );

         if ( !tmp_err && p_family ) sed_process_queue_set_families( q , p_family , &tmp_err );
         if ( !tmp_err && p_check  ) sed_process_queue_validate    ( q , p_check  , &tmp_err );

/*
         if ( !tmp_err )
         {
            if ( on_procs )
            {
               gchar** name;
               sed_process_queue_deactivate( q , "<all>" );

               for ( name=on_procs ; *name ; name++ )
                  sed_process_queue_activate( q , *name );
            }

            //if ( hook_func ) hook_func( q );
            if ( !tmp_err && p_check  ) sed_process_queue_validate( q , p_check , &tmp_err );
            if ( !tmp_err && p_family ) sed_process_queue_set_families( q , p_family , &tmp_err );
         }
*/
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = sed_process_queue_destroy( q );
      }
   }
   return q;
}

Sed_process_queue
sed_process_queue_set_families( Sed_process_queue q , Sed_process_family f[] , GError** error )
{
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( q )
   {
      GError*     tmp_err = NULL;
      gint        i;
      Sed_process p;
      Sed_process c;
      gchar**     err_s = NULL;

      for ( i=0 ; f[i].parent ; i++ )
      {
         p = sed_process_queue_find_nth_obj( q , f[i].parent , 0 );
         c = sed_process_queue_find_nth_obj( q , f[i].child  , 0 );

         if ( !sed_process_is_active(p) ) p=NULL;
         if ( !sed_process_is_active(c) ) c=NULL;

         if ( c && !p )
         {
            eh_strv_append( &err_s , g_strdup_printf( "Child process has a missing or inactive parent" ) );
            eh_strv_append( &err_s , g_strdup_printf( "Missing %s needed by %s" , f[i].parent , f[i].child ) );
         }
         else if ( c && p )
            sed_process_append_child( p , c );
      }
      if ( tmp_err )
      {
         eh_set_error_strv( &tmp_err ,
                            SED_PROC_ERROR ,
                            SED_PROC_ERROR_MISSING_PARENT ,
                            err_s );
         g_propagate_error( error , tmp_err );
         q = NULL;
      }

      g_strfreev( err_s );
   }

   return q;
}

Sed_process
sed_process_child( Sed_process p , const gchar* child_s )
{
   Sed_process found_child = NULL;

   if ( p && p->child && child_s )
   {
      Sed_process* c = NULL;

      for ( c=p->child ; !found_child && *c ; c++ )
      {
         if ( g_ascii_strcasecmp( (*c)->name , child_s )==0 )
            found_child = *c;
      }

   }

   return found_child;
}

Sed_process
sed_process_append_child( Sed_process p , Sed_process c )
{
   eh_require( p );
   eh_require( c );

   if ( p && c )
   {
      eh_strv_append( (gchar***)(&(p->child )) , (gchar*)c );
      eh_strv_append( (gchar***)(&(c->parent)) , (gchar*)p );

      c->is_child = TRUE;
   }
   return p;
}

gboolean
sed_process_is_parent( Sed_process p )
{
   gboolean is_parent = FALSE;

   if ( p ) is_parent = !(p->is_child);

   return is_parent;
}

Sed_process_queue
sed_process_queue_scan( Sed_process_queue q , Eh_key_file file , GError** error )
{
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( q && file )
   {
      GList*              link;
      GList*              new_p;
      __Sed_process_link* data;
      GError*             tmp_e = NULL;

      for ( link=q->l ; !tmp_e && link ; link=link->next )
      {
         data = link->data;

         new_p = sed_process_scan( file , data->p , &tmp_e );

         if ( tmp_e && tmp_e->code == SED_PROC_ERROR_NOT_FOUND )
            g_clear_error( &tmp_e );

         if ( !tmp_e )
            data->obj_list = g_list_concat( data->obj_list , new_p );
      }

      if ( tmp_e )
      {
         g_propagate_error( error , tmp_e );
         q = NULL;
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
      link = g_list_find_custom( q->l , name , (GCompareFunc)__sed_process_compare_name );
   }

   return link;
}

Sed_process_queue
sed_process_queue_push( Sed_process_queue q , Sed_process_init_t init )
{
   if ( q )
   {
      __Sed_process_link* new_link = eh_new( __Sed_process_link , 1 );

      new_link->p = sed_process_create( init.name , init.init_f , init.run_f , init.destroy_f );
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
      if ( link ) obj = g_list_nth_data( SED_PROCESS_LINK(link->data)->obj_list , n );
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
sed_process_queue_run_until( Sed_process_queue q , Sed_cube p , double t_total )
{
   eh_require( q );
   eh_require( p );

   if ( q && p )
   {
      double         t; /* Time in years */
//      Eh_status_bar* bar = eh_status_bar_new( &t , &t_total );

      for ( t  = sed_cube_time_step( p ) ;
               t < t_total
            && !sed_signal_is_pending( SED_SIG_QUIT )
            && !sed_signal_is_pending( SED_SIG_NEXT ) ;
            t += sed_cube_time_step( p ) )
      {
         sed_process_queue_run( q , p );

         if ( sed_signal_is_pending( SED_SIG_DUMP ) )
         {
            sed_process_queue_run_process_now( q , "data dump" , p );
            sed_signal_reset( SED_SIG_DUMP );
         }

         sed_cube_increment_age( p );

         //fprintf( stderr , "%7g (%3.0g%%)\r" , t , t/t_total*100 );
      }

      if ( sed_signal_is_pending( SED_SIG_NEXT ) )
      {
         sed_cube_set_age( p , t_total );
         sed_signal_reset( SED_SIG_NEXT );
      }

//      eh_status_bar_destroy( bar );
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

      if ( link && link->data )
         g_list_foreach( SED_PROCESS_LINK(link->data)->obj_list , (GFunc)sed_process_run_now , cube );
   }

   return q;
}

#define TRACK_MASS_BALANCE TRUE

static FILE*     info_fp        = NULL;
static gboolean  info_fp_is_set = FALSE;

Sed_process sed_process_create( const char*  name   ,
                                init_func    f_init ,
                                run_func     f_run  ,
                                destroy_func f_destroy )
{
   Sed_process p;

   NEW_OBJECT( Sed_process , p );

//   p->data = eh_new( gchar , data_size );
   p->data         = NULL;

   p->log_files    = eh_new( FILE* , SED_MAX_LOG_FILES+1 );
   p->log_files[0] = NULL;

//   p->data_size    = data_size;

   p->active       = FALSE;
   p->start        = -G_MAXDOUBLE;
   p->stop         =  G_MAXDOUBLE;

   p->run_count    = 0;
   p->is_set       = FALSE;

   p->parent       = NULL;
   p->child        = NULL;

   p->is_child     = FALSE;

   p->name         = g_strdup( name );
   p->tag          = NULL;
   p->interval     = -1;
   p->next_event   = g_array_new(TRUE,TRUE,sizeof(double));

   p->f_init       = f_init;
   p->f_run        = f_run;
   p->f_destroy    = f_destroy;
   p->f_load       = NULL;
   p->f_dump       = NULL;

   p->info                   = eh_new0( Sed_real_process_info , 1 );
   p->info->mass_total_added = 0.;
   p->info->mass_total_lost  = 0.;
   p->info->error            = FALSE;

   p->info->timer            = g_timer_new();
   p->info->secs             = 0.;
   p->info->u_secs           = 0;

   if ( g_getenv("SED_TRACK_MASS")  && !info_fp_is_set )
   {
      info_fp_is_set = TRUE;
      info_fp        = eh_fopen( "mass_balance.txt" , "w" );
   }

/*
   (*f_init)(NULL,p->data);
   (*f_run)(p->data,NULL);
*/

   return p;
}

Sed_process sed_process_copy(Sed_process d , Sed_process s)
{
   eh_return_val_if_fail( s , NULL );

   eh_require( s->data==NULL );

   {
      gssize i;

      if ( !d )
         d = sed_process_create( s->name , s->f_init , s->f_run , s->f_destroy );

      d->next_event = g_array_new(TRUE,TRUE,sizeof(double));

      for ( i=0 ; i<s->next_event->len ; i++ )
         g_array_append_val(d->next_event,g_array_index(s->next_event,double,i));

      d->tag = g_strdup( s->tag );

      //g_memmove( d->data , s->data , s->data_size );
      d->data = s->data;

      g_memmove( d->log_files , 
                 s->log_files ,
                 (SED_MAX_LOG_FILES+1)*sizeof( FILE* ) );
      g_memmove( d->info , s->info , sizeof(Sed_real_process_info) );

      d->info->timer  = g_timer_new();
      d->info->secs   = s->info->secs;
      d->info->u_secs = s->info->u_secs;

      d->active   = s->active;
      d->start    = s->start;
      d->stop     = s->stop;

      d->run_count= s->run_count;
      d->is_set   = s->is_set;

      d->parent   = s->parent;
      d->child    = s->child;
      d->is_child = s->is_child;

      d->logging  = s->logging;
      d->interval = s->interval;
   }

   return d;
}

Sed_process
sed_process_dup( Sed_process s )
{
   return sed_process_copy( NULL , s );
}

Sed_process
sed_process_destroy( Sed_process p )
{
   if ( p )
   {
      if ( p->f_destroy ) p->f_destroy( p );

      g_array_free(p->next_event,TRUE);

      g_timer_destroy( p->info->timer );

      eh_free( p->child     );
      eh_free( p->parent    );
      eh_free( p->info      );
      eh_free( p->log_files );
      eh_free( p->name      );
      eh_free( p->tag       );
      eh_free( p            );
   }

   return NULL;
}

void
sed_process_clean( Sed_process p )
{
   eh_return_if_fail( p );

   if ( p->f_destroy ) p->f_destroy( p );

//   p->f_run ( p->data , NULL    );
//   p->f_init( NULL    , p->data );
}

double
sed_process_next_event( Sed_process p )
{
   return g_array_index(p->next_event,double,p->next_event->len-1);
}

Sed_process
sed_process_set_next_event( Sed_process p , double new_next_event )
{
   g_array_index( p->next_event , double , p->next_event->len-1 ) = new_next_event ;
   return p;
}

gboolean
sed_process_is_on( Sed_process p , double time )
{
   gboolean is_on = FALSE;

   eh_return_val_if_fail( p , FALSE );

   if ( p )
   {
      double last_event;
      if (    !p->active
           || sed_process_interval_is_at_end(p) 
           || time < p->start
           || time > p->stop )
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

gboolean
sed_process_array_run( GPtrArray *a , Sed_cube p )
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
   if ( sed_process_is_on( a , sed_cube_age(p) ) && sed_process_is_parent(a) )
      rtn_val = sed_process_run_now(a,p);
   return rtn_val;
}

gboolean
sed_process_run_at_end( Sed_process a , Sed_cube p )
{
   gboolean rtn_val = TRUE;
   if ( sed_process_is_active(a) && sed_process_interval_is_at_end(a) && sed_process_is_parent(a) )
      rtn_val = sed_process_run_now(a,p);
   return rtn_val;
}

gboolean
sed_process_run_now( Sed_process a , Sed_cube p )
{
   gboolean rtn_val = TRUE;

   if ( a && a->f_run && a->is_set )
   {
      gchar*           log_name;
      Sed_process_info info;
      gulong           u_secs = 0;

      g_timer_start( a->info->timer );

      if ( a->logging ) log_name = a->name;
      else              log_name = "/dev/null";

      eh_redirect_log( log_name , DEFAULT_LOG );

      fprintf( stderr , "%7g years [ Running process: %-25s]\r" , sed_cube_age_in_years(p) , a->name );

      if ( g_getenv("SED_TRACK_MASS") )
      {
         double mass_before = sed_cube_mass(p) + sed_cube_mass_in_suspension(p);

         info = a->f_run(a,p);

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
      {
         info = a->f_run( a , p );

         a->info->mass_total_added += info.mass_added;
         a->info->mass_total_lost  += info.mass_lost;
      }

      eh_reset_log(DEFAULT_LOG);

      a->info->secs   += g_timer_elapsed( a->info->timer , NULL );
      a->info->u_secs += u_secs;

      a->run_count++;
   }

   return rtn_val;
}

void
sed_process_init( Sed_process p , Eh_symbol_table t , GError** error )
{
   if ( p->f_init ) p->f_init( p , t , error );
   p->is_set = TRUE;
}

#define SED_KEY_TAG         "process tag"
#define SED_KEY_ACTIVE      "active"
#define SED_KEY_LOGGING     "logging"
#define SED_KEY_INTERVAL    "repeat interval"

static gchar* sed_process_req_labels[] =
{
   SED_KEY_ACTIVE   ,
   SED_KEY_LOGGING  ,
   SED_KEY_INTERVAL ,
   NULL
};

GList*
sed_process_scan( Eh_key_file k , Sed_process p , GError** error )
{
   GList*  p_array = NULL;
   GError* tmp_err = NULL;

   eh_require( k );
   eh_require( p );

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );
   if ( !eh_key_file_has_group( k , p->name ) )
   {
      Sed_process new_proc;

      g_set_error( &tmp_err , SED_PROC_ERROR , SED_PROC_ERROR_NOT_FOUND , "Process not found: %s" , p->name );

      new_proc         = sed_process_dup( p );
      new_proc->active = FALSE;
      p_array          = g_list_append( p_array , new_proc );
   }
   else
   {
      //gboolean*  active  = eh_key_file_get_bool_values  ( k , p->name , SED_KEY_ACTIVE  );
      Eh_symbol_table* t        = eh_key_file_get_symbol_tables( k , p->name );
      gchar**          tag      = eh_key_file_get_str_values   ( k , p->name , SED_KEY_TAG  );
      gchar**          active_s = eh_key_file_get_str_values   ( k , p->name , SED_KEY_ACTIVE  );
      gboolean*        logging  = eh_key_file_get_bool_values  ( k , p->name , SED_KEY_LOGGING );
      gchar**          interval = eh_key_file_get_all_values   ( k , p->name , SED_KEY_INTERVAL );
      gssize           len      = eh_key_file_group_size       ( k , p->name );
      gssize           i;
      Sed_process      new_proc;
      gboolean         is_active;

      for ( i=0 ; !tmp_err && i<len ; i++ )
         eh_symbol_table_require_labels( t[i] , sed_process_req_labels , &tmp_err );

      for ( i=0 ; !tmp_err && i<len ; i++ )
      {
         new_proc      = sed_process_dup( p );
         new_proc->tag = tag[i];

         if ( eh_str_is_boolean( active_s[i] ) )
         {
            is_active   = eh_str_to_boolean( active_s[i] , &tmp_err );

            if ( is_active )
            {
               new_proc->start = -G_MAXDOUBLE;
               new_proc->stop  =  G_MAXDOUBLE;
            }
         }
         else
         {
            double* active_time = eh_str_to_time_range( active_s[i] , &tmp_err );

            is_active       = TRUE;

            new_proc->start = active_time[0];
            new_proc->stop  = active_time[1];

            eh_free( active_time );
         }

         new_proc->active = is_active;

         if ( !tmp_err && is_active )
         {
            new_proc->logging = logging[i];

            if      ( g_ascii_strcasecmp( interval[i] , "always" ) == 0 )
               new_proc->interval = -1;
            else if ( g_ascii_strcasecmp( interval[i] , "at-end" ) == 0 )
               new_proc->interval = -2;
            else
               new_proc->interval = eh_str_to_time_in_years( interval[i] , &tmp_err );

            g_array_append_val( new_proc->next_event , new_proc->interval );

            if ( !tmp_err ) sed_process_init( new_proc , t[i] , &tmp_err );

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
      }

      if ( tmp_err )
      {
         g_list_foreach( p_array , (GFunc)sed_process_destroy , NULL );
         g_list_free   ( p_array );
         p_array = NULL;
      }

      for ( i=0 ; i<len ; i++ )
         eh_symbol_table_destroy( t[i] );

      eh_free( t       );
      eh_free( active_s );
      eh_free( logging );
      eh_free( tag     );
      g_strfreev( interval );
   }

   if ( tmp_err )
   {
      gchar* err_s = g_strdup_printf( "Process scan error (%s): %s" , p->name , tmp_err->message );
      eh_free( tmp_err->message );
      tmp_err->message = err_s;
      g_propagate_error( error , tmp_err );
   }

   return p_array;
}

gssize
sed_process_fprint( FILE *fp , Sed_process p )
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
      if ( p->active )
      {
         if ( eh_compare_dbl( p->start , -G_MAXDOUBLE , 1e-6 ) )
            n += fprintf( fp,"start         = %s\n" , "The begining" );
         else
            n += fprintf( fp,"start         = %f\n" , p->start    );

         if ( eh_compare_dbl( p->stop  ,  G_MAXDOUBLE , 1e-6 ) )
            n += fprintf( fp,"stop          = %s\n" , "The end" );
         else
            n += fprintf( fp,"stop          = %f\n" , p->stop     );
      }
      else
      {
         n += fprintf( fp,"start         = N/A\n" );
         n += fprintf( fp,"stop          = N/A\n" );
      }

      n += fprintf( fp,"run count     = %d\n" , p->run_count );
      n += fprintf( fp,"is set        = %d\n" , p->is_set    );

      if ( sed_process_interval_is_always(p) )
         n += fprintf( fp,"interval      = %s\n" , "Always" );
      else
         n += fprintf( fp,"interval      = %f\n" , p->interval );

      n += fprintf( fp,"logging       = %s\n" , logging_str );
   }

   return n;
}

gssize
sed_process_queue_summary( FILE* fp , Sed_process_queue q )
{
   gssize n = 0;
   if ( q )
   {
      __Sed_process_link* link;
      GList* this_link;
      GList* this_obj;

      n += fprintf( fp , "             Name | Mass Added | Mass Removed | Time\n" );

      for ( this_link=q->l ; this_link ; this_link=this_link->next )
      {
         link = this_link->data;
         for ( this_obj=link->obj_list ; this_obj ; this_obj=this_obj->next )
            n += sed_process_summary( fp , this_obj->data );
      }
   }
   return n;
}

gssize
sed_process_queue_fprint( FILE* fp , Sed_process_queue q )
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
   
      n = len - n_active - n_absent;

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
gpointer
sed_process_data( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , NULL );
   return p->data;
}

void
sed_process_provide( Sed_process p , GQuark key , gpointer data )
{
   if ( p ) g_dataset_id_set_data_full( p , key , data , NULL );
}

void
sed_process_withhold( Sed_process p , GQuark key )
{
   if ( p ) g_dataset_id_remove_data( p , key );
}

gpointer
sed_process_use( Sed_process p , GQuark key )
{
   gpointer data = NULL;

   if ( p ) data = g_dataset_id_get_data( p , key );

   return data;
}

/** Get the name of a process

Obtain a copy of the name of a process.  The pointer should be freed after use.

@param p A Sed_process

@return A newly-allocated string containing the name of the process
*/
gchar*
sed_process_name( Sed_process p )
{
   eh_return_val_if_fail( p!=NULL , NULL );
   return g_strdup( p->name );
}

gint
sed_process_run_count( Sed_process p )
{
   eh_return_val_if_fail( p , 0 );
   return p->run_count;
}

gboolean
sed_process_is_set( Sed_process p )
{
   eh_return_val_if_fail( p , FALSE );
   return p->is_set;
}
gpointer
sed_process_user_data( Sed_process p )
{
   eh_return_val_if_fail( p , NULL );
   return p->data;
}

gpointer
sed_process_malloc_user_data( Sed_process p , gssize n_bytes )
{
   gpointer data = NULL;

   if ( p )
   {
      if ( !p->data )
         p->data = eh_new( gchar , n_bytes );
      data = p->data;
   }

   return data;
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
gboolean
sed_process_is_active( Sed_process p )
{
   gboolean is_active = FALSE;

   if ( p )
      is_active = p->active;
   
   return is_active;
}

gssize
sed_process_summary( FILE* fp , Sed_process p )
{
   gint n = 0;
   if ( fp && p )
   {
      double t = p->info->secs + p->info->u_secs/1.e6;
      gchar* t_str = eh_render_time_str( t );

      n += fprintf( fp , "%18s | %10.3g | %10.3g | %s\n" ,
                    p->name ,
                    p->info->mass_total_added ,
                    p->info->mass_total_lost ,
                    t_str );
   }
   return n;
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

gboolean
sed_process_error( Sed_process p )
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
sed_process_queue_check_item( Sed_process_queue q , const gchar* p_name )
{
   int    flag = 0;
   GList* item;

   item = sed_process_queue_find( q , p_name );

   if ( item )
   {
      GList* list = SED_PROCESS_LINK(item->data)->obj_list;

      /* Check if there are any instances of this process */
      if ( g_list_length( list )==1 )
         flag |= SED_PROC_UNIQUE;

      if ( list )
      {
         gboolean is_active = FALSE;
         gboolean is_always = TRUE;
         GList*   l;

         /* Check if the time step of each instance is set to always */
         for ( l=list ; l ; l=l->next )
         {
            is_active |= sed_process_is_active( l->data );
            is_always &= sed_process_interval_is_always( l->data );
         }
      
         /* Check if any of the instances are turned on */
         if ( is_active )
            flag |= SED_PROC_ACTIVE;

         /* Check if all of the instances are always */
         if ( is_always )
            flag |= SED_PROC_ALWAYS;
      }
   }

   return flag;
}

gint
sed_process_queue_check_family( Sed_process_queue q ,
                                const gchar* parent_name ,
                                const gchar* child_name  )
{
   gint flag = 0;

   eh_require( q           );
   eh_require( parent_name );

   /* There should be a parent. */
   if ( parent_name && child_name )
   {
      gint   parent_flag = sed_process_queue_check_item( q , parent_name );
      gint   child_flag  = sed_process_queue_check_item( q , child_name );
      double parent_dt;
      double child_dt;

      flag |= parent_flag;

      if ( parent_flag&SED_PROC_UNIQUE ) flag |= SED_PROC_UNIQUE_PARENT;
      if ( parent_flag&SED_PROC_ACTIVE ) flag |= SED_PROC_ACTIVE_PARENT;
      if ( child_flag &SED_PROC_UNIQUE ) flag |= SED_PROC_UNIQUE_CHILD;
      if ( child_flag &SED_PROC_ACTIVE ) flag |= SED_PROC_ACTIVE_CHILD;

      child_dt  = sed_process_queue_item_interval( q , child_name  );
      parent_dt = sed_process_queue_item_interval( q , parent_name );

      if ( eh_compare_dbl( parent_dt , child_dt , 1e-12 ) )
         flag |= SED_PROC_SAME_INTERVAL;
   }
   else if ( parent_name )
      flag |= sed_process_queue_check_item( q , parent_name );
   else
      eh_require_not_reached();

   return flag;
}

double
sed_process_queue_item_interval( Sed_process_queue q , const gchar* name )
{
   double interval = G_MAXDOUBLE;

   if ( q && name )
   {
      GList* item       = sed_process_queue_find( q , name );
      GList* obj_list   = SED_PROCESS_LINK(item->data)->obj_list;
      gboolean is_found = TRUE;
      GList* l;

      for ( l=obj_list ; l && !is_found ; l=l->next )
      {
         if ( sed_process_is_active( l->data ) )
         {
            interval = sed_process_interval( l->data );
            is_found = TRUE;
         }
      }
   }

   return interval;
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
               g_list_foreach( SED_PROCESS_LINK(list->data)->obj_list , (GFunc)sed_process_activate , NULL );
            else
               g_list_foreach( SED_PROCESS_LINK(list->data)->obj_list , (GFunc)sed_process_deactivate , NULL );
         }
      }
   }

   return q;
}

gchar*
sed_process_flag_str( gint flag )
{
   gchar* flag_s = NULL;

   if ( flag )
   {
      gchar* s = NULL;

      switch ( flag )
      {
         case SED_PROC_UNIQUE:        s = "Process is unique"; break;
         case SED_PROC_ACTIVE:        s = "Process is active"; break;
         case SED_PROC_ACTIVE_PARENT: s = "Parent process is active"; break;
         case SED_PROC_ACTIVE_CHILD:  s = "Child process is active"; break;
         case SED_PROC_UNIQUE_PARENT: s = "Parent process is unique"; break;
         case SED_PROC_UNIQUE_CHILD:  s = "Child process is unique"; break;
         case SED_PROC_SAME_INTERVAL: s = "Parent and child process have same interval"; break;
      }

      flag_s = g_strdup( s );
   }

   return flag_s;
}

gchar*
sed_process_render_flag_str( gint flag , const gchar* pre_s , const gchar* name )
{
   gchar* err_s = NULL;

   if ( flag!=0 )
   {
      gchar** all_strs = NULL;

      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_UNIQUE        ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_ACTIVE        ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_ACTIVE_PARENT ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_ACTIVE_CHILD  ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_UNIQUE_PARENT ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_UNIQUE_CHILD  ) );
      eh_strv_append( &all_strs , sed_process_flag_str( flag&SED_PROC_SAME_INTERVAL ) );

      {
         gchar* s;
         gint   i;

         for ( i=0 ; all_strs[i] ; i++ )
         {
            s = all_strs[i];

            if   ( pre_s ) all_strs[i] = g_strjoin( ": " , pre_s , all_strs[i] , name , NULL );
            else           all_strs[i] = g_strjoin( ": " ,         all_strs[i] , name , NULL );

            eh_free( s );
         }
      }
         
      err_s = g_strjoinv( "\n" , all_strs );

      g_strfreev( all_strs );
   }

   return err_s;
}

gboolean
sed_process_queue_validate( Sed_process_queue q , Sed_process_check check[] , GError** error )
{
   gboolean is_valid;

   eh_require( q );
   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );
   
   if ( q && check )
   {
      gint    i;
      gchar*  err_s      = NULL;
      gchar** err_s_list = NULL;
      gint    on_flags;
      gint    req_flags;

      for ( i=0 ; check[i].parent ; i++ )
      {
         req_flags = check[i].flags;
         on_flags  = sed_process_queue_check_family( q , check[i].parent , check[i].child )
                   & req_flags;

         if ( on_flags != req_flags )
         {
            err_s = sed_process_render_flag_str( on_flags^req_flags , "Failed requirement" , check[i].parent );
            eh_strv_append( &err_s_list , err_s );
         }
      }

      if ( err_s_list!=NULL )
      {
         GError* tmp_err = NULL;
         gchar*  err_msg = g_strjoinv( "\n" , err_s_list );

         g_set_error( &tmp_err , SED_PROC_ERROR , SED_PROC_ERROR_BAD_INIT_FILE , err_msg );

         g_propagate_error( error , tmp_err );

         eh_free   ( err_msg    );
         g_strfreev( err_s_list );

         is_valid = FALSE;
      }
      else
         is_valid = TRUE;
   }

   return is_valid;
}

