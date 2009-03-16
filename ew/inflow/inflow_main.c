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
#include "inflow.h"
#include "inflow_local.h"
#include <sed/sed_sedflux.h>
#include <utils/utils.h>

// Command line arguments
static gint     verbose    = 0;
static gboolean version    = FALSE;
static gboolean debug      = FALSE;
static gdouble  day        = 1.;
static gdouble  angle      = 14.;
static gchar*   in_file    = NULL;
static gchar*   out_file   = NULL;
static gchar*   bathy_file = NULL;
static gchar*   flood_file = NULL;
static gchar*   data_file  = NULL;

static GOptionEntry entries[] =
{
   { "in-file"    , 'i' , 0 , G_OPTION_ARG_FILENAME , &in_file    , "Initialization file" , "<file>" } ,
   { "out-file"   , 'o' , 0 , G_OPTION_ARG_FILENAME , &out_file   , "Output file"         , "<file>" } ,
   { "bathy-file" , 'b' , 0 , G_OPTION_ARG_FILENAME , &bathy_file , "Bathymetry file"     , "<file>" } ,
   { "flood-file" , 'f' , 0 , G_OPTION_ARG_FILENAME , &flood_file , "Flood file"          , "<file>" } ,
   { "data-file"  , 'd' , 0 , G_OPTION_ARG_FILENAME , &data_file  , "Data file"           , "<file>" } ,
   { "angle"      , 'a' , 0 , G_OPTION_ARG_DOUBLE   , &angle      , "Spreading angle"     , "DEG"    } ,
   { "verbose"    , 'V' , 0 , G_OPTION_ARG_INT      , &verbose    , "Verbosity level"     , "n"      } ,
   { "version"    , 'v' , 0 , G_OPTION_ARG_NONE     , &version    , "Version number"      , NULL     } ,
   { "debug"      , 'b' , 0 , G_OPTION_ARG_NONE     , &debug      , "Write debug messages", NULL     } ,
   { NULL }
};

void
inflow_run_flood( Inflow_bathy_st* b    ,
                  Inflow_flood_st* f    ,
                  Inflow_sediment_st* s ,
                  Inflow_const_st* c    ,
                  double** deposit_in_m )
{
   gint n, i;
   //FILE* fp_debug = g_getenv("INFLOW_DEBUG")?stderr:NULL;
   double t       = 0.;
   double dt      = S_SECONDS_PER_DAY;
   double total_t = f->duration;
   double** deposition = eh_new_2( double , s->n_grains , b->len );
   double** erosion    = eh_new_2( double , s->n_grains , b->len );

   for ( t=0 ; t<total_t ; t+=dt )
   {
      if ( t+dt > total_t )
         dt = total_t-t;

      f->duration = dt;

      inflow_wrapper( b , f , s , c , deposition , erosion );

      inflow_update_bathy_data( b , deposition , erosion , s->n_grains );

      for ( n=0 ; n<s->n_grains ; n++ )
         for ( i=0 ; i<b->len ; i++ )
            deposit_in_m[n][i] += deposition[n][i] - erosion[n][i];
   }

   eh_free_2( erosion    );
   eh_free_2( deposition );
}

void inflow_set_width( Inflow_bathy_st* bathy_data ,
                       double river_width          ,
                       double spreading_angle );

int main(int argc,char *argv[])
{
   gchar* program_name;
   GOptionContext* context = g_option_context_new( "Run hyperpycnal flow model." );
   GError* error = NULL;
   double spreading_angle;
   Eh_dbl_grid deposit;
   Eh_dbl_grid total_deposit;
   gint i;
   gboolean mode_1d;
   Inflow_param_st*    param;
   Inflow_bathy_st*    bathy_data;
   Inflow_flood_st**   flood_data;
   Inflow_const_st*    const_data;
   Inflow_sediment_st* sediment_data;

   g_option_context_add_main_entries( context , entries , NULL );

   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   day            *= S_SECONDS_PER_DAY;
   spreading_angle = tan(angle*G_PI/180.);

   if ( version )
   {
      eh_fprint_version_info( stdout , "inflow" , 0 , 9 , 0 );
      eh_exit(0);
   }

   if ( debug )
      g_setenv( "INFLOW_DEBUG" , "TRUE" , TRUE );

   program_name = g_path_get_basename( argv[0] );
   if ( strcasecmp( program_name , "inflow1d")==0 )
   {
      angle   = 0.;
      mode_1d = TRUE;
   }

   if ( verbose )
   {
      if ( mode_1d )
         eh_info( "Operating in 1D mode (ignoring width information)." );
      else
         eh_info( "Operating in 1.5D mode." );

      eh_info( "Duration of flow (days)   : %f" , day*S_DAYS_PER_SECOND );
      eh_info( "Spreading angle (degrees) : %f" , angle );
   }

   if (    ( param      = inflow_scan_parameter_file( in_file            , &error ) )==NULL
        || ( flood_data = inflow_scan_flood_file    ( flood_file , param , &error ) )==NULL
        || ( bathy_data = inflow_scan_bathy_file    ( bathy_file , param , &error ) )==NULL )
      eh_error( "%s" , error->message );

   const_data    = inflow_set_constant_data  ( param );
   sediment_data = inflow_set_sediment_data  ( param );

   deposit       = eh_grid_new( double , sediment_data->n_grains , bathy_data->len );
   total_deposit = eh_grid_new( double , sediment_data->n_grains , bathy_data->len );

   for ( i=0 ; flood_data[i] ; i++ )
   {
      inflow_set_width( bathy_data , flood_data[i]->width , spreading_angle );

      inflow_run_flood( bathy_data , flood_data[i] , sediment_data , const_data ,
                        eh_dbl_grid_data(deposit) );
      eh_dbl_grid_add( total_deposit , deposit );
   }

   inflow_write_output( out_file , bathy_data , eh_dbl_grid_data(total_deposit) , sediment_data->n_grains );

//   for ( n=0 ; n<n_grains ; n++ )
//      fwrite( deposit[n] , n_nodes , sizeof(double) , fp_data );

   eh_grid_destroy( total_deposit , TRUE );
   eh_grid_destroy( deposit       , TRUE );

   return 0;
}

void
inflow_set_width( Inflow_bathy_st* bathy_data ,
                  double river_width          ,
                  double spreading_angle )
{
   gint   i;
   double dx = bathy_data->x[1] - bathy_data->x[0];
   double flow_width;

   // Create a spreading angle.
   bathy_data->width[0] = river_width;
   for ( i=1 ; i<bathy_data->len ; i++ )
   {
      flow_width = bathy_data->width[i-1] + spreading_angle*dx;
      if ( flow_width < bathy_data->width[i] )
         bathy_data->width[i] = flow_width;
      else
         break;
   }

   return;
}

#if defined( OLD )
{

   Eh_args *args;
   FILE *fp_in, *fp_out, *fp_flood, *fp_data;
   char *infile, *outfile, *bathyfile, *floodfile, *datafile;
   gboolean verbose, mode_1d=FALSE;
   char comment[S_LINEMAX];
   double basin_len, dx, densitySeaWater, densityRiverWater;
   double depositionStart;
   double *lambda, *diameterEquivalent, *diameterComponent, *fraction;
   double *equivalentHeight, *bulkDensity, *grainDensity, *phe_bottom;
   double diameterBottom, BulkDensityBottom;
   int n_grains;
   double *discharge, *densityFlow, *river_width, *river_depth, *river_velocity;
   int flood_days;
   int i, j, n, n_nodes;
   double *x, *depth, *width, *slope, **deposit;
   double sum;
   double day;
   double angle, spreading_angle, flow_width;
   I_bottom_t get_phe_data;
   Inflow_t consts;

// read input parameters.

   fp_in = eh_open_file( infile , "r" );
   read_double_vector( fp_in , &basin_len         , 1 );
   basin_len *= 1000.;
   read_double_vector( fp_in , &dx                , 1 );
   read_double_vector( fp_in , &densitySeaWater   , 1 );
   read_double_vector( fp_in , &densityRiverWater , 1 );
   read_int_vector   ( fp_in , &n_grains          , 1 );

   lambda             = eh_new( double , n_grains );
   diameterEquivalent = eh_new( double , n_grains );
   diameterComponent  = eh_new( double , n_grains );
   fraction           = eh_new( double , n_grains );
   equivalentHeight   = eh_new( double , n_grains );
   bulkDensity        = eh_new( double , n_grains );
   grainDensity       = eh_new( double , n_grains );
   phe_bottom         = eh_new( double , n_grains );

   read_double_vector( fp_in , lambda             , n_grains );
   for ( n=0 ; n<n_grains ; n++ )
      lambda[n] /= 86400;
   read_double_vector( fp_in , diameterEquivalent , n_grains );
   read_double_vector( fp_in , diameterComponent  , n_grains );
   for ( n=0 ; n<n_grains ; n++ )
   {
      diameterEquivalent[n] /= 1e6;
      diameterComponent[n]  /= 1e6;
   }
   read_double_vector( fp_in , fraction           , n_grains );
   read_double_vector( fp_in , equivalentHeight   , n_grains );
   read_double_vector( fp_in , bulkDensity        , n_grains );
   read_double_vector( fp_in , grainDensity       , n_grains );

   read_double_vector( fp_in , &depositionStart   , 1 );
   depositionStart *= 1000;
   read_double_vector( fp_in , &diameterBottom    , 1 );
   read_double_vector( fp_in , &BulkDensityBottom , 1 );

   read_double_vector( fp_in , phe_bottom         , n_grains );
   read_double_vector( fp_in , &consts.sua        , 1 );
   read_double_vector( fp_in , &consts.sub        , 1 );
   read_double_vector( fp_in , &consts.Ea         , 1 );
   read_double_vector( fp_in , &consts.Eb         , 1 );
   read_double_vector( fp_in , &consts.Cd         , 1 );
   read_double_vector( fp_in , &consts.tanPhi     , 1 );
   consts.tanPhi = tan(consts.tanPhi*M_PI/180.);
   read_double_vector( fp_in , &consts.mu         , 1 );
   consts.mu /= 1e6;
   consts.rhoSW = 1028.;
   
   fclose( fp_in );

   /* Divide the lambdas by the equivalentHeights.  This is added
      to account for different grains occupying different portions
      of the flow height (ie sands mostly near the bottom, clays 
      distributed evenly bottom to top).
   */
   for (n=0;n<n_grains;n++)
      lambda[n] /= equivalentHeight[n];

// end reading input parameters
   
// Read flood information.

   fp_flood = eh_open_file( floodfile , "r" );
   read_int_vector( fp_flood , &flood_days , 1 );
   discharge      = eh_new( double , flood_days );
   densityFlow    = eh_new( double , flood_days );
   river_depth    = eh_new( double , flood_days );
   river_width    = eh_new( double , flood_days );
   river_velocity = eh_new( double , flood_days );

   for ( i=0 ; i<flood_days ; i++ )
   {
      fgets( comment , S_LINEMAX , fp_flood );
      read_double_vector( fp_flood , &densityFlow[i]    , 1 );
      read_double_vector( fp_flood , &river_depth[i]    , 1 );
      read_double_vector( fp_flood , &river_width[i]    , 1 );
      read_double_vector( fp_flood , &river_velocity[i] , 1 );

      if ( mode_1d )
         river_width[i] = 1.;

      discharge[i] = river_width[i]*river_depth[i]*river_velocity[i];
   }

   fclose(fp_flood);

// end reading flood info.

// read bathymetry.

   {
      Eh_data_record* all_records;
      gssize len;
      double* y = eh_uniform_array( 0 , basin_len , dx , &len );

      all_records = eh_data_record_scan_file( bathyfile , "," , EH_FAST_DIM_COL , FALSE );
      eh_data_record_interpolate_rows( all_records[0] , 0 , y , len );

      x     = eh_data_record_dup_row( all_records[0] , 0 );
      depth = eh_data_record_dup_row( all_records[0] , 1 );
      width = eh_data_record_dup_row( all_records[0] , 2 );

      slope = eh_new( double , len );
   
      for ( i=0 ; i<=len ; i++ )
         slope[i] = atan((depth[i+1]-depth[i])/(x[i+1]-x[i]));
      slope[n_nodes-1] = slope[n_nodes-2];

      for ( i=0 ; all_records[i] ; i++ )
         eh_data_record_destroy( all_records[i] );
      eh_free( all_records );

      n_nodes = len;
   }

   for ( i=0 ; i<flood_days ; i++ )
      if ( river_width[i] > width[0] )
         g_error( "The river width is greater than the basin width." );

   fwrite( &n_nodes , 1       , sizeof(int)    , fp_data );
   fwrite( x        , n_nodes , sizeof(double) , fp_data );
   fwrite( depth    , n_nodes , sizeof(double) , fp_data );
   fwrite( width    , n_nodes , sizeof(double) , fp_data );

   get_phe_data.phe_bottom = phe_bottom;
   get_phe_data.n_grains   = n_grains;
   consts.get_phe_data     = (gpointer)&get_phe_data;
   consts.get_phe          = (Sed_query_func)&inflow_get_phe;

   deposit = eh_new_2( double , n_grains , n_nodes );

   for (i=0;i<flood_days;i++)
   {
// Create a spreading angle.
      width[0] = river_width[i];
      for ( j=1 ; j<n_nodes ; j++ )
      {
         flow_width = width[j-1] + spreading_angle*dx;
         if ( flow_width < width[j] )
            width[j] = flow_width;
         else
            break;
      }

      inflow( day                , x              , slope             ,
              width              , n_nodes        , dx                ,
              depositionStart    , river_width[i] , river_velocity[i] ,
              river_depth[i]     , river_q[i]     , fraction          ,
              diameterEquivalent , lambda         , bulkDensity       ,
              grainDensity       , n_grains       , densityRiverWater ,
              densityFlow[i]     , consts         , deposit           ,
              fp_data );
   }

   for ( n=0 ; n<n_grains ; n++ )
      fwrite( deposit[n] , n_nodes , sizeof(double) , fp_data );

   fp_out = eh_open_file( outfile , "w" );

   for ( i=0 ; i<n_nodes ; i++ )
   {
      for ( n=0,sum=0 ; n<n_grains ; n++ )
         sum += deposit[n][i];
      fprintf( fp_out , "x, y, w : %f, %f, %f\n" , x[i] , depth[i]+sum , width[i] );
   }

   fclose( fp_out );
   fclose( fp_data );
   
   eh_free(river_velocity);
   eh_free(river_depth);
   eh_free(densityFlow);
   eh_free(discharge);
   eh_free(phe_bottom);
   eh_free(bulkDensity);
   eh_free(fraction);
   eh_free(diameterComponent);
   eh_free(diameterEquivalent);
   eh_free(lambda);
   eh_free_2(deposit);
   
   return 0;
}
#endif

