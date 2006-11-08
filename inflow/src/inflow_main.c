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
#include "sed_sedflux.h"
#include "utils.h"

/*** Self Documentation ***/
static char *help_msg[] =
{
"                                                                             ",
" inflow [options] [parameters]  [filein]                                     ",
"  run a turbidity current.                                                   ",
"                                                                             ",
" Options                                                                     ",
"  -o outfile  - send output to outfile instead of stdout. [stdout]           ",
"  -v          - be verbose. [off]                                            ",
"  -h          - print this help message.                                     ",
"                                                                             ",
" Parameters                                                                  ",
"  -pday=value - set the length of a day to be value fraction of 24 hrs. [1]  ",
"  -pangle=value - set the spreading angle of the flow (degrees). [14]        ",
"                                                                             ",
"  -fin        - input parameter file. [stdin]                                ",
"  -fflood     - input flood data file. [inflow.flood]                        ",
"  -fbathy     - input bathymetry file. [inflow.bathy]                        ",
"  -fdata      - output data file. [inflow.data]                              ",
"  -fout       - output deposit file. [stdout]                                ",
NULL
};

#ifndef M_PI
# define M_PI		3.14159265358979323846
#endif

#ifdef OLDWAY
extern double *pheBottom_;
extern FILE *fpout_;
#endif

#define DEFAULT_DAY_LENGTH           1
#define DEFAULT_SPREADING_ANGLE      (14.)
#define DEFAULT_VERBOSE              0
#define DEFAULT_DATA_FILE_NAME       "inflow.data"
#define DEFAULT_BATHYMETRY_FILE_NAME "inflow.bathy"
#define DEFAULT_FLOOD_FILE_NAME      "inflow.flood"
#define DEFAULT_INPUT_FILE_NAME      "inflow.input"

void inflow_get_phe( gpointer phe , gpointer data );

int main(int argc,char *argv[])
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
   
   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   verbose   = eh_get_opt_bool( args , "v"     , FALSE                        );
   day       = eh_get_opt_dbl ( args , "day"   , DEFAULT_DAY_LENGTH           );
   angle     = eh_get_opt_dbl ( args , "angle" , DEFAULT_SPREADING_ANGLE      );
   infile    = eh_get_opt_str ( args , "in"    , DEFAULT_INPUT_FILE_NAME      );
   outfile   = eh_get_opt_str ( args , "out"   , NULL                         );
   bathyfile = eh_get_opt_str ( args , "bathy" , DEFAULT_BATHYMETRY_FILE_NAME );
   floodfile = eh_get_opt_str ( args , "flood" , DEFAULT_FLOOD_FILE_NAME      );
   datafile  = eh_get_opt_str ( args , "data"  , DEFAULT_DATA_FILE_NAME       );

   day *= DAY;
   spreading_angle = tan(angle*M_PI/180.);

   if ( strcmp("inflow1d",g_basename(argv[0]))==0 )
   {
      angle = 0.;
      mode_1d = TRUE;
   }

   if ( verbose )
   {
      if ( mode_1d )
      {
         fprintf(stderr,"inflow : --- operating in 1d mode ---\n");
         fprintf(stderr,"inflow : all width information will be lost.\n");
      }
      fprintf(stderr,"inflow : duration of flow (days)   : %f\n",day/DAY);
      fprintf(stderr,"inflow : spreading angle (degrees) : %f\n",angle);
   }

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

   fp_data = eh_open_file( datafile , "w" );

   fwrite( &n_nodes , 1       , sizeof(int)    , fp_data );
   fwrite( x        , n_nodes , sizeof(double) , fp_data );
   fwrite( depth    , n_nodes , sizeof(double) , fp_data );
   fwrite( width    , n_nodes , sizeof(double) , fp_data );
//   fpout_ = fp_data;

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
              river_depth[i]     , discharge[i]   , fraction          ,
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

#ifdef OLDWAY
/* pheBottom will be a global variable that is read as an input into the
   standalone model.
*/
extern double *pheBottom_;

double getPhe(double *phe,int n_grains)
{
   memcpy(phe,pheBottom_,sizeof(double)*n_grains);
   return 0.;
}
#endif

void inflow_get_phe( gpointer query_data , gpointer data )
{
   double *phe_out = EH_STRUCT_MEMBER( I_phe_query_t , query_data , phe );
   double *phe_bottom = EH_STRUCT_MEMBER( I_bottom_t , data , phe_bottom );
   int     n_grains   = EH_STRUCT_MEMBER( I_bottom_t , data , n_grains   );

   memcpy( phe_out , phe_bottom , sizeof(double)*n_grains );

   return;
}

