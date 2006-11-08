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

/* parameters
*
* x[]: node position
* depth[]: water depth at nodes
* slope[]: slope at nodes
* width[]: width of the channel but not used in sakura
* deposit[][]: deposit mass for each grain size fraction at each node
* basin_len: length of the domain
* dx: constant interval between nodes
* dt: time increments
* outtime: time interval for data output to ASCII file
* maxtime: Maximun time of the calculation
* densitySeaWater, densityRiverWater: water densities without sediments
* nGrains: number of grain size fraction
* eh_lambda: removal rate for each size fraction
* diameterEquivalent: grain sizes with flocculation effects; used for settling velocity calculation
* diameterComponent: true grain sizes; not used in sakura
* fraction: grain size distribution of input suspension from the river

* equivalentHeight: incorporating the density stratification?; not used in sakura
* bulkDensity: density of compacted deposits for each size fraction
* grainDensity: density of sediments in suspension for each size fraction
* depositionStart: not used in sakura
* diameterBottom: average grain size of deposits; not used in sakura
* BulkDensityBottom: average density of deposits; not used in sakura
* phe_bottom: grain size distribution of deposits
* consts: constants used in turbidity current calculation
* flood_days: number of days of the flood
* densityFlow: flow density at the river mouth
* river_width: width of the river mouth; not used in sakura
* river_depth: water depth at the river mouth
* river_velocity: flow velocity at the river mouth
* n_nodes: number of the nodes calculated from basin_len and dx
* settlingVelocity: settling velocity of sediments for each size fraction
* reynoldsNumber: Reynolds number of ediments for each size fraction
* Dstar, Wstar: temporary parameters for settling velocity calculations
* x_array, x_pos: temporary parameters for bathymetry data interpolation
* sum, av_graindensity: temporary parameters 
* day, angle, spreading_angle, flow_width: not used in sakura
* discharge: not used in sakura
* concentration: sediment concentration of the river water
*/
/* include files */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <fcntl.h>
#include <stdarg.h>
#include "sakura.h"
#include <utils.h>

/** The length of a day in days. */
#define DEFAULT_DAY_LENGTH           (1)
/** The spreading angle of the turbidity current in degrees. */
#define DEFAULT_SPREADING_ANGLE      (14.)
/** Specifies if verbosity is on or off. */
#define DEFAULT_VERBOSE              (0)
/** Name of output file. */
#define DEFAULT_DATA_FILE_NAME       ("sakura.data")
/** Name of input bathymetry file. */
#define DEFAULT_BATHYMETRY_FILE_NAME ("sakura.bathy")
/** Name of input flood file. */
#define DEFAULT_FLOOD_FILE_NAME      ("sakura.flood")
/** Name of main sakura input file. */
#define DEFAULT_INPUT_FILE_NAME      ("sakura.input")
/** Name of the output bathymetry file. */
#define DEFAULT_OUTPUT_FILE_NAME     ("sakura_new.bathy")
#define DEFAULT_DISCHARGE_FILE_NAME  ("sakura.discharge")
#define DISCHARGE_FILE0  ("settling.discharge")
#define DISCHARGE_FILE1  ("smallpulse.discharge")
#define DISCHARGE_FILE2  ("largepulse.discharge")
#define DISCHARGE_FILE3  ("continuous.discharge")

/* define turbidity current initial conditions */

/** Supply duration in seconds. */
#define SUPPLYTIME (60.0*60.0*12)

void sakura_get_phe( gpointer user_data , gpointer data );
void sakura_add( gpointer user_data , gpointer data );
void sakura_remove( gpointer user_data , gpointer data );
void sakura_get_depth( gpointer user_data , gpointer data );
void sakura_set_depth( gpointer user_data , gpointer data );

/** @memo simulate a turbidity current motion and deposition.

@doc
2-step time increments, with tentative flow velocity at t + 0.5 DT.  Based on
layer-averaged, 3-eqs model of Parker et al. (1986)

Eulerian type (fixed) grids of uniform interval.  Staggered cell with U (and
Ustar) placed at cell boundary (i+1/2) while C and H placed at cell center (i).

TVD scheme for interpolating values at half position using 1st order upwind
difference for unstable area to avoid numerical oscillation and 2nd order
upwind difference for other areas.  Limiter function is "minmod".

A turbidity current is generated by continuous supply of suspension from
upstream end.

There is free outflow at the downstream end.
*/
int main (void)
{
   // declare variables
   char *infile, *outfile;
   char *bathyfile, *floodfile, *datafile;  // input filenames
   char *dischargefile;
   char *discharge0, *discharge1, *discharge2, *discharge3;
   char discharge_filename[1028];
   char comment[S_LINEMAX];
   FILE *fp_in, *fp_out, *fp_flood, *fp_data;  // input files
   Eh_data_file *fp_bathy, *fp_discharge;  // input files for bathymetry data
   Eh_data_record *data, *data2; // temporary parameter to convert bathymetry data
   Eh_data_file_attr attr;
   
   double *X1, *Z1, *S1;  // provisional parameters to produce bathymetry data
   double *x, *depth, *width, *slope, **deposit, **final; // bathymetry and related dataset
   double *t, *init_u, *init_c;
   double basin_len, dx, dt, outtime, maxtime;
   double  densitySeaWater, densityRiverWater; // from input file
   int nGrains;   // from input file
   double *eh_lambda, *diameterEquivalent;
   double *diameterComponent, *fraction; // from input file
   double *equivalentHeight, *bulkDensity, *grainDensity;   // from input file
   double depositionStart, diameterBottom, BulkDensityBottom; // from input file
   double *phe_bottom;   // from input file: might be extern in SedFlux
   Sakura_t consts;   // from input file
   
   int flood_days;   // from flood file
   double *densityFlow, *river_width; // from flod file
   double *river_depth, *river_velocity; // from flood file
   double *supplytime; // from flood file
   int *flood_type;
   int n_nodes; // calculated from input file data
   double *settlingVelocity, *reynoldsNumber; // calculated from input file data
   double Dstar, Wstar, Rden; // temporary parameters for settling velocity calculations
   GArray *x_array, *t_array; // temporary parameters for bathymetry data interpolation
   double x_pos, t_pos; // temporary parameters for bathymetry data interpolation
   double *discharge, *concentration; // calculated from flood data
   int i,j,n; // temporary parameters
   double day, angle, spreading_angle, flow_width; // not used in sakura but kept for future modification
   double sum, av_graindensity=0, bulkdensitybottom=0, porosity; // temporary parameters
   FILE *outfp;
   Sak_bottom_t get_phe_data;
   Sak_bathy_t bathy_data;
   Sakura_t sakura_const;

   // open ASCII output file
   if ( (outfp = fopen("sakura_test.txt", "w")) == NULL ) {
      fprintf(stderr, "Err opening output file...EXITING\n");
      eh_exit(-1);
   }
   
   printf("start reading input file\n");
   
   // READ OPTION VALUES
   day       = DEFAULT_DAY_LENGTH;
   angle     = DEFAULT_SPREADING_ANGLE;
   infile    = DEFAULT_INPUT_FILE_NAME;
   outfile   = DEFAULT_OUTPUT_FILE_NAME;
   dischargefile = DEFAULT_DISCHARGE_FILE_NAME;
   bathyfile = DEFAULT_BATHYMETRY_FILE_NAME;
   floodfile = DEFAULT_FLOOD_FILE_NAME;
   datafile  = DEFAULT_DATA_FILE_NAME;
   
   discharge0 = DISCHARGE_FILE0;
   discharge1 = DISCHARGE_FILE1;
   discharge2 = DISCHARGE_FILE2;
   discharge3 = DISCHARGE_FILE3;
   day *= DAY;
   
   // READ INPUT FILE
   fp_in = eh_open_file( infile , "r" );
   read_double_vector( fp_in , &basin_len         , 1 );   
   basin_len *= 1000.;
   read_double_vector( fp_in , &dx                , 1 );
   read_double_vector( fp_in , &dt                , 1 );
   read_double_vector( fp_in , &outtime           , 1 );
   read_double_vector( fp_in , &maxtime           , 1 );
   read_double_vector( fp_in , &densitySeaWater   , 1 );
   read_double_vector( fp_in , &densityRiverWater , 1 );
   read_int_vector   ( fp_in , &nGrains           , 1 );
   
   // MEMORY ALLOCATION for input file
   eh_lambda          = eh_new( double , nGrains );
   diameterEquivalent = eh_new( double , nGrains );
   diameterComponent  = eh_new( double , nGrains );
   fraction           = eh_new( double , nGrains );
   equivalentHeight   = eh_new( double , nGrains );
   bulkDensity        = eh_new( double , nGrains );
   grainDensity       = eh_new( double , nGrains );
   phe_bottom         = eh_new( double , nGrains );
   
   read_double_vector( fp_in , eh_lambda             , nGrains );
   for ( n=0 ; n<nGrains ; n++ )
      eh_lambda[n] /= (10. * DAY);
   read_double_vector( fp_in , diameterEquivalent , nGrains );
   read_double_vector( fp_in , diameterComponent  , nGrains );
   for ( n=0 ; n<nGrains ; n++ ){
      diameterEquivalent[n] /= 1e6;
      diameterComponent[n] /= 1e6;
   }
   read_double_vector( fp_in , fraction           , nGrains );
   read_double_vector( fp_in , equivalentHeight   , nGrains );
   read_double_vector( fp_in , bulkDensity        , nGrains );
   read_double_vector( fp_in , grainDensity       , nGrains );
   read_double_vector( fp_in , &depositionStart   , 1 );
//   depositionStart *= 1000;
   read_double_vector( fp_in , &diameterBottom    , 1 );
   read_double_vector( fp_in , &BulkDensityBottom , 1 );
   read_double_vector( fp_in , phe_bottom         , nGrains );
   read_double_vector( fp_in , &consts.sua        , 1 );
   read_double_vector( fp_in , &consts.sub        , 1 );
   read_double_vector( fp_in , &consts.Ea         , 1 );
   read_double_vector( fp_in , &consts.Eb         , 1 );
   read_double_vector( fp_in , &consts.Cd         , 1 );
   read_double_vector( fp_in , &consts.tanPhi     , 1 );
   consts.tanPhi = tan(consts.tanPhi*M_PI/180.);
   read_double_vector( fp_in , &consts.mu         , 1 );
   consts.mu /= 1e6;
   consts.rhoSW = densitySeaWater;
   consts.rhoRW = densityRiverWater;

   for ( i=0 ; i<nGrains ; i++ )
   {
      av_graindensity   += grainDensity[i]*phe_bottom[i];
      BulkDensityBottom += BulkDensityBottom*phe_bottom[i];
   }
   porosity = 1.0 - bulkdensitybottom/av_graindensity;

   fclose( fp_in );
   /*END of READING INPUT FILE*/
   
   settlingVelocity = eh_new0( double , nGrains );
   reynoldsNumber   = eh_new0( double , nGrains );

   /* Divide the lambdas by the equivalentHeights.  This is added
      to account for different grains occupying different portions
      of the flow height (ie sands mostly near the bottom, clays 
      distributed evenly bottom to top).
   */
   for (n=0;n<nGrains;n++)
      eh_lambda[n] /= equivalentHeight[n];
   
   /* READ FLOOD FILE*/
   printf("start reading flood file\n");
   
   fp_flood = eh_open_file( floodfile , "r" );
   read_int_vector( fp_flood , &flood_days , 1 );
   densityFlow    = eh_new( double , flood_days );
   river_depth    = eh_new( double , flood_days );
   river_width    = eh_new( double , flood_days );
   river_velocity = eh_new( double , flood_days );
   flood_type     = eh_new( int    , flood_days );
   supplytime     = eh_new( double , flood_days );
   
   for ( i=0 ; i<flood_days ; i++ ){
      fgets( comment , S_LINEMAX , fp_flood );
      read_double_vector( fp_flood , &densityFlow[i]    , 1 );
      read_double_vector( fp_flood , &river_depth[i]    , 1 );
      read_double_vector( fp_flood , &river_width[i]    , 1 );
      read_double_vector( fp_flood , &river_velocity[i] , 1 );
      read_int_vector   ( fp_flood , &flood_type[i]     , 1 );
      read_double_vector( fp_flood , &supplytime[i]     , 1 );
      supplytime[i] *= 60.0;
      read_string_value( fp_flood , discharge_filename );
      g_strstrip( discharge_filename );
   }
   fclose(fp_flood);
   /*END of READING FLOOD FILE*/
   
   // READING BATHYMETRY FILE
   printf("start preparing bathymetric data\n"); 
   {
      Eh_data_record* all_records;
      gssize len;
      double* x_0 = eh_uniform_array( dx/2. , basin_len , dx , &len );

      all_records = eh_data_record_scan_file( bathyfile , "," , EH_FAST_DIM_COL , FALSE );
      eh_data_record_interpolate_rows( all_records[0] , 0 , x_0 , len );

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
      eh_free( x_0 );

      n_nodes = len;
   }

   deposit = eh_new_2( double , nGrains , n_nodes );

   // END of READING BATHYMETRY FILE
   
/*
   // generate provisional grid and bathymetry
   X1 = (double *)malloc(48 * sizeof(double));      
   Z1 = (double *)malloc(48 * sizeof(double));      
   S1 = (double *)malloc(48 * sizeof(double));      
   
   bathyC(X1, Z1, S1, dx);
   interpolate2(X1,Z1,48, x, depth, n_nodes, dx);
   slope[0] = 0;
   width[0] = 1.;
   for(i=1; i<n_nodes; i++){
      slope[i] = -(depth[i] - depth[i-1])/dx;
      width[i] = 1.;
   }
   for (i=0; i < 48; i++)
         fprintf(outfp, "%d\t%f\t%f\t%f\n", i, X1[i], Z1[i], S1[i]);
   for (i=0; i < n_nodes; i++)
   fprintf(outfp, "%d\t%f\t%f\t%f\n", i, x[i], depth[i], slope[i]);
   free(X1);
   free(Z1);
   free(S1);
*/

   /* end of provisional grid and bathymetry*/
   
   fp_data = eh_open_file( datafile , "w" );
   fwrite( &n_nodes , 1         , sizeof(int)    , fp_data );
   fwrite( x        , n_nodes-1 , sizeof(double) , fp_data );
   fwrite( depth    , n_nodes-1 , sizeof(double) , fp_data );
   fwrite( width    , n_nodes-1 , sizeof(double) , fp_data );

   for ( i=0 ; i<n_nodes ; i++ )
      fprintf( outfp , "%d, %f, %f\n" , i , width[i] , depth[i] );
   
/*
   for (i=0;i<flood_days;i++){
      // Create a spreading angle.
      width[0] = river_width[i];
      for ( j=1 ; j<n_nodes ; j++ ){
         flow_width = width[j-1] + spreading_angle*dx;
         if ( flow_width < width[j] )
            width[j] = flow_width;
         else
            break;
      }
   }
*/
   
   // INITIALIZE settling velocity
   for (i = 0; i < nGrains; i ++){
      Rden  = grainDensity[i]/consts.rhoRW;
      Dstar = G * Rden * pow(diameterEquivalent[i], 3.0) / pow(consts.mu, 2.0);
      Wstar = -3.76715
            + (1.92944 * log10(Dstar)) 
            - (0.09815 * pow(log10(Dstar), 2.0))
            - (0.00575 * pow(log10(Dstar), 3.0))
            + (0.00056 * pow(log10(Dstar), 4.0));
      Wstar = pow(10.0, Wstar);
      settlingVelocity[i] = pow((Rden * G * consts.mu * Wstar), 0.33333); 
      reynoldsNumber[i]   = sqrt(Rden * G * pow(diameterEquivalent[i], 3.0))
                          / consts.mu;
   }
   
   // set up the query function.  these functions are used to communicate
   // the the architecture.
   get_phe_data.phe_bottom = phe_bottom;
   get_phe_data.n_grains   = nGrains;
   bathy_data.x            = x;
   bathy_data.depth        = depth;
   bathy_data.slope        = slope;
   bathy_data.n_nodes      = n_nodes;
   bathy_data.x            = x;
   consts.get_phe_data     = (void*)&get_phe_data;
   consts.add_data         = (void*)&bathy_data;
   consts.remove_data      = (void*)&bathy_data;
   consts.depth_data       = (void*)&bathy_data;
   consts.get_phe          = (Eh_query_func)&sakura_get_phe;
   consts.add              = (Eh_query_func)&sakura_add;
   consts.remove           = (Eh_query_func)&sakura_remove;
   consts.get_depth        = (Eh_query_func)&sakura_get_depth;
   consts.set_depth        = (Eh_query_func)&sakura_set_depth;

   final = eh_new_2( double , flood_days , n_nodes-1 );

   for (i = 0; i<flood_days;i++)
   {
      width[0] = river_width[i];

      {
         gssize t_len;
         double* t_0 = eh_uniform_array( 0 , supplytime[i] , dt , &t_len );
         Eh_data_record* all_records = eh_data_record_scan_file( discharge_filename , "," , EH_FAST_DIM_COL , FALSE );

         eh_data_record_interpolate_rows( all_records[0] , 0 , t_0 , t_len );
         t      = eh_data_record_dup_row( all_records[0] , 0 );
         init_u = eh_data_record_dup_row( all_records[0] , 1 );
         init_c = eh_data_record_dup_row( all_records[0] , 2 );

         eh_free( t_0 );
         for ( j=0 ; all_records[j] ; j++ )
            eh_data_record_destroy( all_records[j] );
         eh_free( all_records );
      }
/*
      t_array = g_array_new( FALSE , FALSE , sizeof(double) );
      for ( t_pos=0 ; t_pos<supplytime[i] ; t_pos+=dt )
         g_array_append_val( t_array , t_pos );

      fp_discharge = eh_open_data_file( (const char*) discharge_filename ,
                                        &attr );
      data2 = eh_get_data_from_file( fp_discharge , 0 );
      eh_interpolate_data_record_rows( data2 , 0 , t_array );
      t      = eh_get_data_record_row_ptr( data2 , 0 , double );
      init_u = eh_get_data_record_row_ptr( data2 , 1 , double );
      init_c = eh_get_data_record_row_ptr( data2 , 2 , double );
*/
      sakura( dx             , dt               , basin_len        ,
              n_nodes        , nGrains          , x                ,
              depth          , width            , init_u           ,
              init_c         , eh_lambda        , settlingVelocity ,
              reynoldsNumber , grainDensity     , river_depth[i]   ,
              supplytime[i]  , depositionStart  , fraction         ,
              phe_bottom     , bulkDensity      , outtime          ,
              consts         , deposit          , fp_data );

      for ( j=0 ; j<n_nodes-1 ; j++ )
      {
         for ( n=0 , sum=0 ; n<nGrains ; n++ )
            sum += deposit[n][j];
         depth[j] += sum;
         final[i][j] = depth[j];
      }

      eh_free( t );
      eh_free( init_u );
      eh_free( init_c );
   }

   for ( j=0 ; j<n_nodes-1 ; j++ )
   {
      fprintf( outfp , "%f," , x[j] );
      for ( i=0 ; i<flood_days ; i++ )
         fprintf( outfp , "%f," , final[i][j] );
      fprintf( outfp , "\n" );
   }
   
//   for (n=0;n<nGrains;n++)
//      fwrite(deposit[n],n_nodes-1,sizeof(double),fp_data);
   fclose( fp_data );

   // write the new bathymetry.  this is written in a form so that it can
   // be used as input for another sakura run.
   fp_out = eh_open_file( outfile , "w" );
   fprintf( fp_out , "--- 'data' ---\n" );
   for (i=0;i<n_nodes;i++)
   {
      for (n=0,sum=0;n<nGrains;n++)
            sum += deposit[n][i];
      fprintf(fp_out,"%f, %f\n",x[i],depth[i]+sum,width[i]);
   }
   fclose( fp_out  );
   
   fclose( outfp  );

   eh_free( river_velocity     );
   eh_free( river_depth        );
   eh_free( river_width        );
   eh_free( densityFlow        );
   eh_free( discharge          );
   eh_free( phe_bottom         );
   eh_free( bulkDensity        );
   eh_free( fraction           );
   eh_free( diameterComponent  );
   eh_free( diameterEquivalent );
   eh_free( eh_lambda          );
   eh_free( equivalentHeight   );
   eh_free( grainDensity       );
   eh_free( settlingVelocity   );
   eh_free( reynoldsNumber     );
   eh_free( concentration      );
   eh_free( equivalentHeight   );
   eh_free( grainDensity       );
   eh_free( x                  );
   eh_free( depth              );
   eh_free( width              );
   eh_free( slope              );
   eh_free_2( final   );
   eh_free_2( deposit );
   
   return 0;
   
} // end of main

/** @memo Get the grain size distribution of bottom sediments.

@doc Get the fractions of each grain type of bottom sediments from a
Sed_cube.  This function is intended to be used within another program that
needs to communicate with the sedflux architecture but is otherwise separate
from sedflux.

Note that the member, eroded_depth may be changed to reflect the actual amount
of bottom sediment available to be eroded.  That is, we may be trying to erode
more sediment than is actually present.

In this case, the data pointer should point to a Sakura_get_phe_t structure
that contains the grain size distribution and the number of grains.  This
information is simply copied to the location pointed to by phe.

@param phe  The fraction of each grain type in the bottom sediments.
@param data A structure that contains the necessary data for the function to
            retreive the grain type fracitons.

@return A pointer to the array of grain type fractions.

*/
void sakura_get_phe( gpointer user_data , gpointer data )
{
   double *phe_out  = EH_STRUCT_MEMBER( Sak_phe_query_t , user_data , phe );
   double *phe_bot  = EH_STRUCT_MEMBER( Sak_bottom_t , data , phe_bottom );
   int     n_grains = EH_STRUCT_MEMBER( Sak_bottom_t , data , n_grains   );

   memcpy( phe_out , phe_bot , sizeof(double)*n_grains );

   return;
}

void sakura_remove( gpointer remove_query , gpointer data )
{
   double remove = EH_STRUCT_MEMBER( Sak_erode_query_t , remove_query , dh );
   int    i      = EH_STRUCT_MEMBER( Sak_erode_query_t , remove_query , i  );

   double *x       = EH_STRUCT_MEMBER( Sak_bathy_t , data , x       );
   double *depth   = EH_STRUCT_MEMBER( Sak_bathy_t , data , depth   );
   double *slope   = EH_STRUCT_MEMBER( Sak_bathy_t , data , slope   );
   int     n_nodes = EH_STRUCT_MEMBER( Sak_bathy_t , data , n_nodes );

   remove = (remove>0)?remove:-remove;
   depth[i] -= remove;

/*
   if ( i==n_nodes-1 )
      slope[i]   = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1])); 
   else
      slope[i]   = atan( (depth[i+1]-depth[i])/(x[i+1]-x[i]));

   if ( i>0 )
      slope[i-1] = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
*/
   return;
}

void sakura_add( gpointer add_query , gpointer data )
{
   double add = EH_STRUCT_MEMBER( Sak_add_query_t , add_query , dh );
   int    i   = EH_STRUCT_MEMBER( Sak_add_query_t , add_query , i  );
   double *x       = EH_STRUCT_MEMBER( Sak_bathy_t , data , x       );
   double *depth   = EH_STRUCT_MEMBER( Sak_bathy_t , data , depth   );
   double *slope   = EH_STRUCT_MEMBER( Sak_bathy_t , data , slope   );
   int     n_nodes = EH_STRUCT_MEMBER( Sak_bathy_t , data , n_nodes );

   add = (add>0)?add:-add;
   depth[i] += add;

/*
   if ( i==n_nodes-1 )
      slope[i]   = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1])); 
   else
      slope[i]   = atan( (depth[i+1]-depth[i])/(x[i+1]-x[i]));

   if ( i>0 )
      slope[i-1] = atan( (depth[i]-depth[i-1])/(x[i]-x[i-1]));
*/

   return;
}

void sakura_get_depth( gpointer depth_query , gpointer data )
{
   int    i   = EH_STRUCT_MEMBER( Sak_depth_query_t , depth_query , i );
   double *depth   = EH_STRUCT_MEMBER( Sak_bathy_t , data , depth   );

   EH_STRUCT_MEMBER( Sak_depth_query_t , depth_query , depth ) = depth[i];

   return;
}

void sakura_set_depth( gpointer depth_query , gpointer data )
{
   int    i         = EH_STRUCT_MEMBER( Sak_depth_query_t ,
                                        depth_query ,
                                        i  );
   double new_depth = EH_STRUCT_MEMBER( Sak_depth_query_t ,
                                        depth_query ,
                                        depth );
   double *depth   = EH_STRUCT_MEMBER( Sak_bathy_t , data , depth   );

   depth[i] = new_depth;

   return;
}





