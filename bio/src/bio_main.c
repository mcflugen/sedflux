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

#include <math.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "bio.h"

double** bio_scan_column_file( const gchar* file , gint* n_grains , gint* n_layers , GError** error );
gint     bio_print_output    ( const gchar* file , double** col , gint n_grains , gint n_layers , GError** error );

// Command line arguments
static double   layer_dz  = .01;
static double   k         = 1.;
static double   duration  = 1.;
static gchar*   in_file   = NULL;
static gchar*   out_file  = NULL;
static gboolean version   = FALSE;
static gboolean verbose   = FALSE;

static GOptionEntry entries[] =
{
   { "dz"         , 'h' , 0 , G_OPTION_ARG_DOUBLE   , &layer_dz   , "Layer thickness"        , "x"      } ,
   { "diff-coef"  , 'k' , 0 , G_OPTION_ARG_DOUBLE   , &k          , "Diffusion coefficient"  , "k"      } ,
   { "duration"   , 'T' , 0 , G_OPTION_ARG_DOUBLE   , &duration   , "Duration of model run"  , "DAYS"   } ,
   { "in-file"    , 'i' , 0 , G_OPTION_ARG_FILENAME , &in_file    , "Input file"             , "<file>" } ,
   { "out-file"   , 'o' , 0 , G_OPTION_ARG_FILENAME , &out_file   , "Output file"            , "<file>" } ,
   { "verbose"    , 'V' , 0 , G_OPTION_ARG_NONE     , &verbose    , "Verbose"                , NULL     } ,
   { "version"    , 'v' , 0 , G_OPTION_ARG_NONE     , &version    , "Version number"         , NULL     } ,
   { NULL }
};

gint
main( int argc, char *argv[])
{
   GOptionContext* context;
   GError*         error   = NULL;

   eh_init_glib();

   context = g_option_context_new( "Run bioturbation model." );

   g_option_context_add_main_entries( context , entries , NULL );

   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   if ( version )
   {
      eh_fprint_version_info( stdout , BIO_PROGRAM_NAME , BIO_MAJOR_VERSION , BIO_MINOR_VERSION , BIO_MICRO_VERSION );
      eh_exit( 0 );
   }
   else
   {
      gint     n_grains;
      gint     n_layers;
      double** col = bio_scan_column_file( in_file , &n_grains , &n_layers , &error );

      if ( verbose )
      {
         eh_info( "Diffusion coefficient (m^2/d) : %f" , k                 );
         eh_info( "Diffusion depth (m)           : %f" , n_layers*layer_dz );
         eh_info( "Duration (d)                  : %f" , duration          );
      }

      k        *= S_DAYS_PER_SECOND;
      duration *= S_SECONDS_PER_DAY;

      if ( !error )
      {
         bioturbate( col , n_grains , n_layers , layer_dz , k , duration );

         bio_print_output( out_file , col , n_grains , n_layers , &error );

         eh_free_2( col );
      }

      if ( error ) eh_error( "bio: %s" , error->message );
   }

   return 0;
}

double**
bio_scan_column_file( const gchar* file , gint* n_grains , gint* n_layers , GError** error )
{
   double** col = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file )
   {
      col = eh_dlm_read_swap( file , ";," , n_grains , n_layers , error );
   }

   return col;
}

gint
bio_print_output( const gchar* file , double** col , gint n_grains , gint n_layers , GError** error )
{
   gint n = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );

   if ( col && col[0] )
   {
      n = eh_dlm_print_swap( file , ";" , col , n_grains , n_layers , error );
   }

   return n;
}

