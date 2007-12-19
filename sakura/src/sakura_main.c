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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include "sakura.h"
#include "sakura_local.h"
#include <sed/sed_sedflux.h>
#include <utils/utils.h>

// Command line arguments
static gint     _verbose     = 0;
static gboolean _reset_bathy = FALSE;
static gboolean _version     = FALSE;
static gboolean _debug       = FALSE;
static gdouble  _day         = 1.;
static gdouble  _angle       = 14.;
static gchar*   _in_file     = NULL;
static gchar*   _out_file    = NULL;
static gchar*   _bathy_file  = NULL;
static gchar*   _flood_file  = NULL;
static gchar*   _data_file   = NULL;
static gint*    _data_id     = NULL;
static gint     _data_int    = 1;

gboolean parse_data_list( const gchar* name , const gchar* value , gpointer data , GError** error );

static GOptionEntry entries[] =
{
   { "in-file"    , 'i' , 0 , G_OPTION_ARG_FILENAME , &_in_file     , "Initialization file" , "<file>" } ,
   { "out-file"   , 'o' , 0 , G_OPTION_ARG_FILENAME , &_out_file    , "Output file"         , "<file>" } ,
   { "bathy-file" , 'b' , 0 , G_OPTION_ARG_FILENAME , &_bathy_file  , "Bathymetry file"     , "<file>" } ,
   { "flood-file" , 'f' , 0 , G_OPTION_ARG_FILENAME , &_flood_file  , "Flood file"          , "<file>" } ,
   { "data-file"  , 'd' , 0 , G_OPTION_ARG_FILENAME , &_data_file   , "Data file"           , "<file>" } ,
   { "out-data"   , 'D' , 0 , G_OPTION_ARG_CALLBACK , parse_data_list , "List of data to watch" , "[var1[,var2[...]]]" } ,
   { "out-int"    , 'I' , 0 , G_OPTION_ARG_INT      , &_data_int    , "Data output interval (-)" , "INT" } ,
   { "angle"      , 'a' , 0 , G_OPTION_ARG_DOUBLE   , &_angle       , "Spreading angle (deg)"     , "DEG"    } ,
   { "reset"      ,  0  , 0 , G_OPTION_ARG_NONE     , &_reset_bathy , "Reset bathymetry with every flood"     , NULL    } ,
   { "verbose"    , 'V' , 0 , G_OPTION_ARG_INT      , &_verbose     , "Verbosity level"     , "n"      } ,
   { "version"    , 'v' , 0 , G_OPTION_ARG_NONE     , &_version     , "Version number"      , NULL     } ,
   { "debug"      , 'b' , 0 , G_OPTION_ARG_NONE     , &_debug       , "Write debug messages", NULL     } ,
   { NULL }
};

static const gchar* _DATA_VAL_KEYS[] = 
{
   "velocity" , "height" , "concentration" , NULL
};

gboolean
parse_data_list( const gchar* name , const gchar* value , gpointer data , GError** error )
{
   gboolean success = FALSE;
   gint*    data_id = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( name && value )
   {
      GError* tmp_err   = NULL;
      gchar** data_list = g_strsplit( value , "," , 0 );

      if ( !data_list )
      {
         g_set_error( &tmp_err ,
                      G_OPTION_ERROR ,
                      G_OPTION_ERROR_FAILED ,
                      "Failed to parse comma-separated list of data values to monitor" );
      }
      else
      {
         gchar** key;
         gint    id;
         gint    i;


         data_id = eh_new( gint , g_strv_length( data_list )+1 );

         for ( key=data_list,i=0 ; *key && !tmp_err ; key++,i++ )
         {
            id         = eh_strv_find( _DATA_VAL_KEYS , *key );
            data_id[i] = id;

            if ( id<0 )
               g_set_error( &tmp_err ,
                            G_OPTION_ERROR ,
                            G_OPTION_ERROR_FAILED ,
                            "Invalid data key (%s)" , *key );
         }
         data_id[i] = -1;
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         eh_free( data_id );
         data_id = NULL;
         success = FALSE;
      }
      else
         success = TRUE;
   }

   _data_id = data_id;

   return success;
}

void
sakura_run_flood( Sakura_bathy_st* b    ,
                  Sakura_flood_st* f    ,
                  Sakura_sediment_st* s ,
                  Sakura_const_st* c    ,
                  double** deposit_in_m )
{
   //double** deposition = eh_new_2( double , s->n_grains , b->len );

   //if ( deposition )
   {
      gint     n, i;
      double** deposit = NULL;
      gint     n_grains;
      gint     len;

      deposit = sakura_wrapper( b , f , s , c , &n_grains , &len );

      eh_require( n_grains==s->n_grains );
      eh_require( len==b->len );

//      sakura_update_bathy_data( b , deposition , erosion , s->n_grains );

      for ( n=0 ; n<s->n_grains ; n++ )
         for ( i=0 ; i<b->len ; i++ )
            deposit_in_m[n][i] = deposit[n][i];

      eh_free_2( deposit );
   }
}

gint
main(int argc,char *argv[])
{
   GError*  error = NULL;
   gchar*   program_name;
   gboolean mode_1d;

   g_thread_init( NULL );
   eh_init_glib();

   {
      GOptionContext* context = g_option_context_new( "Run hyperpycnal flow model." );

      g_option_context_add_main_entries( context , entries , NULL );

      if ( !g_option_context_parse( context , &argc , &argv , &error ) )
         eh_error( "Error parsing command line arguments: %s" , error->message );
   }

   _day            *= S_SECONDS_PER_DAY;

   if ( _version )
   {
      eh_fprint_version_info( stdout , "sakura" , 0 , 9 , 0 );
      eh_exit(0);
   }

   if ( _debug )
      g_setenv( "SAKURA_DEBUG" , "TRUE" , TRUE );

   program_name = g_path_get_basename( argv[0] );
   if ( strcasecmp( program_name , "sakura")==0 )
   {
      _angle   = 0.;
      mode_1d = TRUE;
   }

   if ( _verbose )
   {
      if ( mode_1d ) eh_info( "Operating in 1D mode (ignoring width information)." );
      else           eh_info( "Operating in 1.5D mode." );

      eh_info( "Duration of flow (days)   : %f" , _day*S_DAYS_PER_SECOND );
      eh_info( "Spreading angle (degrees) : %f" , _angle );
   }

   if ( !error )
   {
      gint i;
      Sakura_param_st*    param         = NULL;
      Sakura_bathy_st*    bathy_data    = NULL;
      Sakura_bathy_st*    bathy_data_0  = NULL;
      Sakura_flood_st**   flood_data    = NULL;
      Sakura_const_st*    const_data    = NULL;
      Sakura_sediment_st* sediment_data = NULL;
      Eh_dbl_grid         deposit;
      Eh_dbl_grid         total_deposit;
      const double        spreading_angle = tan(_angle*G_PI/180.);

      if (    ( param        = sakura_scan_parameter_file( _in_file            , &error ) )==NULL
           || ( flood_data   = sakura_scan_flood_file    ( _flood_file , param , &error ) )==NULL
           || ( bathy_data_0 = sakura_scan_bathy_file    ( _bathy_file , param , &error ) )==NULL )
         eh_error( "%s" , error->message );

      bathy_data    = sakura_copy_bathy_data         ( NULL       , bathy_data_0 );
      const_data    = sakura_set_constant_data       ( param      , bathy_data   );
      const_data    = sakura_set_constant_output_data( const_data , _data_file , _data_id , _data_int );
      sediment_data = sakura_set_sediment_data       ( param );

      deposit       = eh_grid_new( double , sediment_data->n_grains , bathy_data->len );
      total_deposit = eh_grid_new( double , sediment_data->n_grains , bathy_data->len );

      for ( i=0 ; flood_data[i] ; i++ )
      { /* Run each day of the flood */

         /* The width starts at the river width for each day */
         sakura_set_width( bathy_data , flood_data[i]->width , spreading_angle );

         sakura_run_flood( bathy_data , flood_data[i] , sediment_data , const_data , eh_dbl_grid_data(deposit) );

         eh_dbl_grid_add( total_deposit , deposit );

         if ( _data_file   ) sakura_write_data     ( _data_file  , deposit      );
         if ( _reset_bathy ) sakura_copy_bathy_data( bathy_data , bathy_data_0 );
      }

      sakura_write_output( _out_file , bathy_data , eh_dbl_grid_data(total_deposit) , sediment_data->n_grains );

      eh_grid_destroy( total_deposit , TRUE );
      eh_grid_destroy( deposit       , TRUE );
   }

   return EXIT_SUCCESS;
}



