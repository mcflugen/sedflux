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
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "avulsion.h"
#include "avulsion_api.h"

static double   init_angle = 90.;
static double   min_angle  = 0.;
static double   max_angle  = 180.;
static double   std_dev    = 1.;
static gint     seed       = 0;
static gint     n_times    = 100;
static gint     verbose    = 0;
static gboolean version    = FALSE;
static gboolean header     = FALSE;

static gboolean lite       = FALSE;

static double   eps        = 5.;
static double   alpha      = 45.;
static gint     n_i        = 100;
static gint     n_j        = 100;
static gint     n_rivers   = 1;
static gchar*   in_file    = NULL;
static gchar*   out_type_s = NULL;

static GOptionEntry entries[] =
{
   { "n-times" , 'n' , 0 , G_OPTION_ARG_INT    , &n_times    , "Number of iterations"      , "N" } ,
   { "std-dev" , 's' , 0 , G_OPTION_ARG_DOUBLE , &std_dev    , "Standard deviation (degs)" , "SIGMA" } ,
   { "min"     , 'l' , 0 , G_OPTION_ARG_DOUBLE , &min_angle  , "Minimum angle (degs)"      , "MIN" } ,
   { "max"     , 'h' , 0 , G_OPTION_ARG_DOUBLE , &max_angle  , "Maximum angle (degs)"      , "MAX" } ,
   { "start"   , 'i' , 0 , G_OPTION_ARG_DOUBLE , &init_angle , "Starting angle (degs)"     , "ANGLE" } ,
   { "seed"    , 'd' , 0 , G_OPTION_ARG_INT    , &seed       , "Seed for RNG"              , "SEED" } ,
   { "verbose" , 'V' , 0 , G_OPTION_ARG_INT    , &verbose    , "Verbosity level"           , "N" } ,
   { "version" , 'v' , 0 , G_OPTION_ARG_NONE   , &version    , "Version number"            , NULL } ,
   { "header"  , 'H' , 0 , G_OPTION_ARG_NONE   , &header     , "Print a header"            , NULL } ,

   { "lite"    , 'L' , 0 , G_OPTION_ARG_NONE   , &lite       , "Run the lite version"      , NULL } ,

   { "eps"     , 'e' , 0 , G_OPTION_ARG_DOUBLE , &eps        , "River join angle (degs)"   , "ANGLE" } ,
   { "alpha"   , 'a' , 0 , G_OPTION_ARG_DOUBLE , &alpha      , "River split angle (degs)"  , "ANGLE" } ,
   { "nx"      , 'x' , 0 , G_OPTION_ARG_INT    , &n_i        , "Number of rows"            , "N" } ,
   { "ny"      , 'y' , 0 , G_OPTION_ARG_INT    , &n_j        , "Number of columns"         , "N" } ,
   { "n-rivers", 'r' , 0 , G_OPTION_ARG_INT    , &n_rivers   , "Number of rivers"          , "N" } ,
   { "river-file", 'f' , 0 , G_OPTION_ARG_FILENAME , &in_file   , "River file"          , "<file>" } ,
   { "out-type", 't' , 0 , G_OPTION_ARG_STRING , &out_type_s , "Type of output file"       , "<out-key>" } ,

   { NULL }
};

int avulsion_lite( void );
int avulsion_full( void );

typedef enum
{
   AVULSION_OUTPUT_ANGLES,
   AVULSION_OUTPUT_DEPTHS
}
Avulsion_out_type;

Avulsion_out_type output_type;

int main( int argc , char *argv[] )
{
   g_thread_init (NULL);
   eh_init_glib ();
   g_log_set_handler (NULL, G_LOG_LEVEL_MASK, &eh_logger, NULL);

  {
   GError* error = NULL;
   GOptionContext* context = g_option_context_new( "Run random walk avulsion model" );
   gchar** command_line = eh_new0( gchar* , argc );
   gint i;

   for ( i=1 ; i<argc ; i++ )
      command_line[i-1] = g_strdup( argv[i] );

   g_option_context_add_main_entries( context , entries , NULL );

   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   if ( version )
   {
      eh_fprint_version_info( stdout ,
                              AVULSION_PROGRAM_NAME ,
                              AVULSION_MAJOR_VERSION , 
                              AVULSION_MINOR_VERSION , 
                              AVULSION_MICRO_VERSION );
      exit(0);
   }

   eh_set_verbosity_level( verbose );

   min_angle  *= S_RADS_PER_DEGREE;
   max_angle  *= S_RADS_PER_DEGREE;
   init_angle *= S_RADS_PER_DEGREE;
   std_dev    *= S_RADS_PER_DEGREE;
   eps        *= S_RADS_PER_DEGREE;
   alpha      *= S_RADS_PER_DEGREE;

   if ( out_type_s )
   {
      if      ( g_ascii_strcasecmp(out_type_s,"ANGLE")==0 )
         output_type = AVULSION_OUTPUT_ANGLES;
      else if ( g_ascii_strcasecmp(out_type_s,"DEPTH")==0 )
         output_type = AVULSION_OUTPUT_DEPTHS;
      else
         eh_require_not_reached();
   }
   else
      output_type = AVULSION_OUTPUT_ANGLES;

   if ( header )
   {
      fprintf( stdout , "# Minimum angle      = %f\n" , min_angle/S_RADS_PER_DEGREE );
      fprintf( stdout , "# Maximum angle      = %f\n" , max_angle/S_RADS_PER_DEGREE );
      fprintf( stdout , "# Standard deviation = %f\n" , std_dev/S_RADS_PER_DEGREE   );
      fprintf( stdout , "# Command line       = %s\n" , g_strjoinv( " " , command_line ) );
   }

   if ( lite )
      avulsion_lite( );
   else
      main_new ();
      //avulsion_full( );

   g_strfreev( command_line );

  }

  return 0;
}

int
avulsion_lite( void )
{
   {
      gint n;
      double angle = init_angle;
      GRand* rand = (seed==0)?g_rand_new():g_rand_new_with_seed(seed);
      double m;

      m = 2.*G_PI/(max_angle-min_angle);

      std_dev *= m;

      for ( n=0 ; n<n_times ; n++ )
      {
         angle = avulsion( rand , angle , std_dev );
         fprintf( stdout , "%f\n" , (angle/m+.5*(min_angle+max_angle))*S_DEGREES_PER_RAD );
      }

      g_rand_free( rand );
   }

   return 0;
}

typedef struct
{
   double angle;
   double stddev;
   double size;
   int id;
}
stream_st;

GList *avulse_streams( GList *streams );
GList *create_streams( GList *streams , double alpha );
GList *merge_streams( GList *streams , double eps );
GList *print_streams( GList *streams );
GList *print_streams_as_table( GList *streams );
void avulse_stream( stream_st *stream , gpointer data );
int sort_streams_by_angle( stream_st *s1 , stream_st *s2 );
int sort_streams_by_discharge( stream_st *s1 , stream_st *s2 );
int sort_streams_by_id( stream_st *s1 , stream_st *s2 );
void combine_rivers( Sed_riv r_1 , Sed_riv r_2 );

void sed_merge_all_rivers( Sed_cube c , double eps );
GList *merge_rivers( GList *rivers , double eps );
int sort_rivers_by_angle( Sed_riv r_1 , Sed_riv r_2 );
int sort_rivers_by_discharge( Sed_riv r_1 , Sed_riv r_2 );
void combine_river_discharge( Sed_riv r_1 , Sed_riv r_2 );
Sed_riv split_river_discharge( Sed_riv r_1 , Sed_riv r_2 );
void sed_split_all_rivers( Sed_cube c , double alpha );
GList *create_rivers( GList *rivers , double alpha );
void deposit_sediment_at_river_mouth( Sed_cube c );

int avulsion_full( )
{
   Sed_cube cube;
  
   {
      GError*      error         = NULL;
      Sed_sediment sediment_type = sed_sediment_scan( SED_SEDIMENT_TEST_FILE , &error );

      if ( !sediment_type )
         eh_error( "%s: Unable to read sediment file: %s" , SED_SEDIMENT_TEST_FILE , error->message);

      sed_sediment_set_env( sediment_type );

      sed_sediment_destroy( sediment_type );
   }

   {
      Eh_dbl_grid bathy_grid = sed_get_floor_3_default( 1 , n_i , n_j );

      cube = sed_cube_new( eh_grid_n_x(bathy_grid) ,
                           eh_grid_n_y(bathy_grid) );
      sed_cube_set_dz   ( cube , 1. );
      sed_cube_set_bathy( cube , bathy_grid );
   
      eh_grid_destroy( bathy_grid , TRUE );
   }

   {
      gint i;
      Avulsion_st* data;
      Sed_riv new_river;
      Sed_hydro* hydro_data = sed_hydro_scan( NULL , NULL );
      double**   river_data = NULL;

      if ( in_file )
      {
         gint n_rows, n_cols;

         river_data = eh_dlm_read( in_file , ",;" , &n_rows , &n_cols , NULL );

         eh_require( n_cols==3 );
         n_rivers = n_rows;
      }
      else
      {
         river_data = eh_new_2( double , n_rivers , 3 );
         for ( i=0 ; i<n_rivers ; i++ )
         {
            river_data[i][0] = 0.;
            river_data[i][1] = 90.;
            river_data[i][2] = std_dev*S_DEGREES_PER_RAD;
         }
      }

      for ( i=0 ; i<n_rivers ; i++ )
      {
         min_angle = river_data[i][0]*S_RADS_PER_DEGREE;
         max_angle = river_data[i][1]*S_RADS_PER_DEGREE;
         std_dev   = river_data[i][2]*S_RADS_PER_DEGREE;

         data = avulsion_new( (seed==0)?g_rand_new():g_rand_new_with_seed(seed) , std_dev );

         new_river = sed_river_new  ( NULL );

         sed_river_set_angle        ( new_river , .5*(min_angle+max_angle) );
         sed_river_set_angle_limit  ( new_river , min_angle , max_angle );
         sed_river_set_hinge        ( new_river , sed_cube_n_x(cube)/2 , 0 );
         sed_river_set_hydro        ( new_river , hydro_data[0] );
         sed_river_set_avulsion_data( new_river , data );
         sed_cube_add_trunk         ( cube      , new_river );
      }

      eh_free_2( river_data );
   }

   {
      gint i, n;
      double t=0, t_end=n_times;
      Eh_status_bar* sb = eh_status_bar_new( &t , &t_end );

      for ( n=0 ; n<n_times ; n++,t++ )
      {
         sed_cube_avulse_all_rivers( cube );
/*
         sed_merge_all_rivers ( cube , eps );
         sed_split_all_rivers( cube , alpha );
*/
         if ( output_type==AVULSION_OUTPUT_ANGLES )
         {
            for ( i=0 ; i<n_rivers ; i++ )
               fprintf( stdout , "%f " , sed_river_angle( sed_cube_nth_river( cube,i ) )*S_DEGREES_PER_RAD );
            fprintf( stdout , "\n" );
         }

         deposit_sediment_at_river_mouth( cube );
      }

      eh_status_bar_destroy(sb);
   }

   if ( output_type==AVULSION_OUTPUT_DEPTHS )
   {
/*
      Sed_measurement x = sed_measurement_new( "depth" );
      Sed_tripod met_fp = sed_tripod_new( "test.depth" , x , NULL );

      sed_tripod_write( met_fp , cube );

      sed_tripod_destroy( met_fp );
      sed_measurement_destroy( x );
*/
      gint i,j;
      gint top_i = sed_cube_n_x(cube);
      gint top_j = sed_cube_n_y(cube)-1;

      for ( i=0 ; i<top_i ; i++ )
      {
         for ( j=0 ; j<top_j ; j++ )
            fprintf( stdout , "%f " , sed_cube_water_depth(cube,i,j) );
         fprintf( stdout , "%f\n" , sed_cube_water_depth(cube,i,j) );
      }
   }

   sed_sediment_unset_env();

   return 0;
}

int
main_new ()
{
  Avulsion_state* s = avulsion_init (NULL);

  eh_message ("Read sediment file");
  {
    GError* error = NULL;
    //Sed_sediment sediment_type = sed_sediment_scan (NULL, &error);
    gchar* buffer = sed_sediment_default_text ();
    Sed_sediment sediment_type = sed_sediment_scan_text (buffer, &error);

    if (!sediment_type)
      eh_error ("%s: Unable to read sediment file: %s", SED_SEDIMENT_TEST_FILE,
                error->message);

    sed_sediment_set_env( sediment_type );
    sed_sediment_destroy( sediment_type );
  }

  eh_message ("Set the grid");
  { /* Set the grid */
    Eh_dbl_grid bathy_grid = sed_get_floor_3_default (3, n_i, n_j);
    gint shape[2] = {n_j, n_i};
    double res[2] = {1., 1.};

    avulsion_set_grid (s, shape, res);
    avulsion_set_elevation (s, eh_dbl_grid_data_start (bathy_grid));

    eh_grid_destroy (bathy_grid, TRUE);
  }

  eh_message ("Set avulsion data");
  {
    gchar* buffer = sed_hydro_default_text ();
    Sed_hydro* hydro_data = sed_hydro_scan_text (buffer, NULL);

    eh_message ("Set angle limits");
    {
      double limit[2];
      limit[0] = min_angle;
      limit[1] = max_angle;

      avulsion_set_river_angle_limit (s, limit);
    }

    eh_message ("Set variance");
    {
      double variance = std_dev;

      avulsion_set_variance (s, variance);
    }

    eh_message ("Set hinge");
    {
      gint hinge[2];
      hinge[0] = 0;
      hinge[1] = n_i/2;

      avulsion_set_river_hinge (s, hinge);
    }

    eh_message ("Set hydro");
    //avulsion_set_river_hydro (s, hydro_data[0]);

    eh_message ("Run the model");
    {
      int i;
      for (i=1; i<=n_times; i++)
      {
        avulsion_run_until (s, i);
        fprintf (stderr, "%f\n", avulsion_get_angle (s));
      }

      {
        gint lower[3], upper[3], stride[3];
        double* data = avulsion_get_value_data (s, "discharge", lower, upper,
                                                stride);
        const int len = n_i*n_j;
        for (i=0; i<len; i++)
          fprintf (stdout,"%f\n", data[i]);
      }
    }

    eh_message ("Clean up the model");
    avulsion_finalize (s, FALSE);

    eh_message ("Done");
    sed_sediment_unset_env();

  }

  return 0;
}

void deposit_sediment_helper( Sed_riv this_river , Sed_cube c );

void deposit_sediment_at_river_mouth( Sed_cube c )
{
   g_list_foreach( sed_cube_river_list(c) , (GFunc)&deposit_sediment_helper , c );
}

void deposit_sediment_helper( Sed_riv this_river , Sed_cube c )
{
   Eh_ind_2 mouth_pos;
   Sed_cell deposit_cell;

   {
      gssize n;
      gssize len = sed_sediment_env_n_types();
      double* f = eh_new( double , len );

      f[0] = eh_get_fuzzy_dbl( 0 , 1 );
      for ( n=1 ; n<len ; n++ )
         f[n] = ( 1.-f[0] ) / ( (double)len - 1. );

      deposit_cell = sed_cell_new_sized( len , 1. , f );

      eh_free( f );
   }

   this_river = sed_cube_find_river_mouth( c , this_river );

   mouth_pos = sed_river_mouth( this_river );

   if ( sed_cube_is_in_domain( c , mouth_pos.i , mouth_pos.j ) )
   {
      double depth = sed_column_water_depth(sed_cube_col_ij(c,mouth_pos.i,mouth_pos.j));

      if ( depth<sed_cell_size(deposit_cell) )
         sed_cell_resize( deposit_cell , depth );
      sed_column_add_cell( sed_cube_col_ij(c,mouth_pos.i,mouth_pos.j) , deposit_cell );
   }

   sed_cell_destroy( deposit_cell );
}

void
sed_merge_all_rivers( Sed_cube c , double eps )
{
   sed_cube_set_river_list( c , merge_rivers( sed_cube_river_list(c) , eps ) );
}

GList
*merge_rivers( GList *rivers , double eps )
{
   double d_theta;
   Sed_riv this_river, last_river;
   GList *this_link, *last_link;

   rivers = g_list_sort( rivers , (GCompareFunc)&sort_rivers_by_angle );

   for ( this_link=rivers->next ; this_link ; this_link=this_link->next )
   {
      last_link  = this_link->prev;

      this_river = this_link->data;
      last_river = last_link->data;

      d_theta = sed_river_angle(this_river)
              - sed_river_angle(last_river);
      if ( d_theta < eps )
      {
         if ( sort_rivers_by_discharge( this_river , last_river ) > 0 )
         {
            combine_river_discharge( this_river , last_river );
            rivers = g_list_remove_link( rivers , last_link );
         }
         else
         {
            combine_river_discharge( last_river , this_river );
            rivers = g_list_remove_link( rivers , this_link );
         }
      }
   }
      
   return rivers;
}

int sort_rivers_by_angle( Sed_riv r_1 , Sed_riv r_2 )
{
   if ( sed_river_angle(r_1) > sed_river_angle(r_2) )
      return 1;
   else if ( sed_river_angle(r_1) < sed_river_angle(r_2) )
      return -1;
   else
      return 0;
}

int sort_rivers_by_discharge( Sed_riv r_1 , Sed_riv r_2 )
{
   double q_1 = sed_river_water_flux(r_1);
   double q_2 = sed_river_water_flux(r_2);

   if ( q_1 > q_2 )
      return 1;
   else if ( q_1 < q_2 )
      return -1;
   else
      return 0;
}

int sort_streams_by_id( stream_st *s1 , stream_st *s2 )
{
   if ( s1->id > s2->id )
      return 1;
   else if ( s1->id < s2->id )
      return -1;
   else
      return 0;
}

void combine_river_discharge( Sed_riv r_1 , Sed_riv r_2 )
{
   double total_q;

   eh_require( r_1!=NULL );
   eh_require( r_2!=NULL );

   total_q = sed_river_water_flux( r_1 ) 
           + sed_river_water_flux( r_2 );

   sed_river_set_width( r_1 ,   total_q
                              / ( sed_river_velocity(r_1)*sed_river_depth(r_1) ) );

}

Sed_riv split_river_discharge( Sed_riv r_1 , Sed_riv r_2 )
{
   double q_1, q_2;
   double f=.25;
   Sed_riv new_river;

   eh_require( r_1!=NULL );
   eh_require( r_2!=NULL );

   q_1 = sed_river_water_flux( r_1 );
   q_2 = sed_river_water_flux( r_2 );

   new_river = sed_river_dup( r_1 );

   sed_river_set_angle   ( new_river , .5*( sed_river_angle   (r_1) + sed_river_angle   (r_2) ) );
   sed_river_set_velocity( new_river , .5*( sed_river_velocity(r_1) + sed_river_velocity(r_2) ) );
   sed_river_set_depth   ( new_river , .5*( sed_river_depth   (r_1) + sed_river_depth   (r_2) ) );
   sed_river_set_bedload ( new_river , .5*( sed_river_bedload (r_1) + sed_river_bedload (r_2) ) );
   sed_river_set_width   ( new_river , f*( q_1+q_2 )
                                     / (   sed_river_velocity( new_river )
                                         * sed_river_depth   ( new_river ) ) );

   sed_river_set_width( r_1 , sed_river_width(r_1)*(1.-f) );
   sed_river_set_width( r_2 , sed_river_width(r_2)*(1.-f) );

   return new_river;
}

void sed_split_all_rivers( Sed_cube c , double alpha )
{
   sed_cube_set_river_list( c , create_rivers( sed_cube_river_list(c) , alpha ) );
}

GList *create_rivers( GList *rivers , double alpha )
{
   double d_theta;
   GList *this_link, *last_link;
   Sed_riv left_wall, right_wall;
   Sed_riv this_river, last_river, new_river;

   rivers = g_list_sort( rivers , (GCompareFunc)&sort_rivers_by_angle );

   left_wall  = sed_river_dup( g_list_first( rivers )->data );
   sed_river_set_angle( left_wall , G_PI );
   sed_river_set_width( left_wall , 0.   );

   right_wall  = sed_river_dup( g_list_last( rivers )->data );
   sed_river_set_angle( right_wall , 0. );
   sed_river_set_width( right_wall , 0. );

   rivers = g_list_insert_sorted( rivers     ,
                                  left_wall  ,
                                  (GCompareFunc)&sort_rivers_by_angle );
   rivers = g_list_insert_sorted( rivers     ,
                                  right_wall ,
                                  (GCompareFunc)&sort_rivers_by_angle );

   for ( this_link=rivers->next ; this_link ; this_link=this_link->next )
   {
      last_link = this_link->prev;

      this_river = this_link->data;
      last_river = last_link->data;

      d_theta = sed_river_angle(this_river) - sed_river_angle(last_river);

      if ( d_theta > alpha )
      {

         new_river = split_river_discharge( this_river , last_river );

         rivers = g_list_insert_sorted( rivers    ,
                                        new_river ,
                                        (GCompareFunc)&sort_rivers_by_angle );
      }
   }
   
   rivers = g_list_remove( rivers , left_wall  );
   rivers = g_list_remove( rivers , right_wall );

   sed_river_destroy( left_wall  );
   sed_river_destroy( right_wall );

   return rivers;
}
/*
GList *print_streams( GList *streams )
{
   int i, j;
   int n_cols = 144, pos, last_pos, d_pos;
   stream_st left_wall = { -M_PI_2 , 0. , 0. , -1 };
   GList *cur, *last;

   streams = g_list_sort( streams , (GCompareFunc)&sort_streams_by_angle );
   streams = g_list_insert_sorted( streams , &left_wall , (GCompareFunc)&sort_streams_by_angle );

   last_pos = 0;
   for ( cur=streams->next ; cur ; cur=cur->next )
   {
      pos = (((stream_st*)(cur->data))->angle+M_PI_2)*n_cols/M_PI;
      d_pos = pos-last_pos;
      for ( j=0 ; j<d_pos-1 ; j++ )
         fprintf(stdout," ");
      fprintf(stdout,"%d",((stream_st*)(cur->data))->id);
      last_pos = pos;
   }

   d_pos = n_cols-last_pos;
   for ( j=0 ; j<d_pos-1 ; j++ )
      fprintf(stdout," ");
   fprintf(stdout,"|\n");

   streams = g_list_remove( streams , &left_wall );

   return streams;
}

GList *print_streams_as_table( GList *streams )
{
   int i, j;
   int n_cols = 144, pos, last_pos, d_pos;
   stream_st left_wall = { -M_PI_2 , 0. , 0. , -1 };
   GList *cur, *last;

   streams = g_list_sort( streams , (GCompareFunc)&sort_streams_by_id );

   last_pos = 0;
   for ( cur=streams ; cur->next ; cur=cur->next )
      fprintf(stdout,"%f, %f, ",((stream_st*)(cur->data))->angle,((stream_st*)(cur->data))->size);
   fprintf(stdout,"%f, %f\n",((stream_st*)(cur->data))->angle,((stream_st*)(cur->data))->size);

   return streams;
}
*/
