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
#include "utils.h"
#include "sed_sedflux.h"

void write_dlm_data( Sed_hydro* a );

static gchar*   in_file    = NULL;
static gchar*   out_file   = NULL;
static double   dt         = 365.;
static double   fraction   = .9;
static gint     n_recs     = 1;
static gint     buf_len    = 365;
static gint     events     = 5;
static gint     start      = 0;
static gboolean just_events = FALSE;
static gboolean version    = FALSE;
static gint     verbosity  = 5;
static gboolean debug      = FALSE;
static gboolean info       = FALSE;
static gint     to         = G_BYTE_ORDER;
static gint     from       = G_BYTE_ORDER;

gboolean parse_byte_order( const gchar* name , const gchar* value , gpointer data , GError** error );

GOptionEntry entries[] =
{
   { "in-file"     , 'i' , 0 , G_OPTION_ARG_FILENAME , &in_file   , "Input file"           , "<file>" } ,
   { "dt"          , 'T' , 0 , G_OPTION_ARG_DOUBLE   , &dt        , "Duration (days)"      , "TIME" } ,
   { "fraction"    , 'f' , 0 , G_OPTION_ARG_DOUBLE   , &fraction  , "Fraction of sediment" , "FRAC" } ,
   { "n-recs"      , 'N' , 0 , G_OPTION_ARG_INT      , &n_recs    , "Number of records"    , "N" } ,
   { "buffer"      , 'l' , 0 , G_OPTION_ARG_INT      , &buf_len   , "Buffer length"        , "LEN" } ,
   { "events"      , 'n' , 0 , G_OPTION_ARG_INT      , &events    , "Number of events"     , "N" } ,
   { "start-rec"   , 's' , 0 , G_OPTION_ARG_INT      , &start     , "Start record"         , "N" } ,
   { "just-events" , 'e' , 0 , G_OPTION_ARG_NONE     , &just_events , "Don't include non-floods"       , NULL } ,
   { "verbose"     , 'V' , 0 , G_OPTION_ARG_INT      , &verbosity , "Verbosity level"      , "n" } ,
   { "version"     , 'v' , 0 , G_OPTION_ARG_NONE     , &version   , "Version number"       , NULL } ,
   { "debug"       , 'd' , 0 , G_OPTION_ARG_NONE     , &debug     , "Write debug messages" , NULL } ,

   { "info"        ,  0  , 0 , G_OPTION_ARG_NONE     , &info           , "Print file info"       , NULL } ,

   { "swap"        ,  0  , 0 , G_OPTION_ARG_CALLBACK , parse_byte_order , "Swap byte order"      , NULL } ,
   { "to"          ,  0  , 0 , G_OPTION_ARG_CALLBACK , parse_byte_order , "Destination byte order" , "[big|little]" } ,
   { "from"        ,  0  , 0 , G_OPTION_ARG_CALLBACK , parse_byte_order , "Source byte order"      , "[big|little]" } ,
   { NULL }
};

gboolean
parse_byte_order( const gchar* name , const gchar* value , gpointer data , GError** error )
{
   gboolean success = FALSE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( name && value )
   {
      GError* tmp_err = NULL;
      gint*   byte_order;

      if (    g_ascii_strcasecmp( name , "--SWAP" )!=0
           && g_ascii_strcasecmp( name , "--TO"   )!=0
           && g_ascii_strcasecmp( name , "--FROM" )!=0 )
      {
            success = FALSE;
            g_set_error( &tmp_err ,
                         G_OPTION_ERROR ,
                         G_OPTION_ERROR_FAILED , 
                         "Invalid option name (%s)" , name );
      }
      else if ( g_ascii_strcasecmp( name , "--SWAP" )==0 )
      {
         from = G_BYTE_ORDER;

         if ( G_BYTE_ORDER==G_BIG_ENDIAN )
            to = G_LITTLE_ENDIAN;
         else
            to = G_BIG_ENDIAN;
      }
      else
      {
         if      ( g_ascii_strcasecmp( name , "--TO"   )==0 )
            byte_order = &to;
         else if ( g_ascii_strcasecmp( name , "--FROM" )==0 )
            byte_order = &from;

         if      (    g_ascii_strcasecmp( value , "BIG"        )==0
                   || g_ascii_strcasecmp( value , "BIG-ENDIAN" )==0 )
         {
            success = TRUE;
            *byte_order = G_BIG_ENDIAN;
         }
         else if (    g_ascii_strcasecmp( value , "LITTLE"        )==0
                   || g_ascii_strcasecmp( value , "LITTLE-ENDIAN" )==0 )
         {
            success = TRUE;
            *byte_order = G_LITTLE_ENDIAN;
         }
         else
         {
            success = FALSE;
            g_set_error( &tmp_err ,
                         G_OPTION_ERROR ,
                         G_OPTION_ERROR_BAD_VALUE , 
                         "Unknown byte order (%s): must be either BIG-ENDIAN or LITTLE-ENDIAN" , value );
         }
      }

      if ( tmp_err )
         g_propagate_error( error , tmp_err );
   }

   return success;
}

#define byte_order_s( o ) ( (o==G_BIG_ENDIAN)?"big endian":"little endian" )

int
main(int argc,char *argv[])
{
   GOptionContext* context = g_option_context_new( "Read a HydroTrend river file" );
   GError*         error   = NULL;
   FILE*           fp_out  = stdout;
   FILE*           fp_in   = stdin;

   g_thread_init( NULL );
   eh_init_glib();
   g_log_set_handler( NULL , G_LOG_LEVEL_MASK|G_LOG_FLAG_FATAL , &eh_logger , NULL );

   g_option_context_add_main_entries( context , entries , NULL );

   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   if ( version )
      eh_fprint_version_info( stdout , "sedflux-read-hydro" , 0 , 1 , 0 ), exit(0);

   if ( debug )
      g_setenv( "SEDFLUX_READ_HYDRO" , "TRUE" , TRUE );

   eh_set_verbosity_level( verbosity );

   if ( in_file )
   {
      fp_in = eh_fopen_error( in_file , "r" , &error );
      if ( !fp_in )
      {
         eh_error( "Error opening file: %s" , error->message );
         return 0;
      }

   }

   if ( from!=sed_hydrotrend_guess_byte_order( fp_in ) )
      eh_error( "\"From\" byte order doesn't appear to match the file byte order" );

   if ( info )
   {
      Sed_hydrotrend_header* h = sed_hydrotrend_read_header_from_byte_order( fp_in , from );

      eh_info( "File name             : %s" , in_file            );
      eh_info( "Number of grain sizes : %d" , h->n_grains        );
      eh_info( "Number of seasons     : %d" , h->n_seasons       );
      eh_info( "Number of records     : %d" , h->n_samples       );
      eh_info( "Comment               : %s" , h->comment         );
      eh_info( "Byte order            : %s" , byte_order_s(from) );

      return 0;
   }

   if ( out_file )
   {
      fp_out = eh_fopen_error( out_file , "w" , &error );
      if ( !fp_out )
         eh_error( "Error opening file: %s" , error->message );
   }

   eh_info( "First record           : %d" , start    );
   eh_info( "Number of records      : %d" , n_recs   );
   eh_info( "Buffer length          : %d" , buf_len  );
   eh_info( "Fraction of load       : %f" , fraction );
   eh_info( "Source byte order      : %s" , byte_order_s(from) );
   eh_info( "Destination byte order : %s" , byte_order_s(to  ) );

   {
      gint i;
      Sed_hydro* all_recs;
      Sed_hydro* big_recs;
      gint top_rec = start + n_recs*buf_len;

      for ( i=start ; i<top_rec ; i+=buf_len )
      {
         all_recs = sed_hydrotrend_read_records( fp_in , i , buf_len , from , &error );
         big_recs = sed_hydro_array_eventize( all_recs , fraction , !just_events );

         write_dlm_data( big_recs );

         eh_info( "Block start            : %d" , i                                        );
         eh_info( "Total load             : %f" , sed_hydro_array_suspended_load(all_recs) );
         eh_info( "Modeled load           : %f" , sed_hydro_array_suspended_load(big_recs) );
         eh_info( "Number of events       : %d" , g_strv_length( (gchar**)big_recs )       );

         all_recs = sed_hydro_array_destroy( all_recs );
         big_recs = sed_hydro_array_destroy( big_recs );
      }
   }

   fclose( fp_in  );
   fclose( fp_out );

   return 0;
}

void
write_dlm_data( Sed_hydro* a )
{
   if ( a )
   {
      Sed_hydro* r;
      double     t;
      for ( r=a ; *r ; r++ )
         for ( t=0 ; t<sed_hydro_duration(*r) ; t++ )
            fprintf( stdout , "%f\n" , sed_hydro_water_flux(*r) );
            //fprintf( stdout , "%f\n" , sed_hydro_suspended_load(*r)/sed_hydro_duration(*r) );
   }
   return;
}

#ifdef IGNORE_THIS

int main(int argc,char *argv[])
{
   char *prop_vals[] = { "q" , "qs" , "v" , "w" , "d" , "bed" , NULL };
   int j;
   int n_recs, buf_len, n_sig;
   int start;
   int type;
   double time, total_time;
   int prop;
   gboolean verbose, use_buf;
   char *infile;
   Eh_args *args;
   Hydro_get_val_func get_val;
   Sed_hydro_file fp;
   Sed_hydro rec;
   GPtrArray *rec_array;

   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   total_time = eh_get_opt_dbl ( args , "dt" , 365.    );
   n_recs  = eh_get_opt_int ( args , "nrecs" , 10    );
   buf_len = eh_get_opt_int ( args , "len"   , 365   );
   n_sig   = eh_get_opt_int ( args , "nsig"  , 5     );
   start   = eh_get_opt_int ( args , "start" , 0     );
   verbose = eh_get_opt_bool( args , "v"     , FALSE );
   use_buf = eh_get_opt_bool( args , "buf"   , FALSE );
   infile  = eh_get_opt_str ( args , "in"    , "-"   );
   prop    = eh_get_opt_key ( args , "prop"  , 0 , prop_vals );
   
   switch ( prop )
   {
      case 0:
         get_val = &sed_hydro_water_flux;
         break;
      case 1:
         get_val = &sed_hydro_suspended_flux;
         break;
      case 2:
         get_val = &sed_hydro_velocity;
         break;
      case 3:
         get_val = &sed_hydro_width;
         break;
      case 4:
         get_val = &sed_hydro_depth;
         break;
      case 5:
         get_val = &sed_hydro_bedload;
         break;
   }
   
   if ( use_buf )
      type = HYDRO_HYDROTREND|HYDRO_USE_BUFFER;
   else
      type = HYDRO_HYDROTREND;

   if ( verbose )
   {
      fprintf(stderr,"--- head ---\n");
      fprintf(stderr,"total time (days) : %f\n",total_time);
      fprintf(stderr,"n records    : %d\n",n_recs);
      fprintf(stderr,"buf length   : %d\n",buf_len);
      fprintf(stderr,"n sig events : %d\n",n_sig);
      fprintf(stderr,"start        : %d\n",start);
      fprintf(stderr,"property     : %s\n",prop_vals[prop]);
      fprintf(stderr,"buffer       : %d\n",use_buf);
   }

   fp = sed_hydro_file_new( infile , type , TRUE );
   sed_hydro_file_set_sig_values( fp , n_sig );
   sed_hydro_file_set_buffer_length( fp , buf_len );

   rec_array = g_ptr_array_new( );
   fprintf(stdout,"%s\n",prop_vals[prop]);
   for ( time=0 ; time<total_time ; )
   {
      rec = sed_hydro_file_read_record( fp );
      if ( sed_hydro_duration(rec) + time > total_time )
         sed_hydro_set_duration( rec , total_time-time );
      for ( j=0 ; j<sed_hydro_duration(rec) ; j++ )
         fprintf(stdout,"%f\n",(*get_val)( rec ));
      time += sed_hydro_duration(rec);
      g_ptr_array_add( rec_array , rec );
   }
   
   if ( verbose )
   {
      double total = 0, total_qs = 0;

      for ( j=0 ; j<rec_array->len ; j++ )
      {
         rec       = g_ptr_array_index( rec_array , j );
         total    += (*get_val)( rec )*sed_hydro_duration(rec);
         total_qs += sed_hydro_suspended_load( rec );
      }

      fprintf(stderr,"--- tail ---\n");
      fprintf(stderr,"n events         : %d\n",rec_array->len);
      fprintf(stderr,"total            : %f\n",total);
      fprintf(stderr,"total qs         : %f\n",total_qs);
   }

   g_ptr_array_free( rec_array , FALSE );

   sed_hydro_file_destroy( fp );

   return 0;
}

#endif

