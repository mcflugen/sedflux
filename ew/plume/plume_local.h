#ifndef __PLUME_LOCALH__
#define __PLUME_LOCALH__

#include <glib.h>
#include <sed/sed_sedflux.h>

#define PLUME_MAJOR_VERSION 0
#define PLUME_MINOR_VERSION 9
#define PLUME_MICRO_VERSION 0

typedef struct
{
   double r_dir;                 /// Angle of the coast normal
   double r_angle;               /// Angle river makes with coast (0 deg is normal to coastline)
   double latitude;              /// Latitude of river mouth
   double ocean_conc;            /// Ocean sediment concentration
   double coastal_current;       /// Velocity of coastal current
   double coastal_current_dir;   /// Direction of coastal current
   double coastal_current_width; /// Normalized width of coastal current
   double river_tracer;          /// Concentration of river water
   double ocean_tracer;          /// Concentration of ocean water

   gint    river_mouth_nodes;    /// Number of grid nodes in river mouth
   double  aspect_ratio;         /// Ratio of cross-shore to along-shore grid spacing

//   double  fjord_wall_left;      /// Position of left fjord wall
//   double  fjord_wall_right;     /// Position of right fjord wall
   double  basin_width;          /// Width of the basin
   double  basin_len;            /// Length of the basin
   double  dx;                   /// Cross-shore output resolution
   double  dy;                   /// Along-shore output resolution

   gint     n_dim;               /// Number of dimensions [1|2]
   gboolean rotate;              /// Rotate the output grid

   double* bulk_density;         /// Saturated density of river sediments
   double* lambda;               /// Removal rate of rive sediments
   gint    n_grains;             /// Number of suspended sediment grain types
}
Plume_param_st;

Plume_param_st* plume_scan_parameter_file( const gchar*    file , GError**        error );
Plume_param_st* plume_check_params       ( Plume_param_st* p    , GError**        error );
gint            plume_print_data         ( const gchar* file , Eh_dbl_grid* deposit , gint len , gint n_grains );
Eh_dbl_grid*    plume_wrapper            ( Sed_hydro       r    , Plume_param_st* p , gint* len , gint* n_grains );

Eh_dbl_grid*    plume_dbl_grid_new       ( gint n_grains , gint n_x , gint n_y , double dx , double dy );

#include "plume_types.h"

Eh_dbl_grid* plume_output_3         ( Plume_enviro *env , Plume_grid *grid , Eh_dbl_grid *dest , gboolean rotate );
double*      plume_river_volume     ( Plume_river r , Plume_sediment* s , const gint n_grains );
Eh_dbl_grid* plume_dbl_grid_set_land( Eh_dbl_grid* grid , double val );
Eh_dbl_grid* plume_dbl_grid_rebin   ( Eh_dbl_grid* grid , Eh_dbl_grid* dest );
Eh_dbl_grid* plume_dbl_grid_rotate  ( Eh_dbl_grid* grid , gint i , gint j , double alpha );
Eh_dbl_grid* plume_dbl_grid_scale   ( Eh_dbl_grid* grid , double* vol_in );
Eh_dbl_grid* plume_dbl_grid_trim    ( Eh_dbl_grid* grid , double val );

#endif
