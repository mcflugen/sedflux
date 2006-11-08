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

#ifndef INFLOW_IS_INCLUDED
# define INFLOW_IS_INCLUDED

#ifndef DAY
# define DAY (86400.)
#endif

#define TURBIDITY_CURRENT_DEFAULT_EA                (0.00153)
#define TURBIDITY_CURRENT_DEFAULT_EB                (0.00204)
#define TURBIDITY_CURRENT_DEFAULT_SUA               (400.)
#define TURBIDITY_CURRENT_DEFAULT_SUB               (2.0)
#define TURBIDITY_CURRENT_DEFAULT_CD                (0.004)
#define TURBIDITY_CURRENT_DEFAULT_TAN_PHI           (0.36397023426620)
#define TURBIDITY_CURRENT_DEFAULT_MU                (1.3e-6)
#define TURBIDITY_CURRENT_DEFAULT_DENSITY_SEA_WATER (1028.)
#define TURBIDITY_CURRENT_DEFAULT_CHANNEL_WIDTH     (100.0)
#define TURBIDITY_CURRENT_DEFAULT_CHANNEL_LENGTH    (30000.0)

#include <glib.h>

typedef void (*Sed_query_func) ( gpointer user_data , gpointer data );

typedef struct
{
   double Ea;
   double Eb;
   double sua;
   double sub;
   double Cd;
   double tanPhi;
   double mu;
   double rhoSW;
   double channelWidth;
   double channelLength;

   gpointer get_phe_data;
   gpointer get_depth_data;
   gpointer remove_data;
   gpointer add_data;
   Sed_query_func get_phe;
   Sed_query_func get_depth;
   Sed_query_func remove;
   Sed_query_func add;
}
Inflow_t;

typedef struct
{
   double *phe_bottom;
   int n_grains;
}
I_bottom_t;

typedef struct
{
   double x;
   double dx;
   double erode_depth;
   double *phe;
}
I_phe_query_t;

int inflow( double day       , double x[]        , double slopeX[]  ,
            double wX[]      , int nNodes        , double dx        , 
            double xDep      , double riverWidth , double u0        ,
            double h0        , double dc         , double *gzF      ,
            double *grainDia , double *lambda    , double *rhoSed   ,
            double *rhoGrain , int nGrains       , double rho0      ,
            double rhoF0     , Inflow_t consts   , double **deposit ,
            FILE *fpout );

#endif /* inflow.h is included */
