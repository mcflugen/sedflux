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

#ifndef SAKURA_LOCAL_IS_INCLUDED
# define SAKURA_LOCAL_IS_INCLUDED

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
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "sakura.h"
#include "sakura_utils.h"

// LIST OF FUNCTIONS defined only for sakura
// define constants
#define HMIN 0.0000001
#define UPPERLIMIT 20

/** @name Physical Constants */
//@{
/// gravitational acceleration (m/s/s)
#define G (9.81)

/// submerged specific gravity of quartz
#define R (1.65)

/// near-bed bulk concentration ratio
#define Ro (2.0)

/// Length of a day in seconds.
# define DAY (86400.)
//@}

typedef struct
{
   double* rho_grain;   //< Grain density
   double* rho_dep;     //< Bulk deposit density
   double* u_settling;  //< Settling velocity
   gint    len;         //< Number of grain types
}
Sakura_sediment;

typedef struct
{
   double  u;       //< Flow velocity
   double  h;       //< Flow height
   double  c;       //< Flow concentration
   double* c_grain; //< Flow concentration for each grain type
   gint    n_grain; //< Number of grain types
}
Sakura_node;

typedef struct
{
   double*  x;       //< Node positions
   double*  w;       //< Node widths
   double*  h;       //< Node flow heights
   double*  u;       //< Node flow velocities
   double*  c;       //< Node concentrations
   double** c_grain; //< Node concentrations for each grain type

   double** d;       //< Total deposition at each node
   double** e;       //< Total erosion at each node

   gint     len;     //< Number of nodes
   gint     n_grain; //< Number of grain types
}
Sakura_array;

Sakura_sediment* sakura_sediment_new           ( gint n_grains );
Sakura_sediment* sakura_sediment_destroy       ( Sakura_sediment* s );
Sakura_sediment* sakura_sediment_set_rho_grain ( Sakura_sediment* s , double* x );
Sakura_sediment* sakura_sediment_set_rho_dep   ( Sakura_sediment* s , double* x );
Sakura_sediment* sakura_sediment_set_u_settling( Sakura_sediment* s , double* x );

Sakura_array* sakura_array_new    ( gint len , gint n_grain );
Sakura_array* sakura_array_destroy( Sakura_array* a );
Sakura_array* sakura_array_copy   ( Sakura_array* d , Sakura_array* s );
Sakura_array* sakura_array_set_x  ( Sakura_array* a , double* x );
Sakura_array* sakura_array_set_w  ( Sakura_array* a , double* w );
Sakura_array* sakura_array_set_bc ( Sakura_array* a , Sakura_node* inflow , Sakura_node* outflow );
double        sakura_array_mass_in_susp  ( Sakura_array* a , Sakura_sediment* s );
double        sakura_array_mass_eroded   ( Sakura_array* a , Sakura_sediment* s );
double        sakura_array_mass_deposited( Sakura_array* a , Sakura_sediment* s );

Sakura_node* sakura_node_new    (                  double u , double c , double h , double* c_grain , gint len );
Sakura_node* sakura_node_set    ( Sakura_node* x , double u , double c , double h , double* c_grain , gint len );
Sakura_node* sakura_node_destroy( Sakura_node* x );

gboolean sakura_set_outflow( Sakura_node* out , Sakura_array* a , double x_head , double dt , double dx );
double sakura_get_sin_slope( Sakura_get_func f , gpointer data , Sakura_array* a , gint i );
gboolean calculate_mid_vel( Sakura_array* a_mid , Sakura_array* a , gint ind_head , Sakura_const_st* Const );
gboolean calculate_next_vel( Sakura_array* a_last , Sakura_array* a_mid , Sakura_array* a_next , gint ind_head , Sakura_const_st* Const );
gboolean compute_c_grain( Sakura_array* a , Sakura_array* a_last , double* u , gint i , double dx , Sakura_const_st* Const , Sakura_sediment* sed );
gboolean calculate_next_c_and_h( Sakura_array* a_new , Sakura_array* a_last , double* u_temp , gint ind_head , Sakura_const_st* Const , Sakura_sediment* sed );
gboolean calculate_mid_c_and_h( Sakura_array* a_mid , Sakura_array* a_last , Sakura_array* a_next );
gint calculate_head_index( Sakura_array* a , double* u , gint ind_head , double dx , double dt , double* x_head );

typedef struct
{
   double*   x;
   double*   depth;
   double*   width;
   double*   slope;
   double**  dep;
   gint      n_grains;
   gint      len;
   double    dx;
}
Sakura_bathy_st;

typedef struct
{
   double  duration;
   double  width;
   double  depth;
   double  velocity;
   double  q;
   double* fraction;
   double  rho_flow;
   gint    n_grains;
}
Sakura_flood_st;

typedef struct
{
   double* size_equiv;
   double* lambda;
   double* bulk_density;
   double* grain_density;
   double* u_settling;
   double* reynolds_no;
   gint    n_grains;
}
Sakura_sediment_st;

typedef struct
{
   Sakura_bathy_st* b;
   double*          phe;
   gint             n_grains;
}
Sakura_arch_st;

#define SAKURA_DEFAULT_BASIN_LEN                (80.)
#define SAKURA_DEFAULT_DX                       (100.)
#define SAKURA_DEFAULT_DT                       (30.)
#define SAKURA_DEFAULT_OUT_TIME                 (3600.)
#define SAKURA_DEFAULT_MAX_TIME                 (86400.)
#define SAKURA_DEFAULT_RHO_SEA_WATER            (1028.)
#define SAKURA_DEFAULT_RHO_RIVER_WATER          (1028.)

#define SAKURA_DEFAULT_LAMBDA                   { 25. , 16.8 , 9. , 3.2 , 2.4 }
#define SAKURA_DEFAULT_SIZE_EQUIV               { 202. , 105. , 69. , 25. , 10. }
#define SAKURA_DEFAULT_SIZE_COMP                { 150. , 50. , 25. , 5. , 1. }
#define SAKURA_DEFAULT_GRAIN_FRACTION           { 0. , .1 , .2 , .3 , .4 }
#define SAKURA_DEFAULT_FLOW_FRACTION            { 1. , 1. , 1. , 1. , 1. }
#define SAKURA_DEFAULT_BULK_DENSITY             { 1850. , 1600. , 1400. , 1300. , 1200. }
#define SAKURA_DEFAULT_GRAIN_DENSITY            { 2650. , 2650. , 2650. , 2650. , 2650. }
#define SAKURA_DEFAULT_DEP_START                (2.)
#define SAKURA_DEFAULT_SIZE_BOTTOM              (64.)
#define SAKURA_DEFAULT_BULK_DENSITY_BOTTOM      (1600.)
#define SAKURA_DEFAULT_BOTTOM_FRACTION          { .2 , .2 , .2 , .2 , .2 }
#define SAKURA_DEFAULT_N_GRAINS                 (5)

#define SAKURA_DEFAULT_SUA                      (30.)
#define SAKURA_DEFAULT_SUB                      (.5)
#define SAKURA_DEFAULT_EA                       (0.00153)
#define SAKURA_DEFAULT_EB                       (0.0204)
#define SAKURA_DEFAULT_CD                       (0.004)
#define SAKURA_DEFAULT_FRICTION_ANGLE           (20.)
#define SAKURA_DEFAULT_MU_WATER                 (1.3)

#define SAKURA_DEFAULT_FLOOD_FILE               "sakura_flood.kvf"

typedef struct
{
   double  basin_len;
   double  dx;
   double  dt;
   double  out_dt;
   double  max_t;
   double  rho_sea_water;
   double  rho_river_water;
   double* lambda;
   double* size_equiv;
   double* size_comp;
   double* grain_fraction;
   double* flow_fraction;
   double* bulk_density;
   double* grain_density;
   double  dep_start;
   double  size_bottom;
   double  rho_bottom;
   double* bottom_fraction;
   double  sua;
   double  sub;
   double  e_a;
   double  e_b;
   double  c_drag;
   double  tan_phi;
   double  mu_water;
   char*   flood_file;
   gint    n_grains;

   double channel_width;
   double channel_len;
}
Sakura_param_st;

double** sakura_wrapper( Sakura_bathy_st*    b ,
                         Sakura_flood_st*    f ,
                         Sakura_sediment_st* s ,
                         Sakura_const_st*    c ,
                         gint* n_grains        ,
                         gint* len );

void sakura_set_width( Sakura_bathy_st* bathy_data  ,
                       double           river_width ,
                       double           spreading_angle );

Sakura_param_st*    sakura_scan_parameter_file( const gchar* file , GError** error );
Sakura_param_st*    sakura_check_params       ( Sakura_param_st* p , GError** error );
Sakura_bathy_st*    sakura_scan_bathy_file    ( const gchar* file , Sakura_param_st* p , GError** error );
Sakura_flood_st**   sakura_scan_flood_file    ( const gchar* file , Sakura_param_st* p , GError** error );
Sakura_flood_st*    sakura_set_flood_data     ( Sed_hydro h , double rho_river_water );
Sakura_flood_st*    sakura_sed_set_flood_data ( Sed_hydro h , double rho_river_water );
Sakura_flood_st*    sakura_destroy_flood_data ( Sakura_flood_st* f );
Sakura_sediment_st* sakura_set_sediment_data  ( Sakura_param_st* p );
Sakura_const_st*    sakura_set_constant_data  ( Sakura_param_st* p , Sakura_bathy_st* b );
Sakura_const_st*    sakura_set_constant_output_data( Sakura_const_st* c , const gchar* file , gint* id , gint dt );
Sakura_bathy_st*    sakura_set_bathy_data     ( double** bathy , gint len , double dx , gint n_grains );
Sakura_bathy_st*    sakura_new_bathy_data     ( gint n_grains , gint len );
Sakura_bathy_st*    sakura_copy_bathy_data    ( Sakura_bathy_st* d , const Sakura_bathy_st* s );
Sakura_bathy_st*    sakura_destroy_bathy_data ( Sakura_bathy_st* b );
Sakura_bathy_st*    sakura_update_bathy_data  ( Sakura_bathy_st* b , double** deposition , double** erosion , gint n_grains );
gint                sakura_write_data         ( const gchar*     file , Eh_dbl_grid deposit );
gint                sakura_write_output       ( const gchar* file  ,
                                                Sakura_bathy_st* b ,
                                                double** deposit   ,
                                                gssize n_grains );
double sakura_reynolds_number( double rho_grain , double equiv_dia , double rho_river_water , double mu_river_water );
double sakura_settling_velocity( double rho_grain , double equiv_dia , double rho_river_water , double mu_river_water );

void   sakura_get_phe  ( Sakura_arch_st* data , double x , Sakura_phe_st*  phe_data );
double sakura_add      ( Sakura_arch_st* data , double x , Sakura_cell_st* s );
double sakura_remove   ( Sakura_arch_st* data , double x , Sakura_cell_st* s );
double sakura_get_depth( Sakura_arch_st* data , double x );

void   sakura_sed_get_phe        ( Sed_cube p , double y , Sakura_phe_st* phe_data );
double sakura_sed_add_sediment   ( Sed_cube p , double y , Sakura_cell_st* s );
double sakura_sed_remove_sediment( Sed_cube p , double y , Sakura_cell_st* s );
double sakura_sed_get_depth      ( Sed_cube p , double y );
/*
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

   Sakura_query_func get_phe;
   gpointer          get_phe_data;

   gpointer get_depth_data;
   gpointer remove_data;
   gpointer add_data;
   Sed_query_func get_depth;
   Sed_query_func remove;
   Sed_query_func add;
}
Sakura_t;
*/

#endif /* sakura_local.h is included */
