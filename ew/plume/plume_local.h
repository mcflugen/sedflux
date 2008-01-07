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

   double* bulk_density;         /// Saturated density of river sediments
   double* lambda;               /// Removal rate of rive sediments
   gint    n_grains;             /// Number of suspended sediment grain types
}
Plume_param_st;

Plume_param_st* plume_scan_parameter_file( const gchar*    file , GError**        error );
Plume_param_st* plume_check_params       ( Plume_param_st* p    , GError**        error );
gint            plume_print_data         ( const gchar* file , double** deposit , gint len , gint n_grains );
double**        plume_wrapper            ( Sed_hydro       r    , Plume_param_st* p , gint* len , gint* n_grains );

#endif
