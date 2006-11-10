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

#if !defined(SED_SEDIMENT_H)
#define SED_SEDIMENT_H

#include <stdio.h>
#include <glib.h>

#include "datadir.h"

#if !defined( DATADIR )
# define DATADIR "/usr/local/share"
#endif

#define SED_SEDIMENT_TEST_FILE DATADIR "/libsed_test_files/test.sediment"

new_handle( Sed_type     );
new_handle( Sed_sediment );

// the wentworth size class 
typedef gint32 Sed_size_class;

typedef double (*Sed_type_property_func_0) ( const Sed_type );
typedef double (*Sed_type_property_func_1) ( const Sed_type , double );
typedef double (*Sed_type_property_func_2) ( const Sed_type , double , double );
typedef double (*Sed_type_property_func_with_data) ( const Sed_type , gpointer user_data );

#include "sed_cell.h"
#include "sed_const.h"
#include "sed_property.h"

typedef enum
{
   S_PROPERTY_ID_BAD_VAL = 0,
   S_PROPERTY_ID_BULK_DENSITY,
   S_PROPERTY_ID_GRAIN_SIZE,
   S_PROPERTY_ID_PLASTIC_INDEX,
   S_PROPERTY_ID_FACIES,
   S_PROPERTY_ID_AGE,
   S_PROPERTY_ID_VELOCITY,
   S_PROPERTY_ID_VOID_RATIO,
   S_PROPERTY_ID_VOID_RATIO_MIN,
   S_PROPERTY_ID_VISCOSITY,
   S_PROPERTY_ID_FRICTION_ANGLE,
   S_PROPERTY_ID_PERMEABILITY,
   S_PROPERTY_ID_POROSITY,
   S_PROPERTY_ID_RELATIVE_DENSITY,
   S_PROPERTY_ID_MV,
   S_PROPERTY_ID_CV,
   S_PROPERTY_ID_SHEAR_STRENGTH,
   S_PROPERTY_ID_COHESION,
   S_PROPERTY_ID_EXCESS_PRESSURE,
   S_PROPERTY_ID_CONSOLIDATION,
   S_PROPERTY_ID_CONSOLIDATION_RATE,
   S_PROPERTY_ID_RELATIVE_PRESSURE,
   S_PROPERTY_ID_SAND_FRACTION,
   S_PROPERTY_ID_SILT_FRACTION,
   S_PROPERTY_ID_CLAY_FRACTION,
   S_PROPERTY_ID_MUD_FRACTION,
   S_PROPERTY_ID_GRAIN_DENSITY,
   S_PROPERTY_ID_MAX_DENSITY,
   S_PROPERTY_ID_GRAIN_SIZE_IN_M,
   S_PROPERTY_ID_POROSITY_MIN,
   S_PROPERTY_ID_POROSITY_MAX,
   S_PROPERTY_ID_HYDRAULIC_CONDUCTIVITY,
   S_PROPERTY_ID_VOID_RATIO_MAX,
   S_PROPERTY_ID_CC,
   S_PROPERTY_ID_YIELD_STRENGTH,
   S_PROPERTY_ID_DYNAMIC_VISCOSITY,
   S_PROPERTY_ID_COMPRESSIBILITY,
   S_PROPERTY_ID_PRESSURE,
   S_PROPERTY_ID_FRACTION = 100
}
Sed_property_id;

#define S_SED_TYPE_NONE             ((Sed_size_class)(0))     // no size class
#define S_SED_TYPE_BOULDER          ((Sed_size_class)(1<<0 )) // -8 -> -12 f
#define S_SED_TYPE_COBBLE           ((Sed_size_class)(1<<1 )) // -5 -> -8 f
#define S_SED_TYPE_PEBBLE           ((Sed_size_class)(1<<2 )) // -2 -> -5 f
#define S_SED_TYPE_GRANULE          ((Sed_size_class)(1<<3 )) // -1 -> -2 f
#define S_SED_TYPE_VERY_COARSE_SAND ((Sed_size_class)(1<<4 )) //  0 -> -1 f
#define S_SED_TYPE_COARSE_SAND      ((Sed_size_class)(1<<5 )) //  1 ->  0 f
#define S_SED_TYPE_MEDIUM_SAND      ((Sed_size_class)(1<<6 )) //  2 ->  1 f
#define S_SED_TYPE_FINE_SAND        ((Sed_size_class)(1<<7 )) //  3 ->  2 f
#define S_SED_TYPE_VERY_FINE_SAND   ((Sed_size_class)(1<<8 )) //  4 ->  3 f
#define S_SED_TYPE_COARSE_SILT      ((Sed_size_class)(1<<9 )) //  5 ->  4 f
#define S_SED_TYPE_MEDIUM_SILT      ((Sed_size_class)(1<<10)) //  6 ->  5 f
#define S_SED_TYPE_FINE_SILT        ((Sed_size_class)(1<<11)) //  7 ->  6 f
#define S_SED_TYPE_VERY_FINE_SILT   ((Sed_size_class)(1<<12)) //  8 ->  7 f
#define S_SED_TYPE_COARSE_CLAY      ((Sed_size_class)(1<<13)) //  9 ->  8 f
#define S_SED_TYPE_MEDIUM_CLAY      ((Sed_size_class)(1<<14)) //  10 -> 9 f
#define S_SED_TYPE_FINE_CLAY        ((Sed_size_class)(1<<15)) //  11 -> 10 f

#define S_SED_TYPE_GRAVEL           ((Sed_size_class)(      \
                                         S_SED_TYPE_PEBBLE  \
                                       | S_SED_TYPE_GRANULE ))
#define S_SED_TYPE_SAND             ((Sed_size_class)(               \
                                         S_SED_TYPE_VERY_COARSE_SAND \
                                       | S_SED_TYPE_COARSE_SAND      \
                                       | S_SED_TYPE_MEDIUM_SAND      \
                                       | S_SED_TYPE_FINE_SAND        \
                                       | S_SED_TYPE_VERY_FINE_SAND   ))
#define S_SED_TYPE_SILT             ((Sed_size_class)(             \
                                         S_SED_TYPE_COARSE_SILT    \
                                       | S_SED_TYPE_MEDIUM_SILT    \
                                       | S_SED_TYPE_FINE_SILT      \
                                       | S_SED_TYPE_VERY_FINE_SILT ))
#define S_SED_TYPE_CLAY             ((Sed_size_class)(          \
                                         S_SED_TYPE_COARSE_CLAY \
                                       | S_SED_TYPE_MEDIUM_CLAY \
                                       | S_SED_TYPE_FINE_CLAY   ))
#define S_SED_TYPE_MUD              ((Sed_size_class)(   \
                                         S_SED_TYPE_SILT \
                                       | S_SED_TYPE_CLAY ))

// these are the maximum phis for each size class.
#define S_SED_TYPE_BOULDER_PHI          (-8.)
#define S_SED_TYPE_COBBLE_PHI           (-5.)
#define S_SED_TYPE_PEBBLE_PHI           (-2.)
#define S_SED_TYPE_GRANULE_PHI          (-1.)
#define S_SED_TYPE_VERY_COARSE_SAND_PHI ( 0.)
#define S_SED_TYPE_COARSE_SAND_PHI      ( 1.)
#define S_SED_TYPE_SAND_PHI             ( 2.)
#define S_SED_TYPE_FINE_SAND_PHI        ( 3.)
#define S_SED_TYPE_VERY_FINE_SAND_PHI   ( 4.)
#define S_SED_TYPE_COARSE_SILT_PHI      ( 5.)
#define S_SED_TYPE_SILT_PHI             ( 6.)
#define S_SED_TYPE_FINE_SILT_PHI        ( 7.)
#define S_SED_TYPE_VERY_FINE_SILT_PHI   ( 8.)
#define S_SED_TYPE_COARSE_CLAY_PHI      ( 9.)
#define S_SED_TYPE_MEDIUM_CLAY_PHI      (10.)
#define S_SED_TYPE_FINE_CLAY_PHI        (11.)

Sed_sediment sed_sediment_new      ( void                                    );
Sed_sediment sed_sediment_new_sized( gssize n                                );
Sed_sediment sed_sediment_copy     ( Sed_sediment       , const Sed_sediment );
Sed_sediment sed_sediment_dup      ( const Sed_sediment                      );
gssize       sed_sediment_n_types  ( const Sed_sediment                      );
Sed_sediment sed_sediment_add_type ( Sed_sediment       , const Sed_type     );
Sed_sediment sed_sediment_insert_sorted( Sed_sediment s , Sed_type t ) G_GNUC_INTERNAL;
gboolean     sed_sediment_has_type ( Sed_sediment s , Sed_type t );
Sed_sediment sed_sediment_append   ( Sed_sediment s     , Sed_type t ) G_GNUC_INTERNAL;
void         sed_sediment_foreach  ( Sed_sediment s     , GFunc f , gpointer user_data );
double*      sed_sediment_property ( Sed_sediment s     , Sed_type_property_func_0 f );
double       sed_sediment_property_avg  ( Sed_sediment s , double* f , Sed_type_property_func_0 p_func );
double       sed_sediment_property_avg_1( Sed_sediment s , double* f , double arg_1 , Sed_type_property_func_1 p_func );
double       sed_sediment_property_avg_2( Sed_sediment s , double* f , double arg_1 , double arg_2 , Sed_type_property_func_2 p_func );
double       sed_sediment_property_avg_with_data( Sed_sediment s , double* f , Sed_type_property_func_with_data p_func , gpointer user_data );
gssize       sed_sediment_fprint   ( FILE* fp , Sed_sediment s );
void         sed_sediment_fprint_default( FILE *fp );
Sed_type     sed_sediment_type     ( const Sed_sediment , gssize             );
Sed_sediment sed_sediment_destroy  ( Sed_sediment                            );
gssize       sed_sediment_write    ( FILE*              , const Sed_sediment );
Sed_sediment sed_sediment_load     ( FILE*                                   );
Sed_sediment sed_sediment_scan     ( const char*                             );

Sed_sediment sed_sediment_set_env    ( Sed_sediment s );
Sed_sediment sed_sediment_unset_env  ( );
Sed_sediment sed_sediment_env        ( ) G_GNUC_INTERNAL;
gboolean     sed_sediment_env_is_set ( );
gssize       sed_sediment_env_size   ( );
Sed_sediment sed_sediment_resize     ( Sed_sediment s , gssize new_len ) G_GNUC_INTERNAL;

Sed_type     sed_type_set_rho_sat           ( Sed_type t , double rho_sat );
Sed_type     sed_type_set_rho_grain         ( Sed_type t , double rho_grain );
Sed_type     sed_type_set_grain_size        ( Sed_type t , double gz );
Sed_type     sed_type_set_plastic_index     ( Sed_type t , double pi ) G_GNUC_DEPRECATED;
Sed_type     sed_type_set_void_ratio_min    ( Sed_type t , double void_min );
Sed_type     sed_type_set_diff_coef         ( Sed_type t , double k );
Sed_type     sed_type_set_lambda            ( Sed_type t , double l );
Sed_type     sed_type_set_settling_velocity ( Sed_type t , double w );
Sed_type     sed_type_set_c_consolidation   ( Sed_type t , double c_v );
Sed_type     sed_type_set_compressibility   ( Sed_type t , double c );

double         sed_type_plastic_index( const Sed_type t ) G_GNUC_DEPRECATED;

double         sed_type_rho_sat( const Sed_type t );
double         sed_type_rho_grain( const Sed_type t );
double         sed_type_grain_size( const Sed_type t );
double         sed_type_void_ratio_min(const Sed_type t );
double         sed_type_diff_coef(const Sed_type t );
double         sed_type_lambda( const Sed_type t );
double         sed_type_lambda_in_per_seconds ( const Sed_type t );
double         sed_type_settling_velocity( const Sed_type t );
double         sed_removal_rate_to_settling_velocity( double l );
double         sed_type_c_consolidation( const Sed_type t );
double         sed_type_compressibility( const Sed_type t );
double         sed_type_density_0( const Sed_type t );
double         sed_type_sat_density( const Sed_type t );
double         sed_type_rho_max( const Sed_type t );
double         sed_type_grain_size_in_meters( const Sed_type t );
double         sed_type_inv_grain_size_in_meters( const Sed_type t );
double         sed_type_grain_size_in_phi( const Sed_type t );
double         sed_type_is_sand( const Sed_type t );
double         sed_type_is_silt( const Sed_type t );
double         sed_type_is_clay( const Sed_type t );
double         sed_type_is_mud( const Sed_type t );
Sed_size_class sed_type_grain_size_in_wentworth ( const Sed_type t );
Sed_size_class sed_type_size_class( const Sed_type t );
double         sed_type_velocity( const Sed_type t );
double         sed_type_viscosity( const Sed_type t );
double         sed_type_dynamic_viscosity( const Sed_type t );
double         sed_type_relative_density( const Sed_type t );
double         sed_type_void_ratio( Sed_type t );
double         sed_type_void_ratio_max( Sed_type t );
double         sed_type_porosity( const Sed_type t );
double         sed_type_porosity_min( const Sed_type t );
double         sed_type_porosity_max(const Sed_type t );
double         sed_type_permeability( const Sed_type t );
double         sed_type_hydraulic_conductivity( const Sed_type t );
double         sed_type_water_content( const Sed_type t );
double         sed_specific_gravity( const Sed_type t );
double         sed_type_friction_angle( const Sed_type t );
double         sed_type_yield_strength( const Sed_type t );
double         sed_type_mv( const Sed_type t );
double         sed_type_cv( const Sed_type t );
double         sed_type_shear_strength( const Sed_type t , double load );
double         sed_type_cohesion( const Sed_type t , double load );
double         sed_type_consolidation( const Sed_type t , double d , double dt );
double         sed_type_consolidation_rate( const Sed_type t , double d , double dt );
double         sed_calculate_avg_consolidation( double c_v , double d , double t );
double         sed_calculate_consolidation( double c_v , double d , double z , double t );
double         sed_type_is_size_class( Sed_type t , Sed_size_class size );
Sed_size_class sed_size_class( const double phi );

double sed_type_shear_strength_with_data( const Sed_type t , gpointer data ) G_GNUC_INTERNAL;
double sed_type_cohesion_with_data( const Sed_type t , gpointer data ) G_GNUC_INTERNAL;
double sed_type_consolidation_with_data( const Sed_type t , gpointer data ) G_GNUC_INTERNAL;
double sed_type_consolidation_rate_with_data( const Sed_type t , gpointer data ) G_GNUC_INTERNAL;
double sed_type_is_size_class_with_data( Sed_type t , gpointer size ) G_GNUC_INTERNAL;
double sed_type_sum_size_classes_with_data( Sed_type t , gpointer size ) G_GNUC_INTERNAL;
double sed_type_void_ratio_compacted( Sed_type t , gpointer d ) G_GNUC_INTERNAL;
double sed_type_density_compacted( Sed_type t , gpointer d ) G_GNUC_INTERNAL;

double sed_removal_rate_to_settling_velocity( double );
double sed_settling_velocity_to_removal_rate( double w_s );
Sed_size_class sed_grain_size_in_wentworth( const Sed_sediment , gssize );

Sed_type sed_type_new    ( );
Sed_type sed_type_destroy( Sed_type s        );
Sed_type sed_type_copy   ( Sed_type dest , Sed_type src );
Sed_type sed_type_dup    ( Sed_type src );
gssize   sed_type_fprint ( FILE* fp , Sed_type t );
gssize   sed_type_write  ( FILE* fp , Sed_type t );
Sed_type sed_type_read   ( FILE* fp );
Sed_type sed_type_init   ( Eh_symbol_table t );
gboolean sed_type_is_same( Sed_type t_1 , Sed_type t_2 );

double sed_gravity                 ( void            );
double sed_gravity_units           ( Sed_units units );
double sed_set_gravity             ( double new_val  );
double sed_rho_sea_water           ( void            );
double sed_rho_sea_water_units     ( Sed_units units );
double sed_set_rho_sea_water       ( double new_val  );
double sed_rho_fresh_water         ( void            );
double sed_rho_fresh_water_units   ( Sed_units units );
double sed_set_rho_fresh_water     ( double new_val  );
double sed_sea_salinity            ( void            );
double sed_sea_salinity_units      ( Sed_units units );
double sed_set_sea_salinity        ( double new_val  );
double sed_rho_quartz              ( void            );
double sed_rho_quartz_units        ( Sed_units units );
double sed_set_rho_quartz          ( double new_val  );
double sed_rho_mantle              ( void            );
double sed_rho_mantle_units        ( Sed_units units );
double sed_set_rho_mantle          ( double new_val  );

#endif /* sed_sediment.h is included */

