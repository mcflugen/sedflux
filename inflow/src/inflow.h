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


/*
// Old defaults.
#define INFLOW_DEFAULT_SUA                      (400.)
#define INFLOW_DEFAULT_SUB                      (2.0)
#define INFLOW_DEFAULT_EA                (0.00153)
#define INFLOW_DEFAULT_EB                (0.00204)
#define INFLOW_DEFAULT_CD                (0.004)
#define INFLOW_DEFAULT_TAN_PHI           (0.36397023426620)
#define INFLOW_DEFAULT_MU                (1.3e-6)
#define INFLOW_DEFAULT_DENSITY_SEA_WATER (1028.)
#define INFLOW_DEFAULT_CHANNEL_WIDTH     (100.0)
#define INFLOW_DEFAULT_CHANNEL_LENGTH    (30000.0)
*/
#include <stdio.h>
#include <glib.h>
#include "sed_sedflux.h"

#include "datadir_path.h"
#if !defined( DATADIR )
# define DATADIR "/usr/local/share"
#endif
#define INFLOW_TEST_PARAM_FILE DATADIR "/ew/inflow_param.kvf"
#define INFLOW_TEST_BATHY_FILE DATADIR "/ew/inflow_bathy.csv"
#define INFLOW_TEST_FLOOD_FILE DATADIR "/ew/inflow_flood.kvf"

typedef enum
{
   INFLOW_ERROR_BAD_PARAMETER
}
Inflow_error;

#define INFLOW_ERROR inflow_error_quark()

typedef struct
{
   double x;            //< Position to query
   double dx;           //< Grid spacing
   double erode_depth;  //< Burial depth to query
   double *phe;         //< Fraction of each grain size
}
Inflow_phe_query_st;

typedef void (*Inflow_query_func) ( Inflow_phe_query_st* data , gpointer user_data );

typedef struct
{
   double e_a;
   double e_b;
   double sua;
   double sub;
   double c_drag;
   double tan_phi;
   double mu_water;
   double rho_river_water;
   double rho_sea_water;
   double channel_width;
   double channel_len;
   double dep_start;

   Inflow_query_func get_phe;
   gpointer          get_phe_data;
}
Inflow_const_st;

gboolean inflow( double day       , double x[]        , double slopeX[]  ,
                 double wX[]      , int nNodes        , double dx        , 
                 double xDep      , double riverWidth , double u0        ,
                 double h0        , double dc         , double *gzF      ,
                 double *grainDia , double *lambda    , double *rhoSed   ,
                 double *rhoGrain , int nGrains       , double rho0      ,
                 double rhoF0     , Inflow_const_st* c, double **deposit ,
                 double** eroded  , FILE *fpout );
Sed_hydro inflow_flood_from_cell( Sed_cell c , double area );
gboolean  sed_inflow( Sed_cube         p       ,
                      Sed_hydro        f       ,
                      gint             i_start ,
                      double           dx      ,
                      Inflow_const_st* c );

#endif /* inflow.h is included */
