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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sed/sed_sedflux.h>

/*** Self Documentation ***/
char *help_msg[] =
{
"                                                                             ",
" bing [options] [parameters]  [filein]                                       ",
"  run a bingham plastic debris flow.                                         ",
"                                                                             ",
" Options                                                                     ",
"  -o outfile  - send output to outfile instead of stdout. [stdout]           ",
"  -v          - be verbose. [off]                                            ",
"  -h          - print this help message.                                     ",
"                                                                             ",
" Parameters                                                                  ",
"  -pdt=value  - Use a time step of value seconds. [ .01 ]                    ",
"  -pnu=value  - Use a viscosity value of value m^2/s for the flow. [ .00083 ]",
"  -pnuart=value - Use a numerical viscosity value of value m^2/s for the     ",
"                  flow. [ .01 ]                                              ",
"  -pyield=value - Use a yield strength of value N/m^2. [ 100. ]              ",
"  -prho=value - Use a flow density of value kg/m^3. [ 1500. ]                ",
"  -pend=value - Stop the simulation after value minutes. [ 60. ]             ",
"  -pint=value - Write to output file every value seconds [ 120 ]             ",
"                                                                             ",
" Files                                                                       ",
"  Input File                                                                 ",
"   filein      -- Read bathymetry data from filein. [ stdin ]                ",
"    Bathymetry is defined by x-y pairs with each point given on a new line.  ",
"    Values are in meters from an arbitrary origin.  Comment lines are allowed",
"    and are defined as any lines not beginning with a number.                ",
"                                                                             ",
"  Output File:                                                               ",
"    Output is binary and gives flow characteristics for each node of the flow",
"    at regular time intervals.  A header is written, then the data.          ",
"    --The header--                                                           ",
"    The number of points defining the profile - 4 byte int                   ",
"    X-coordinates of profile nodes            - 8 byte doubles               ",
"    Y-coordinates of profile nodes            - 8 byte doubles               ",
"    The number of nodes defining the flow     - 4 byte int                   ",
"    --The body--                                                             ",
"    Flag indicating that there is more data   - 8 byte double                ",
"    X-coordinate of each node                 - 8 byte doubles               ",
"    Thickness of flow at each node            - 8 byte doubles               ",
"    Velocity of flow at each node             - 8 byte doubles               ",
"    Y-coordinate of base of flow at each node - 8 byte doubles               ",
"                                                                             ",
NULL
};

#include <utils/utils.h>
#include "bing.h"

/* int ReadFloor(FILE *,double **,double **); */
/*
int bing(double[],int,int,double,double*,double*,double,double,
   double,double,double,double,int,double[]);
*/

#ifndef EPS
# define EPS 1e-5
#endif

#define NNODES                  7     /* number of nodes defining flow */
#define nxpts                   1000    /* array lengths */

#define D_MAX                   4.0
#define INIT_LEN                1000.0 /* inital length of debris flow (m) */
#define FLOW_START              10     /* index to node of tail of flow */

/* Default values */
#define DEFAULT_VERBOSE             0
#define DEFAULT_IN_FILE             stdin
#define DEFAULT_OUT_FILE            stdout
#define DEFAULT_IN_FILE_NAME        "stdin"
#define DEFAULT_OUT_FILE_NAME       "stdout"
#define DEFAULT_WRITE_INTERVAL      120
#define DEFAULT_END_TIME            60.0
#define DEFAULT_SLOPE               2.0
#define DEFAULT_VISCOSITY           0.00083
#define DEFAULT_NUMERICAL_VISCOSITY 0.01
#define DEFAULT_YIELD_STRENGTH      100.0
#define DEFAULT_TIME_STEP           .01
#define DEFAULT_FLOW_DENSITY        1500.0

#define FLOOR_LENGTH 100000.

int main(int argc, char *argv[]) 
{
   Eh_args *args;
   int write_interval;
   double p_mud, tau_y, nu, nu_ar;
   double end_time, dt;
   FILE *fpin, *fpout;
   char *infile, *outfile;
   gboolean verbose;
   double dx;
   int i;
   double Sj;
   double *deposit;
   int const nodes=NNODES-1;
   bing_t bing_const;
   pos_t *bathy, *flow;
   
   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit( -1 );

   write_interval = eh_get_opt_int ( args , "int"   , DEFAULT_WRITE_INTERVAL );
   end_time       = eh_get_opt_dbl ( args , "end"   , DEFAULT_END_TIME       );
   p_mud          = eh_get_opt_dbl ( args , "rho"   , DEFAULT_FLOW_DENSITY   );
   tau_y          = eh_get_opt_dbl ( args , "yield" , DEFAULT_YIELD_STRENGTH );
   nu             = eh_get_opt_dbl ( args , "nu"    , DEFAULT_VISCOSITY      );
   nu_ar          = eh_get_opt_dbl ( args , "nuart" , DEFAULT_NUMERICAL_VISCOSITY );
   dt             = eh_get_opt_dbl ( args , "dt"    , DEFAULT_TIME_STEP      );
   verbose        = eh_get_opt_bool( args , "v"     , DEFAULT_VERBOSE        );
   infile         = eh_get_opt_str ( args , "in"    , DEFAULT_IN_FILE_NAME   );
   outfile        = eh_get_opt_str ( args , "out"   , DEFAULT_OUT_FILE_NAME  );

   if ( strcmp( infile , "stdin" )==0 )
      fpin = stdin;
   else
      fpin = eh_fopen( infile , "r" );
   if ( strcmp( outfile , "stdout" )==0 )
      fpout = stdout;
   else
      fpout = eh_fopen( outfile , "w" );

   /* convert output interval to iterations.
   */
   write_interval = (int) (write_interval / dt);
   
   if ( verbose )
   {
      fprintf(stderr,"\nBingham Plastic Debris Flow Model.\n\n");
      fprintf(stderr,"\tType 'bing help' for help.\n\n");
   
      fprintf(stderr,"Bing is go ...\n");
      fprintf(stderr,"\tSlope          = reading from %s\n",infile);
      fprintf(stderr,"\tTime step      = %.5f seconds\n",dt);
      fprintf(stderr,"\tViscosity      = %.5f m^2/s\n",nu);
      fprintf(stderr,"\tYield Strength = %.5f N/m^2\n",tau_y);
      fprintf(stderr,"\tDensity        = %.5f kg/m^3\n",p_mud);
      fprintf(stderr,"\tEnd Time       = %.5f minutes\n",end_time);
      fprintf(stderr,"\tPrinting every = %.5f minutes\n",write_interval*dt/60.);
   }

   Sj = tan(Sj*PI/180.);

/**
*** Read in bathymetry 
**/   
/*   len=ReadFloor(fpin,&x,&h); */
   if ( verbose ) fprintf( stderr , "reading bathymetry...\n" );

   dx = 10;
   bathy = createPosVec((int)(FLOOR_LENGTH/dx));
   for (i=0;i<bathy->size;i++)
      bathy->x[i] = i*dx;
//   ReadFloor(fpin,bathy->x,bathy->size,bathy->y);
   sed_get_floor_vec(infile,bathy->x,bathy->size,bathy->y , NULL );
   for (i=0;i<bathy->size;i++)
      bathy->y[i] *= -1.;

/* Write header to output file */
   if ( verbose ) fprintf( stderr , "writing output header...\n" );

   fwrite(&bathy->size,sizeof(bathy->size),1,fpout);
   fwrite(bathy->x,sizeof(double),bathy->size,fpout);
   fwrite(bathy->y,sizeof(double),bathy->size,fpout);
   fwrite(&nodes,sizeof(int),1,fpout);

/**
*** Set initial shape of debris flow 
**/
   flow = createPosVec(NNODES);
   for (i=0;i<NNODES;i++) 
   {
      float DX = INIT_LEN/(NNODES-1);
      float width = DX*(NNODES-1)/2.;
      
/**
*** Parabola: 
**/     
      flow->y[i] = (DX*(i-(NNODES-1)/2.+.5)-width)*(DX*(i-(NNODES-1)/2.+.5)+width)/(-width*width)*D_MAX;
      flow->x[i] = bathy->x[FLOW_START] + DX*i;
   }

   deposit = eh_new( double , bathy->size );
   
   bing_const.yieldStrength = tau_y;
   bing_const.viscosity = nu;
   bing_const.numericalViscosity = nu_ar;
   bing_const.flowDensity = p_mud;
   bing_const.dt = dt;
   bing_const.maxTime = end_time;
   bing_const.MC = write_interval;

   if ( verbose ) fprintf( stderr , "running the debris flow...\n" );
   bing(bathy,flow,bing_const,deposit);
/*
   bing(h,len,NNODES,dx,nodeX,nodeH,tau_y,nu,nu_ar,p_mud,dt,end_time,write_interval,deposit);
*/

   if ( verbose ) fprintf( stderr , "writing output...\n" );
   fwrite(deposit,bathy->size,sizeof(double),fpout);

   fclose(fpin);
   fclose(fpout);

   destroyPosVec(flow);
   destroyPosVec(bathy);
   eh_free(deposit);
   
   return 0;

}

