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

#ifndef INFLOW_LOCAL_IS_INCLUDED
# define INFLOW_LOCAL_IS_INCLUDED

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
#include <sed/sed_sedflux.h>
#include "inflow.h"

G_BEGIN_DECLS

typedef struct
{
   double* x;
   double* depth;
   double* width;
   double* slope;
   gint len;
}
Inflow_bathy_st;

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
Inflow_flood_st;

typedef struct
{
   double* size_equiv;
   double* lambda;
   double* bulk_density;
   double* grain_density;
   gint    n_grains;
}
Inflow_sediment_st;

#define INFLOW_DEFAULT_BASIN_LEN                (50.)
#define INFLOW_DEFAULT_DX                       (10.)
#define INFLOW_DEFAULT_RHO_SEA_WATER            (1028.)

#define INFLOW_DEFAULT_LAMBDA                   { 25. , 16.8 , 9. , 3.2 , 2.4 }
#define INFLOW_DEFAULT_SIZE_EQUIV               { 202. , 105. , 69. , 25. , 10. }
#define INFLOW_DEFAULT_SIZE_COMP                { 150. , 50. , 25. , 5. , 1. }
#define INFLOW_DEFAULT_GRAIN_FRACTION           { 0. , .1 , .2 , .3 , .4 }
#define INFLOW_DEFAULT_FLOW_FRACTION            { 1. , 1. , 1. , 1. , 1. }
#define INFLOW_DEFAULT_BULK_DENSITY             { 1850. , 1600. , 1400. , 1300. , 1200. }
#define INFLOW_DEFAULT_GRAIN_DENSITY            { 2650. , 2650. , 2650. , 2650. , 2650. }
#define INFLOW_DEFAULT_DEP_START                (2.)
#define INFLOW_DEFAULT_SIZE_BOTTOM              (64.)
#define INFLOW_DEFAULT_BULK_DENSITY_BOTTOM      (1600.)
#define INFLOW_DEFAULT_BOTTOM_FRACTION          { .2 , .2 , .2 , .2 , .2 }
#define INFLOW_DEFAULT_N_GRAINS                 (5)

#define INFLOW_DEFAULT_SUA                      (30.)
#define INFLOW_DEFAULT_SUB                      (.2)
#define INFLOW_DEFAULT_EA                       (0.00153)
#define INFLOW_DEFAULT_EB                       (0.00204)
#define INFLOW_DEFAULT_CD                       (0.004)
#define INFLOW_DEFAULT_FRICTION_ANGLE           (20.)
#define INFLOW_DEFAULT_MU_WATER                 (1.3)

#define INFLOW_DEFAULT_FLOOD_FILE               "flood.kvf"

typedef struct
{
   double  basin_len;
   double  dx;
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
Inflow_param_st;

typedef struct
{
   double* phe_bottom;
   int     n_grains;
}
Inflow_bottom_st;

gboolean inflow_wrapper( Inflow_bathy_st* b    ,
                         Inflow_flood_st* f    ,
                         Inflow_sediment_st* s ,
                         Inflow_const_st* c    ,
                         double** deposition   ,
                         double** erosion );

Inflow_param_st*    inflow_scan_parameter_file( const gchar* file , GError** error );
Inflow_param_st*    inflow_check_params       ( Inflow_param_st* p , GError** error );
Inflow_bathy_st*    inflow_scan_bathy_file    ( const gchar* file , Inflow_param_st* p , GError** error );
Inflow_flood_st**   inflow_scan_flood_file    ( const gchar* file , Inflow_param_st* p , GError** error );
Inflow_flood_st*    inflow_set_flood_data     ( Sed_hydro h , double rho_river_water );
Inflow_flood_st*    inflow_destroy_flood_data ( Inflow_flood_st* f );
Inflow_sediment_st* inflow_set_sediment_data  ( Inflow_param_st* p );
Inflow_const_st*    inflow_set_constant_data  ( Inflow_param_st* p );
Inflow_bathy_st*    inflow_set_bathy_data     ( double** bathy , gint len , double dx , double basin_len );
Inflow_bathy_st*    inflow_destroy_bathy_data ( Inflow_bathy_st* b );
Inflow_bathy_st*    inflow_update_bathy_data  ( Inflow_bathy_st* b , double** deposition , double** erosion , gint n_grains );
gint                inflow_write_output       ( const gchar* file  ,
                                                Inflow_bathy_st* b ,
                                                double** deposit   ,
                                                gssize n_grains );

void inflow_get_phe( Inflow_phe_query_st* query_data , Inflow_bottom_st* bed_data );

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

   Inflow_query_func get_phe;
   gpointer          get_phe_data;

   gpointer get_depth_data;
   gpointer remove_data;
   gpointer add_data;
   Sed_query_func get_depth;
   Sed_query_func remove;
   Sed_query_func add;
}
Inflow_t;
*/

G_END_DECLS

#endif /* inflow.h is included */
