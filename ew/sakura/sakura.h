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

#ifndef SAKURA_IS_INCLUDED
# define SAKURA_IS_INCLUDED

#include <stdio.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#include <sed/datadir_path.h>
#if !defined( DATADIR )
# define DATADIR "/usr/local/share"
#endif
#define SAKURA_TEST_PARAM_FILE DATADIR "/ew/sakura_param.kvf"
#define SAKURA_TEST_BATHY_FILE DATADIR "/ew/sakura_bathy.csv"
#define SAKURA_TEST_FLOOD_FILE DATADIR "/ew/sakura_flood.kvf"

typedef enum
{
   SAKURA_ERROR_BAD_PARAMETER
}
Sakura_error;

typedef enum
{
   SAKURA_VAR_BAD_VAR  = 0 ,
   SAKURA_VAR_VELOCITY = 1 ,
   SAKURA_VAR_DEPTH    = 2 ,
   SAKURA_VAR_WIDTH    = 3
}
Sakura_var;

#define SAKURA_ERROR sakura_error_quark()
/*
typedef struct
{
   double  x;            //< Position to query
   double  dx;           //< Grid spacing
   double  erode_depth;  //< Burial depth to query
   double *phe;         //< Fraction of each grain size
}
Sakura_phe_query_st;

typedef void (*Sakura_query_func) ( Sakura_phe_query_st* data , gpointer user_data );
*/
typedef struct
{
   double* phe;
   gint    n_grains;
   double  val;
}
Sakura_phe_st;

typedef struct
{
   gint   id; //< Id that identifies the sediment type of the cell
   double t;  //< The amount of sediment in the cell
}
Sakura_cell_st;

typedef double (*Sakura_phe_func) ( gpointer data , double x , Sakura_phe_st* s  );
typedef double (*Sakura_add_func) ( gpointer data , double x , Sakura_cell_st* s );
typedef double (*Sakura_get_func) ( gpointer data , double x );

typedef struct
{
   double dt;              ///< Model timestep in seconds

   double e_a;             ///< Entrainment coefficient, a
   double e_b;             ///< Entrainment coefficient, b
   double sua;             ///< Gradient of bottom sediment shear strength
   double sub;             ///< Bottom sediment shear strength at seafloor
   double c_drag;          ///< Drag coefficient
   double rho_river_water; ///< Density of river water
   double rho_sea_water;   ///< Density of sea water

   double tan_phi;
   double mu_water;
   double channel_width;
   double channel_len;
   double dep_start;

   Sakura_var*  data_id;
   FILE*        data_fp;
   gint         data_int;

   Sakura_phe_func get_phe;
   Sakura_add_func add;
   Sakura_add_func remove;
   Sakura_get_func get_depth;

   gpointer        get_phe_data;
   gpointer        add_data;
   gpointer        remove_data;
   gpointer        depth_data;
}
Sakura_const_st;

/*
gboolean sakura ( double dx         , double dt              , double basin_Len ,
                  int n_nodes       , int n_grains           , double* x      ,
                  double* depth     , double *width            , double Init_U[]  ,
                  double Init_C[]   , double *lambda         , double *Stv      ,
                  double *Rey       , double *rho_grain          , double init_h     ,
                  double supply_time , double dep_start , double *fraction ,
                  double *phe_bot , double *rho_bot , double OutTime   ,
                  Sakura_const_st* c   , double **deposit       , FILE *fp_data );
*/
double**
sakura( double  u_riv     , double  c_riv    , double  h_riv  , double* f_riv    ,
        double  dt        , double  duration ,
        double* x         , double* z        , double* w      , gint    len      ,
        double* rho_grain , double* rho_dep  , double* u_fall , gint    n_grains ,
        Sakura_const_st* c );
Sed_hydro sakura_flood_from_cell( Sed_cell c , double area );
gboolean  sed_sakura( Sed_cube         p       ,
                      Sed_hydro        f       ,
                      gint             i_start ,
                      double           dx      ,
                      Sakura_const_st* c );

#endif /* sakura.h is included */
