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

CLASS( Sed_epoch )
{
   char*    name;
   double   number;
   double   duration;
   double   time_step;
   char*    filename;
};

CLASS( Sed_epoch_queue )
{
   GList* l;
};

Sed_epoch
sed_epoch_new( void )
{
   Sed_epoch e;

   NEW_OBJECT( Sed_epoch , e );

   e->name      = NULL;
   e->number    = -1;
   e->duration  = -1;
   e->time_step = -1;
   e->filename  = NULL;

   return e;
}

#define SED_KEY_EPOCH_NUMBER       "number"
#define SED_KEY_EPOCH_DURATION     "duration"
#define SED_KEY_EPOCH_TIME_STEP    "time step"
#define SED_KEY_EPOCH_FILE         "process file"

static gchar* required_labels[] =
{
   SED_KEY_EPOCH_NUMBER       ,
   SED_KEY_EPOCH_DURATION     ,
   SED_KEY_EPOCH_TIME_STEP    ,
   SED_KEY_EPOCH_FILE         ,
   NULL
};

Sed_epoch
sed_epoch_new_from_table( Eh_symbol_table t )
{
   Sed_epoch e = NULL;

   if ( t )
   {
      e = sed_epoch_new( );

      if ( eh_symbol_table_has_labels( t , required_labels ) )
      {

         sed_epoch_set_number   ( e , eh_symbol_table_dbl_value ( t , SED_KEY_EPOCH_NUMBER    ) );
         sed_epoch_set_duration ( e , eh_symbol_table_time_value( t , SED_KEY_EPOCH_DURATION  ) );
         sed_epoch_set_time_step( e , eh_symbol_table_time_value( t , SED_KEY_EPOCH_TIME_STEP ) );
         sed_epoch_set_filename ( e , eh_symbol_table_value     ( t , SED_KEY_EPOCH_FILE      ) );

         if ( !eh_try_open( sed_epoch_filename( e ) ) )
         {
            eh_error( "Could not open Sed_epoch file (%s)" , sed_epoch_filename(e) );
         }

      }
      else
         eh_error( "Required labels are missing in epoch group" );
   }

   return e;
}

Sed_epoch_queue
sed_epoch_queue_new( const gchar* file , GError** error )
{
   Sed_epoch_queue e_list = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file )
   {
      GError*     tmp_err  = NULL;
      Eh_key_file key_file = NULL;

      key_file = eh_key_file_scan( file , &tmp_err );

      if ( key_file )
      {
         Eh_symbol_table*  tables     = eh_key_file_get_symbol_tables( key_file , "epoch" );
         Eh_symbol_table*  this_table;

         NEW_OBJECT( Sed_epoch_queue , e_list );
         e_list->l = NULL;

         for ( this_table=tables ; *this_table ; this_table++ )
         {
            e_list->l = g_list_append( e_list->l , sed_epoch_new_from_table(*this_table) );
            eh_symbol_table_destroy( *this_table );
         }

         e_list->l = g_list_sort( e_list->l , (GCompareFunc)sed_epoch_number_cmp );

         eh_free( tables );
      }
      else
         g_propagate_error( error , tmp_err );

      eh_key_file_destroy( key_file );
   }

   return e_list;
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
      e->number    = -1;
      e->duration  = -1;
      e->time_step = -1;

      eh_free( e->name     );
      eh_free( e->filename );
      eh_free( e           );
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
epoch_set_name( Sed_epoch e , gchar* name )
{
   if ( e )
   {
      if ( e->name )
         eh_free( e->name );
      e->name = g_strdup( name );
   }

   return e;
}

Sed_epoch
sed_epoch_set_number( Sed_epoch e , gssize n )
{
   if ( e )
   {
      e->number = n;
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
   {
      e->time_step = time_step;
   }
   return e;
}

Sed_epoch
sed_epoch_set_filename( Sed_epoch e , gchar* filename )
{
   if ( e )
   {
      if ( e->filename )
         eh_free( e->filename );

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
sed_epoch_duration( Sed_epoch e )
{
   eh_return_val_if_fail( e , -1 );

   return e->duration;
}

double
sed_epoch_time_step( Sed_epoch e )
{
   eh_return_val_if_fail( e , -1 );

   return e->time_step;
}

const gchar*
sed_epoch_filename( Sed_epoch e )
{
   eh_return_val_if_fail( e , NULL );

   return e->filename;
}


gssize
sed_epoch_queue_length ( Sed_epoch_queue q )
{
   gssize len = 0;
   if ( q )
   {
      len = g_list_length( q->l );
   }
   return len;
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

gssize
sed_epoch_fprint( FILE* fp , Sed_epoch e )
{
   gssize n = 0;

   if ( e )
   {
      n += fprintf( fp , "[Epoch Info]\n" );

      n += fprintf( fp , "Id           = %f\n" , e->number    );
      n += fprintf( fp , "Duration     = %f\n" , e->duration  );

      n += fprintf( fp , "Time step    = %f  " , e->time_step );
      n += fprintf( fp , "# NOTE: A river file may override this time step!\n" );

      n += fprintf( fp , "Process file = %s\n" , e->filename  );


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

