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

/** Describe a unit of sediment.

The Sed_cell struct is used to hold a number of differnt sediment types.
The cell of sediment has a thickness that is able to change but it's original
thickness is always remembered.

The fraction pointer points to an array that holds the fraction of each 
sediment type contained within the cel.  Note that the number of sediment
types that are stored in the cell is not part of the Sed_cell structure.  This
was done to save space since most applications make use of this structure will
have a single number of sediment types that does not change.

The \a uncompacted_thickness member saves the initial thickness of the cell,
before any compaction of the sediment occurs.

The \a thickness member is the current thickness of the cell.

The \a age member is the average age of the sediment stored in the cell.  That is,
the average length of time that the sediment has sat in this cell.

The \a pressure member is the amount of porewater pressure built up in the cell.
As more load is applied to the cell, the pressure builds up.

The \a facies member indicates the various facies that are represented by the
cell.  This member is no longer used and is scheduled for deletion.

\see Sediment, sed_create_cell, sed_destroy_cell.

*/

/** A list of sediment cells.

This list should be NULL-terminated.

\see Sed_cell
*/
//typedef Sed_cell* Cell_list;

/** A two-dimensional grid of pointers to Sed_cell's.

\see Sed_cell
*/
derived_handle( Eh_grid , Sed_cell_grid );

/** \name Sediment Cell Property Functions

Predefined functions that calculate a geotechnical property of
a Sed_cell

@{

*/

/*
///Function to get bulk density.
#define S_DENSITY                  (&sed_cell_density)
///Function to get cell grain density.
#define S_GRAIN_DENSITY            (&sed_cell_grain_density)
///Function to get cell 'closest packed' density.
#define S_MAX_DENSITY              (&sed_cell_max_density)
///Function to get cell 'closest packed' density.
#define S_GRAIN_SIZE               (&sed_cell_grain_size)
///Function to get cell grain size in phi units.
#define S_GRAIN_SIZE_IN_PHI        (&sed_cell_grain_size_in_phi)
///Function to get the fraction of sand in a cell.
#define S_SAND_FRACTION            (&sed_cell_sand_fraction)
///Function to get the fraction of silt in a cell.
#define S_SILT_FRACTION            (&sed_cell_silt_fraction)
///Function to get the fraction of clay in a cell.
#define S_CLAY_FRACTION            (&sed_cell_clay_fraction)
///Function to get the fraction of mud (silt and clay) in a cell.
#define S_MUD_FRACTION             (&sed_cell_mud_fraction)
///Function to get the average velocity of sound in the cell.
#define S_VELOCITY                 (&sed_cell_velocity)
///Function to get the average viscosity of the cell.
#define S_VISCOSITY                (&sed_cell_viscosity)
///Function to get the cell density relative to its uncompacted state.
#define S_RELATIVE_DENSITY         (&sed_cell_relative_density)
///Function to get the porosity of the cell.
#define S_POROSITY                 (&sed_cell_porosity)
///Function to get the porosity of the cell in its closest packed state.
#define S_POROSITY_MIN             (&sed_cell_porosity_min)
///Function to get the porosity of the cell in its (loosest) uncompacted state.
#define S_POROSITY_MAX             (&sed_cell_porosity_max)
///Function to get the plastic index of the cell (obsolete).
#define S_PLASTIC_INDEX            (&sed_cell_plastic_index)
///Function to get the permeability of the cell.
#define S_PERMEABILITY             (&sed_cell_permeability)
///Function to get the hydraulic conductivity of the cell.
#define S_HYDRAULIC_CONDUCTIVITY   (&sed_cell_hydraulic_conductivity)
///Function to get the void ratio of a cell.
#define S_VOID_RATIO               (&sed_cell_void_ratio)
///Function to get the minimum possible void ratio of a cell.
#define S_VOID_RATIO_MIN           (&sed_cell_void_ratio_min)
///Function to get the maximum possible void ratio of a cell.
#define S_VOID_RATIO_MAX           (&sed_cell_void_ratio_max)
///Function to get the friction angle of a cell.
#define S_FRICTION_ANGLE           (&sed_cell_friction_angle)
///Function to get the consolidation coefficient of a cell.
#define S_CC                       (&sed_cell_cc)
///Function to get the yield strength of a cell.
#define S_YIELD_STRENGTH           (&sed_cell_yield_strength)
///Function to get the dynamic viscosity of a cell.
#define S_DYNAMIC_VISCOSITY        (&sed_cell_dynamic_viscosity)

#define S_MV                       (&sed_cell_mv)
#define S_CV                       (&sed_cell_cv)
///Function to get the shear strength of a cell.
#define S_SHEAR_STRENGTH           (&sed_cell_shear_strength)
///Function to get the sediment cohesion of a cell.
#define S_COHESION                 (&sed_cell_cohesion)
///Function to get the degree of consolidation of a cell.
#define S_CONSOLIDATION            (&sed_cell_consolidation)
*/

//@}

/** Function to be called when calculating the poroperty of a Sed_cell.

\param s   the Sed_cell that is being examined.
\param sed the type of Sediment that is in the cell.
\param n   the number of types of Sediment in the Sed_cell.

\return the calculated parameter.
*/
/*
typedef double (*Sed_cell_property_func_0) ( const Sed_cell );
typedef double (*Sed_cell_property_func_1) ( const Sed_cell , double );
typedef double (*Sed_cell_property_func_2) ( const Sed_cell , double , double );

typedef union
{
   Sed_cell_property_func_0 f_0;
   Sed_cell_property_func_1 f_1;
   Sed_cell_property_func_2 f_2;
}
Sed_cell_property_func;
*/

/** Function to be called when calculating the poroperty of a Sed_cell when the load is a factor.

\param s    is the Sed_cell that is being examined.
\param load is the load felt by the cell.
\param sed  the type of Sediment that is in the cell.
\param n    is the number of types of Sediment in the Sed_cell.

\return the calculated perameter.
*/
/*
typedef double (*Sed_avg_property_with_load_func) ( const Sed_cell c   ,
                                                    double load        ,
                                                    const Sediment sed ,
                                                    int n );
*/

/** Function to calculate a bulk sediment property of a Sed_cell

Calculate the bulk property of a Sed_cell.  These functions calculate the property of
the entire Sed_cell at once rather than calculating the property for each grain type
and averaging the results.

\param s   the Sed_cell that is being examined.
\param sed the type of Sediment that is in the cell.

\return the calculated property
*/
//typedef double (*Sed_bulk_property_func) ( const Sed_cell s, const Sediment sed );

/** Function to calculate a bulk sediment property of a Sed_cell with a load parameter

Same as Sed_bulk_property_func but with an added \a load parameter.  \a load is meant
to represent the load that the Sed_cell feels but could be any value that the function
requires to calculate the sediment property.

\param s     the Sed_cell that is being examined.
\param load  the load felt by the Sed_cell.
\param sed   the type of Sediment that is in the cell.

\return the calculated property
*/
/*
typedef double (*Sed_bulk_property_with_load_func) ( const Sed_cell c ,
                                                     double load      ,
                                                     const Sediment sed );
*/

/** Class to hold the function used to get a sediment property from a Sed_cell
*/
/*
typedef union
{
   Sed_avg_property_func             f_avg; /// Average the properties for each grain type to get the total
   Sed_avg_property_with_load_func   f_avg_load; /// Average the properties for each grain type (with an extra load parameter) to get the total
   Sed_bulk_property_func            f_bulk; /// Get a bulk value of a sediment property from a Sed_cell
   Sed_bulk_property_with_load_func  f_bulk_load; /// Get a bulk value of a sediment property from a Sed_cell (with an extra load parameter)
}
Sed_property_func;
*/

/** Class to describe a sediment property of a Sed_cell
*/
/*
typedef struct
{
   char *name;             ///< The name of the property
   char *ext;              ///< The file extension for the property
   Sed_property_func f;    ///< The function used to get the property from a Sed_cell
   gssize n_args;
}
Sed_property;
*/

//Sed_property sed_property_by_name( const char* name );

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

//Sed_cell* sed_create_cell_vector  ( int,int,int);
//int       sed_destroy_cell_vector ( Sed_cell ,int,int,int);
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

//Sed_size_class sed_cell_size_class    ( const Sed_cell c );
//Sed_size_class sed_size_class         ( const double d );
//double         sed_size_class_percent ( Sed_size_class sed_type , const Sed_cell c , const Sediment sed );

void         sed_cell_fprint            ( FILE* , const Sed_cell );
gssize       sed_cell_write             ( FILE* , const Sed_cell );
Sed_cell     sed_cell_read              ( FILE* );

//Cell_list sed_create_cell_list(int size,int n_grains);
//Sed_cell **sed_destroy_cell_list(Cell_list c_list);

Sed_cell *sed_cell_list_new( gssize len , gssize n );
Sed_cell* sed_cell_list_destroy(Sed_cell *c_list);

//double sed_cell_property               ( const Sed_property,const Sed_cell,const Sediment,...);
//double sed_cell_property_with_load     ( Sed_avg_property_with_load_func,const Sed_cell,const Sediment,double);

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

/*
///Function to get bulk density.
extern const Sed_property S_DENSITY;
///Function to get cell grain density.
extern const Sed_property S_GRAIN_DENSITY;
///Function to get cell 'closest packed' density.
extern const Sed_property S_MAX_DENSITY;
///Function to get cell 'closest packed' density.
extern const Sed_property S_GRAIN_SIZE_IN_PHI;
///Function to get cell grain size in phi units.
extern const Sed_property S_GRAIN_SIZE_IN_M;
///Function to get the fraction of sand in a cell.
extern const Sed_property S_SAND_FRACTION;
///Function to get the fraction of silt in a cell.
extern const Sed_property S_SILT_FRACTION;
///Function to get the fraction of clay in a cell.
extern const Sed_property S_CLAY_FRACTION;
///Function to get the fraction of mud (silt and clay) in a cell.
extern const Sed_property S_MUD_FRACTION;
///Function to get the average velocity of sound in the cell.
extern const Sed_property S_VELOCITY;
///Function to get the average viscosity of the cell.
extern const Sed_property S_VISCOSITY;
///Function to get the cell density relative to its uncompacted state.
extern const Sed_property S_RELATIVE_DENSITY;
///Function to get the porosity of the cell.
extern const Sed_property S_POROSITY;
///Function to get the porosity of the cell in its closest packed state.
extern const Sed_property S_POROSITY_MIN;
///Function to get the porosity of the cell in its (loosest) uncompacted state.
extern const Sed_property S_POROSITY_MAX;
///Function to get the plastic index of the cell (obsolete).
extern const Sed_property S_PLASTIC_INDEX;
///Function to get the permeability of the cell.
extern const Sed_property S_PERMEABILITY;
///Function to get the permeability of the cell.
extern const Sed_property S_HYDRAULIC_CONDUCTIVITY;
///Function to get the void ratio of a cell.
extern const Sed_property S_VOID_RATIO;
///Function to get the minimum possible void ratio of a cell.
extern const Sed_property S_VOID_RATIO_MIN;
///Function to get the maximum possible void ratio of a cell.
extern const Sed_property S_VOID_RATIO_MAX;
///Function to get the friction angle of a cell.
extern const Sed_property S_FRICTION_ANGLE;
///Function to get the consolidation coefficient of a cell.
extern const Sed_property S_CC;
///Function to get the yield strength of a cell.
extern const Sed_property S_YIELD_STRENGTH;
///Function to get the dynamic viscosity of a cell.
extern const Sed_property S_DYNAMIC_VISCOSITY;

extern const Sed_property S_MV;
extern const Sed_property S_CV;
///Function to get the shear strength of a cell.
extern const Sed_property S_SHEAR_STRENGTH;
///Function to get the sediment cohesion of a cell.
extern const Sed_property S_COHESION;
///Function to get the degree of consolidation of a cell.
extern const Sed_property S_CONSOLIDATION;

extern const Sed_property S_AGE;
extern const Sed_property S_EXCESS_PRESSURE;
extern const Sed_property S_RELATIVE_PRESSURE;
extern const Sed_property S_COMPRESSIBILITY;
extern const Sed_property S_FRACTION;
*/
#endif /* sed_cell.h is included */

