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

#if !defined(SED_CELL_H)
# define SED_CELL_H

#include <glib.h>
#include "utils.h"

new_handle( Sed_cell );

typedef double (*Sed_cell_property_func_0) ( const Sed_cell );
typedef double (*Sed_cell_property_func_1) ( const Sed_cell , double );
typedef double (*Sed_cell_property_func_2) ( const Sed_cell , double , double );
typedef double (*Sed_cell_property_func_data) ( const Sed_cell , gpointer user_data );

typedef union
{
   Sed_cell_property_func_0 f_0;
   Sed_cell_property_func_1 f_1;
   Sed_cell_property_func_2 f_2;
}
Sed_cell_property_func;

#include "sed_property.h"
#include "sed_sediment.h"

/** Describe the facies of a unit of sediment.

Sed_facies is used to describe the facies for a cell of sediment.  The
facies is obtained by examining the bits of the facies value.  There are
a number of masks used to do this (these are the S_FACIES* macros defined
in sed_const.h).

\see Sed_cell.
*/
typedef unsigned char Sed_facies;

/** A two-dimensional grid of pointers to Sed_cell's.

\see Sed_cell
*/
derived_handle( Eh_grid , Sed_cell_grid );

#define SED_CELL_CONST_S_F 1.25

Sed_cell     sed_cell_new                ( gssize n );
Sed_cell     sed_cell_new_env            ( void );
Sed_cell     sed_cell_new_sized          ( gssize n , double t , double* f );
Sed_cell     sed_cell_new_typed          ( Sed_sediment s , double t , Sed_type a_type );
Sed_cell     sed_cell_new_classed        ( Sed_sediment s , double t , Sed_size_class class );

Sed_cell     sed_cell_dup                ( Sed_cell src );
Sed_cell     sed_cell_copy               ( Sed_cell dest , Sed_cell src );
Sed_cell     sed_cell_destroy            ( Sed_cell c );

Sed_cell     sed_cell_clear              ( Sed_cell );
Sed_cell     sed_cell_set_age            ( Sed_cell c , double new_age );
Sed_cell     sed_cell_set_thickness      ( Sed_cell c , double new_t ) G_GNUC_DEPRECATED;
Sed_cell     sed_cell_resize             ( Sed_cell c , double );
Sed_cell     sed_cell_compact            ( Sed_cell c , double );
Sed_cell     sed_cell_set_pressure       ( Sed_cell c , double new_pressure );
Sed_cell     sed_cell_set_facies         ( Sed_cell c , Sed_facies new_facies );
Sed_cell     sed_cell_add_facies         ( Sed_cell c , Sed_facies f );
Sed_cell     sed_cell_set_amount         ( Sed_cell c , const double t[] );
Sed_cell     sed_cell_add_amount         ( Sed_cell a , const double t[] );
Sed_cell     sed_cell_set_fraction       ( Sed_cell c , double new_f[] );
Sed_cell     sed_cell_set_equal_fraction ( Sed_cell c );
Sed_cell     sed_cell_add                ( Sed_cell c_1 , const Sed_cell c_2 );

gboolean     sed_cell_is_empty           ( Sed_cell a );
gboolean     sed_cell_is_clear           ( Sed_cell c );
gboolean     sed_cell_is_size            ( Sed_cell c , double t   );
gboolean     sed_cell_is_age             ( Sed_cell c , double a   );
gboolean     sed_cell_is_mass            ( Sed_cell c , double m   );
gboolean     sed_cell_is_size_class      ( Sed_cell c , Sed_size_class size );
gboolean     sed_cell_is_compatible      ( Sed_cell a , Sed_cell b );

Sed_cell *sed_cell_clear_vector   (Sed_cell *vec,int low,int high ) G_GNUC_DEPRECATED;

Sed_cell     sed_cell_separate_amount    ( Sed_cell , double[] , Sed_cell );
Sed_cell     sed_cell_separate_thickness ( Sed_cell , double   , Sed_cell );
Sed_cell     sed_cell_separate_fraction  ( Sed_cell , double[] , Sed_cell );
Sed_cell     sed_cell_separate_cell      ( Sed_cell , Sed_cell );
Sed_cell     sed_cell_separate           ( Sed_cell , double[] , double , Sed_cell );
void         sed_cell_move_thickness     ( Sed_cell , Sed_cell , double   );
void         sed_cell_move_fraction      ( Sed_cell , Sed_cell , double[] );
void         sed_cell_move               ( Sed_cell , Sed_cell , double[] , double );

double       sed_cell_thickness         ( const Sed_cell c ) G_GNUC_DEPRECATED;
gssize       sed_cell_n_types           ( const Sed_cell c );
double       sed_cell_size              ( const Sed_cell c );
double       sed_cell_size_0            ( const Sed_cell c );
double       sed_cell_age               ( const Sed_cell c );
double       sed_cell_age_in_years      ( const Sed_cell c );
Sed_facies   sed_cell_facies            ( const Sed_cell c );
double       sed_cell_pressure          ( const Sed_cell c );
double       sed_cell_fraction          ( const Sed_cell c , gssize n         );
double       sed_cell_excess_pressure   ( const Sed_cell c , double load      );
double       sed_cell_mass              ( const Sed_cell c );
double       sed_cell_load              ( const Sed_cell c );
double       sed_cell_sediment_load     ( const Sed_cell c );

gssize       sed_cell_fprint            ( FILE* , const Sed_cell );
gssize       sed_cell_write             ( FILE* , const Sed_cell );
gssize       sed_cell_write_to_byte_order( FILE *fp, const Sed_cell c , int order );
Sed_cell     sed_cell_read              ( FILE* );

Sed_cell *sed_cell_list_new( gssize len , gssize n );
Sed_cell* sed_cell_list_destroy(Sed_cell *c_list);

double sed_cell_density_0              ( const Sed_cell );
double sed_cell_density                ( const Sed_cell );
double sed_cell_sediment_volume        ( const Sed_cell );
double sed_cell_sediment_mass          ( const Sed_cell );
double sed_cell_grain_density          ( const Sed_cell );
double sed_cell_max_density            ( const Sed_cell );
double sed_cell_grain_size             ( const Sed_cell );
double sed_cell_grain_size_in_phi      ( const Sed_cell );
double sed_cell_sand_fraction          ( const Sed_cell );
double sed_cell_silt_fraction          ( const Sed_cell );
double sed_cell_clay_fraction          ( const Sed_cell );
double sed_cell_mud_fraction           ( const Sed_cell );
double sed_cell_nth_fraction           ( const Sed_cell , gssize n );
double sed_cell_c_consolidation        ( const Sed_cell );
double sed_cell_velocity               ( const Sed_cell );
double sed_cell_viscosity              ( const Sed_cell );
double sed_cell_relative_density       ( const Sed_cell );
double sed_cell_porosity               ( const Sed_cell );
double sed_cell_porosity_max           ( const Sed_cell );
double sed_cell_porosity_min           ( const Sed_cell );
double sed_cell_plastic_index          ( const Sed_cell );
double sed_cell_permeability           ( const Sed_cell );
double sed_cell_hydraulic_conductivity ( const Sed_cell );
double sed_cell_bulk_permeability      ( const Sed_cell );
double sed_cell_bulk_hydraulic_conductivity( const Sed_cell );

double* sed_cell_fraction_ptr( const Sed_cell c ) G_GNUC_DEPRECATED;
double* sed_cell_copy_fraction( double* f , const Sed_cell c );


double sed_cell_void_ratio             ( const Sed_cell );
double sed_cell_void_ratio_min         ( const Sed_cell );
double sed_cell_void_ratio_max         ( const Sed_cell );
double sed_cell_friction_angle         ( const Sed_cell );
double sed_cell_cc                     ( const Sed_cell );
double sed_cell_compressibility        ( const Sed_cell );
double sed_cell_yield_strength         ( const Sed_cell );
double sed_cell_bulk_yield_strength    ( const Sed_cell );
double sed_cell_dynamic_viscosity      ( const Sed_cell );
double sed_cell_bulk_dynamic_viscosity ( const Sed_cell );
double sed_cell_water_ratio            ( const Sed_cell );
double sed_cell_air_ratio              ( const Sed_cell );
double sed_cell_mv                     ( const Sed_cell );
double sed_cell_cv                     ( const Sed_cell );
double sed_cell_bulk_cv                ( const Sed_cell );

double sed_cell_shear_strength         ( const Sed_cell , double load );
double sed_cell_cohesion               ( const Sed_cell , double load );
double sed_cell_consolidation          ( const Sed_cell , double time );
double sed_cell_consolidation_rate     ( const Sed_cell , double time );
double sed_cell_relative_pressure      ( const Sed_cell , double load );
Sed_size_class sed_cell_size_class     ( const Sed_cell );
double         sed_cell_size_class_percent( Sed_cell c , Sed_size_class size );
Sed_size_class sed_cell_size_classes( Sed_cell c );

Sed_cell_grid    sed_cell_grid_new       ( gsize n_x          , gsize n_y                           );
Sed_cell_grid    sed_cell_grid_init      ( Sed_cell_grid g    , gssize n_grains                     );
void             sed_cell_grid_free      ( Sed_cell_grid g                                          );
void             sed_cell_grid_free_data ( Sed_cell_grid g                                          );
Sed_cell         sed_cell_grid_val       ( Sed_cell_grid g    , gssize i          , gssize j        );
Sed_cell_grid    sed_cell_grid_add       ( Sed_cell_grid g_1  , Sed_cell_grid g_2                   );
Sed_cell_grid    sed_cell_grid_copy_data ( Sed_cell_grid dest , Sed_cell_grid src                   );
Sed_cell_grid    sed_cell_grid_clear     ( Sed_cell_grid g                                          );
double           sed_cell_grid_mass      ( Sed_cell_grid g );
Sed_cell**       sed_cell_grid_data      ( Sed_cell_grid g                                          );

gboolean sed_cell_is_same  ( Sed_cell a , Sed_cell b );
gboolean sed_cell_is_valid ( Sed_cell a );

#endif /* sed_cell.h is included */

