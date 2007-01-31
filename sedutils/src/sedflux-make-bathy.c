#include <glib.h>
#include "sed_sedflux.h"

static gchar*   out_file   = NULL;
static double   slope      = .02;
static gint     n_x        = 100;
static gint     n_y        = 100;
static double   x_res      = 1000.;
static double   y_res      = 1000.;
static gboolean big_endian    = FALSE;
static gboolean little_endian = FALSE;
static gboolean version    = FALSE;
static gint     verbosity  = 0;
static gboolean debug      = FALSE;

GOptionEntry entries[] =
{
   { "out-file"      , 'o' , 0 , G_OPTION_ARG_FILENAME , &out_file      , "Output bathymetry file"        , "<file>" } ,
   { "slope"         , 's' , 0 , G_OPTION_ARG_DOUBLE   , &slope         , "Bathymetric slope"             , "SLOPE"  } ,
   { "nx"            , 'x' , 0 , G_OPTION_ARG_INT      , &n_x           , "Number of cell in x-dimension" , "NX"     } ,
   { "ny"            , 'y' , 0 , G_OPTION_ARG_INT      , &n_y           , "Number of cell in y-dimension" , "NY"     } ,
   { "x-res"         , 'X' , 0 , G_OPTION_ARG_DOUBLE   , &x_res         , "Cell size in x-dimension (m)"  , "DX"     } ,
   { "y-res"         , 'Y' , 0 , G_OPTION_ARG_DOUBLE   , &y_res         , "Cell size in y-dimension (m)"  , "DY"     } ,
   { "big-endian"    , 'b' , 0 , G_OPTION_ARG_NONE     , &big_endian    , "Byte order is big endian"      , NULL     } ,
   { "little-endian" , 'l' , 0 , G_OPTION_ARG_NONE     , &little_endian , "Byte order is little endian"   , NULL     } ,
   { "verbose"       , 'V' , 0 , G_OPTION_ARG_INT      , &verbosity     , "Verbosity level"               , "n"      } ,
   { "version"       , 'v' , 0 , G_OPTION_ARG_NONE     , &version       , "Version number"                , NULL     } ,
   { "debug"         , 'd' , 0 , G_OPTION_ARG_NONE     , &debug         , "Write debugging messages"      , NULL     } ,
   { NULL }
};

int main( int argc , char* argv[] )
{
   GOptionContext* context = g_option_context_new( "Create a bathymetry file for sedflux-3d" );
   GError*         error   = NULL;
   FILE*           fp_out  = stdout;
   gint            byte_order = G_BYTE_ORDER;

   g_option_context_add_main_entries( context , entries , NULL );

   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   if ( version )
      eh_fprint_version_info( stdout , "sedflux-make-bathy" , 0 , 9 , 0 ), exit(0);

   if ( debug )
      g_setenv( "SEDFLUX_MAKE_BATHY_DEBUG" , "TRUE" , TRUE );

   eh_set_verbosity_level( verbosity );

   if ( out_file )
      fp_out = eh_fopen_error( out_file , "w" , &error );

   if ( !fp_out )
      eh_error( "Error opening file: %s" , error->message );

   if      ( big_endian && little_endian )
      eh_error( "Byte order must be big endian or little endian" );
   else if ( !big_endian && !little_endian )
      byte_order = G_BYTE_ORDER;
   else if ( big_endian )
      byte_order = G_BIG_ENDIAN;
   else if ( little_endian )
      byte_order = G_LITTLE_ENDIAN;

   eh_message( "x-resolution (m)  : %f" , x_res );
   eh_message( "y-resolution (m)  : %f" , y_res );
   eh_message( "x-dimension       : %d" , n_x   );
   eh_message( "y-dimension       : %d" , n_y   );
   eh_message( "Bathymetric slope : %f" , slope   );
   eh_message( "Byte order        : %d" , byte_order==G_BIG_ENDIAN?"Big-endian":"Little-endian" );
   eh_message( "Format            : %s" , "<32-bit-integer=N> <<64-bit-double*N> ... EOF>" );

   {
      gint i, j;
      Eh_dbl_grid g = eh_grid_new( double , n_x , n_y );

      for ( i=0 ; i<n_x ; i++ )
         for ( j=0 ; j<n_y ; j++ )
            eh_dbl_grid_set_val( g , i , j , -slope*j*y_res );

      if ( byte_order == G_BYTE_ORDER )
      {
         fwrite( &n_y                  , sizeof(gint32) , 1       , fp_out );
         fwrite( eh_grid_data_start(g) , sizeof(double) , n_x*n_y , fp_out );
      }
      else
      {
         eh_fwrite_int32_swap( &n_y                  , sizeof(gint32) , 1       , fp_out );
         eh_fwrite_dbl_swap  ( eh_grid_data_start(g) , sizeof(double) , n_x*n_y , fp_out );
      }

      eh_grid_destroy( g , TRUE );
   }

   fclose( fp_out );
}

