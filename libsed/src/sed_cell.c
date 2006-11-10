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

#include "sed_cell.h"

CLASS ( Sed_cell )
{
   gssize n;          ///< the number of grain types is the cell.
   double *f;         ///< the fraction of each grain size in the cell.
   double t_0;        ///< the initial thickness of the cell, before any compaction has occured.
   double t;          ///< the current thickness of the cell.
   double age;        ///< the average age of the sediemnt in the cell.
   double pressure;   ///< the excess porewater pressure in the cell.
   Sed_facies facies; ///< the facies designation of the cell.
};

//@Include: sed_cell.h
/*
///Function to get the fraction of sand in a cell.
#define  S_SAND_FRACTION_INIT { S_PROPERTY_ID_SAND_FRACTION , \
                           "sand"                      , \
                           "sand"                      , \
                           0                           , \
                           {&sed_cell_sand_fraction} }
///Function to get the fraction of silt in a cell.
#define  S_SILT_FRACTION_INIT { S_PROPERTY_ID_SILT_FRACTION , \
                           "silt"                      , \
                           "silt"                      , \
                           0                           , \
                           {&sed_cell_silt_fraction} }
///Function to get the fraction of clay in a cell.
#define  S_CLAY_FRACTION_INIT { S_PROPERTY_ID_CLAY_FRACTION , \
                           "clay"                      , \
                           "clay"                      , \
                           0                           , \
                           {&sed_cell_clay_fraction} }
///Function to get the fraction of mud (silt and clay) in a cell.
#define  S_MUD_FRACTION_INIT { S_PROPERTY_ID_MUD_FRACTION  , \
                          "mud"                       , \
                          "mud"                       , \
                          0                           , \
                          {&sed_cell_mud_fraction} }
///Function to get the average velocity of sound in the cell.
#define  S_VELOCITY_INIT { S_PROPERTY_ID_VELOCITY      , \
                         "velocity"                  , \
                         "velocity"                  , \
                         0                           , \
                         {&sed_cell_velocity} }
///Function to get the average viscosity of the cell.
#define  S_VISCOSITY_INIT { S_PROPERTY_ID_VISCOSITY     , \
                         "viscosity"                 , \
                         "viscosity"                 , \
                         0                           , \
                         {&sed_cell_viscosity} }
///Function to get the cell density relative to its uncompacted state.
#define  S_RELATIVE_DENSITY_INIT { S_PROPERTY_ID_RELATIVE_DENSITY , \
                              "relative density"             , \
                              "dr"                           , \
                              0                              , \
                              {&sed_cell_relative_density} }
///Function to get the porosity of the cell.
#define  S_POROSITY_INIT { S_PROPERTY_ID_POROSITY , \
                         "porosity"             , \
                         "porosity"             , \
                         0                      , \
                         {&sed_cell_porosity} }
///Function to get the porosity of the cell in its closest packed state.
#define  S_POROSITY_MIN_INIT { S_PROPERTY_ID_POROSITY_MIN , \
                          "porosity min"             , \
                          "poremin"                  , \
                          0                          , \
                          {&sed_cell_porosity_min} }
///Function to get the porosity of the cell in its (loosest) uncompacted state.
#define  S_POROSITY_MAX_INIT { S_PROPERTY_ID_POROSITY_MAX , \
            "porosity max" , "poremax" , 0 , {&sed_cell_porosity_max} }
///Function to get the plastic index of the cell (obsolete).
#define  S_PLASTIC_INDEX_INIT { S_PROPERTY_ID_PLASTIC_INDEX , \
            "pi" , "pi" , 0 , {&sed_cell_plastic_index} }
///Function to get the permeability of the cell.
#define  S_PERMEABILITY_INIT { S_PROPERTY_ID_PERMEABILITY , \
            "permeability" , "perm" , 2 , {(Sed_avg_property_func)(&sed_cell_bulk_permeability)} }
///Function to get the permeability of the cell.
#define  S_HYDRAULIC_CONDUCTIVITY_INIT { \
            S_PROPERTY_ID_HYDRAULIC_CONDUCTIVITY , \
            "hydraulic conductivity"             , \
            "hydro"                              , \
            2                                    , \
            {(Sed_avg_property_func)(&sed_cell_bulk_hydraulic_conductivity)} }
///Function to get the void ratio of a cell.
#define  S_VOID_RATIO_INIT { S_PROPERTY_ID_VOID_RATIO , \
                                    "void ratio"             , \
                                    "void"                   , \
                                    0                        , \
                                    {&sed_cell_void_ratio} } 
///Function to get the minimum possible void ratio of a cell.
#define  S_VOID_RATIO_MIN_INIT { S_PROPERTY_ID_VOID_RATIO_MIN , \
                                     "void ratio min"             , \
                                     "voidmin"                    , \
                                     0                            , \
                                     {&sed_cell_void_ratio_min} }
///Function to get the maximum possible void ratio of a cell.
#define  S_VOID_RATIO_MAX_INIT { S_PROPERTY_ID_VOID_RATIO_MAX , \
                                     "void ratio max"             , \
                                     "voidmax"                    , \
                                     0                            , \
                                     {&sed_cell_void_ratio_max} }
///Function to get the friction angle of a cell.
#define  S_FRICTION_ANGLE_INIT { S_PROPERTY_ID_FRICTION_ANGLE , \
                                     "friction angle"             , \
                                     "angle"                      , \
                                     0                            , \
                                     {&sed_cell_friction_angle} }
///Function to get the consolidation coefficient of a cell.
#define  S_CC_INIT { S_PROPERTY_ID_CC            , \
                                     "consolidation coefficient" , \
                                     "cc"                        , \
                                     0                           , \
                                     {&sed_cell_cc} }
///Function to get the yield strength of a cell.
#define  S_YIELD_STRENGTH_INIT { S_PROPERTY_ID_YIELD_STRENGTH , \
                                     "yield strength"             , \
                                     "yield"                      , \
                                     0                            , \
                                     {&sed_cell_yield_strength} }
///Function to get the dynamic viscosity of a cell.
#define  S_DYNAMIC_VISCOSITY_INIT { S_PROPERTY_ID_DYNAMIC_VISCOSITY , \
                                     "dynamic viscosity"            , \
                                     "nu"                           , \
                                     0                               , \
                                     {&sed_cell_dynamic_viscosity} }

#define  S_MV_INIT { S_PROPERTY_ID_MV , \
                            "mv"             , \
                            "mv"             , \
                            0                , \
                            {(Sed_avg_property_func)(&sed_cell_mv)} }
#define  S_CV_INIT { S_PROPERTY_ID_CV , \
                            "cv"             , \
                            "cv"             , \
                            2                , \
                            {(Sed_avg_property_func)(&sed_cell_bulk_cv)} }
///Function to get the shear strength of a cell.
#define  S_SHEAR_STRENGTH_INIT { S_PROPERTY_ID_SHEAR_STRENGTH , \
                                     "shear"                      , \
                                     "shear"                      , \
                                     1                            , \
                                     {(Sed_avg_property_func)(&sed_cell_shear_strength)} }
///Function to get the sediment cohesion of a cell.
#define  S_COHESION_INIT { S_PROPERTY_ID_COHESION , \
                           "cohesion"             , \
                           "cohesion"             , \
                           1                      , \
                           {(Sed_avg_property_func)(&sed_cell_cohesion)} }
///Function to get the degree of consolidation of a cell.
#define  S_CONSOLIDATION_INIT { S_PROPERTY_ID_CONSOLIDATION , \
                                "consolidation"             , \
                                "con"                       , \
                                1                           , \
                                {(Sed_avg_property_func)(&sed_cell_consolidation)} }
///Function to get the degree of consolidation rate of a cell.
#define  S_CONSOLIDATION_RATE_INIT { S_PROPERTY_ID_CONSOLIDATION_RATE , \
                                     "consolidation rate"             , \
                                     "du"                       , \
                                     1                           , \
                                     {(Sed_avg_property_func)(&sed_cell_consolidation_rate)} }
#define S_AGE_INIT { S_PROPERTY_ID_AGE , \
                     "age"             , \
                     "age"             , \
                     2                 , \
                     {(Sed_avg_property_func)(&sed_cell_age_helper)} }
#define S_FACIES_INIT { S_PROPERTY_ID_FACIES , \
                     "facies"             , \
                     "facies"             , \
                     2                 , \
                     {(Sed_avg_property_func)(&sed_cell_facies_helper)} }
#define S_PRESSURE_INIT { S_PROPERTY_ID_PRESSURE , \
                     "pressure"                 , \
                     "pressure"                 , \
                     3                          , \
                     {(Sed_avg_property_func)(&sed_cell_pressure_helper)} }
#define S_EXCESS_PRESSURE_INIT { S_PROPERTY_ID_EXCESS_PRESSURE , \
                     "excess pressure"             , \
                     "excess"                      , \
                     3                             , \
                     {(Sed_avg_property_func)(&sed_cell_excess_pressure_helper)} }
#define S_RELATIVE_PRESSURE_INIT { S_PROPERTY_ID_RELATIVE_PRESSURE , \
                     "relative pressure"             , \
                     "rel"                           , \
                     3                               , \
                     {(Sed_avg_property_func)(&sed_cell_relative_pressure_helper)} }
#define S_COMPRESSIBILITY_INIT { S_PROPERTY_ID_COMPRESSIBILITY , \
                     "compressibility"             , \
                     "c"                           , \
                     0                             , \
                     {(Sed_avg_property_func)(&sed_cell_compressibility)} }
#define S_FRACTION_INIT { S_PROPERTY_ID_FRACTION , \
                          "fraction"             , \
                          "fraction"             , \
                          0                      , \
                          {(Sed_avg_property_func)(&sed_cell_nth_fraction)} }
#define S_BAD_VAL_INIT { S_PROPERTY_ID_BAD_VAL   , \
                         NULL                    , \
                         NULL                    , \
                         -1                      , \
                         { NULL } }

///Function to get bulk density.
const Sed_property S_DENSITY = S_DENSITY_INIT; 
///Function to get cell grain density.
const Sed_property S_GRAIN_DENSITY = S_GRAIN_DENSITY_INIT; 
///Function to get cell 'closest packed' density.
const Sed_property S_MAX_DENSITY = S_MAX_DENSITY_INIT; 
///Function to get cell 'closest packed' density.
const Sed_property S_GRAIN_SIZE_IN_PHI = S_GRAIN_SIZE_IN_PHI_INIT; 
///Function to get cell grain size in phi units.
const Sed_property S_GRAIN_SIZE_IN_M = S_GRAIN_SIZE_IN_M_INIT; 
///Function to get the fraction of sand in a cell.
const Sed_property S_SAND_FRACTION = S_SAND_FRACTION_INIT; 
///Function to get the fraction of silt in a cell.
const Sed_property S_SILT_FRACTION = S_SILT_FRACTION_INIT; 
///Function to get the fraction of clay in a cell.
const Sed_property S_CLAY_FRACTION = S_CLAY_FRACTION_INIT; 
///Function to get the fraction of mud (silt and clay) in a cell.
const Sed_property S_MUD_FRACTION = S_MUD_FRACTION_INIT; 
///Function to get the average velocity of sound in the cell.
const Sed_property S_VELOCITY = S_VELOCITY_INIT; 
///Function to get the average viscosity of the cell.
const Sed_property S_VISCOSITY = S_VISCOSITY_INIT; 
///Function to get the cell density relative to its uncompacted state.
const Sed_property S_RELATIVE_DENSITY = S_RELATIVE_DENSITY_INIT; 
///Function to get the porosity of the cell.
const Sed_property S_POROSITY = S_POROSITY_INIT; 
///Function to get the porosity of the cell in its closest packed state.
const Sed_property S_POROSITY_MIN = S_POROSITY_MIN_INIT; 
///Function to get the porosity of the cell in its (loosest) uncompacted state.
const Sed_property S_POROSITY_MAX = S_POROSITY_MAX_INIT; 
///Function to get the plastic index of the cell (obsolete).
const Sed_property S_PLASTIC_INDEX = S_PLASTIC_INDEX_INIT; 
///Function to get the permeability of the cell.
const Sed_property S_PERMEABILITY = S_PERMEABILITY_INIT; 
///Function to get the permeability of the cell.
const Sed_property S_HYDRAULIC_CONDUCTIVITY = S_HYDRAULIC_CONDUCTIVITY_INIT; 
///Function to get the void ratio of a cell.
const Sed_property S_VOID_RATIO = S_VOID_RATIO_INIT;
///Function to get the minimum possible void ratio of a cell.
const Sed_property S_VOID_RATIO_MIN = S_VOID_RATIO_MIN_INIT; 
///Function to get the maximum possible void ratio of a cell.
const Sed_property S_VOID_RATIO_MAX = S_VOID_RATIO_MAX_INIT; 
///Function to get the friction angle of a cell.
const Sed_property S_FRICTION_ANGLE = S_FRICTION_ANGLE_INIT; 
///Function to get the consolidation coefficient of a cell.
const Sed_property S_CC = S_CC_INIT; 
///Function to get the yield strength of a cell.
const Sed_property S_YIELD_STRENGTH = S_YIELD_STRENGTH_INIT; 
///Function to get the dynamic viscosity of a cell.
const Sed_property S_DYNAMIC_VISCOSITY = S_DYNAMIC_VISCOSITY_INIT; 

const Sed_property S_MV = S_MV_INIT; 
const Sed_property S_CV = S_CV_INIT; 
///Function to get the shear strength of a cell.
const Sed_property S_SHEAR_STRENGTH = S_SHEAR_STRENGTH_INIT; 
///Function to get the sediment cohesion of a cell.
const Sed_property S_COHESION = S_COHESION_INIT; 
///Function to get the degree of consolidation of a cell.
const Sed_property S_CONSOLIDATION = S_CONSOLIDATION_INIT; 
///Function to get the degree of consolidation of a cell.
const Sed_property S_CONSOLIDATION_RATE = S_CONSOLIDATION_RATE_INIT; 

const Sed_property S_AGE = S_AGE_INIT;
const Sed_property S_PRESSURE = S_PRESSURE_INIT;
const Sed_property S_EXCESS_PRESSURE = S_EXCESS_PRESSURE_INIT;
const Sed_property S_RELATIVE_PRESSURE = S_RELATIVE_PRESSURE_INIT;
const Sed_property S_COMPRESSIBILITY = S_COMPRESSIBILITY_INIT;
const Sed_property S_FRACTION = S_FRACTION_INIT;
*/

#include <stdlib.h>
/**
\brief Create a new cell of sediment.

\param n_grains the number of Sediment types held in the cell.

\return sed_create_cell will return a newly created cell.  NULL is returned if
there was a problem allocating the memory.

*/
Sed_cell sed_cell_new( gssize n_grains )
{
   Sed_cell c = NULL;

   if ( n_grains>0 )
   {
      NEW_OBJECT( Sed_cell , c );
   
      c->f = eh_new0( double , n_grains );
   
      c->n        = n_grains;
      c->t_0      = 0.;
      c->t        = 0.;
      c->age      = 0.;
      c->pressure = 0.;
      c->facies   = S_FACIES_NOTHING;
   }
   
   return c;
}

Sed_cell sed_cell_new_env( void )
{
   Sed_cell c = NULL;

   if ( sed_sediment_env_is_set() )
      c = sed_cell_new( sed_sediment_env_size() );

   return c;
}

Sed_cell sed_cell_new_sized( gssize n , double t , double* f )
{
   Sed_cell c = sed_cell_new( n );

   sed_cell_resize      ( c , t );
   sed_cell_set_fraction( c , f );

   return c;
}

Sed_cell sed_cell_new_typed( Sed_sediment s , double t , Sed_type a_type )
{
   Sed_cell c = NULL;

   if ( !s )
      s = sed_sediment_env();

   {
      gssize i;
      gssize len = sed_sediment_n_types(s);
      double* f = eh_new0( double , len );

      for ( i=0 ; i<len ; i++ )
         if ( sed_type_is_same( sed_sediment_type( s , i ) , a_type ) )
            f[i] = 1.;

      c = sed_cell_new_sized( len , t , f );

      eh_free( f );
   }

   return c;
}

Sed_cell sed_cell_new_classed( Sed_sediment s , double t , Sed_size_class class )
{
   Sed_cell c = NULL;

   if ( !s )
      s = sed_sediment_env();

   if ( s )
   {
      gssize i;
      gssize len = sed_sediment_n_types(s);
      double* f = eh_new0( double , len );

      for ( i=0 ; i<len ; i++ )
         if ( sed_type_is_size_class( sed_sediment_type( s , i ) , class ) )
            f[i] = 1.;

      eh_dbl_array_normalize( f , len );

      c = sed_cell_new_sized( len , t , f );

      eh_free( f );
   }

   return c;
}

/**
\brief The copy constructor for a Sed_cell.

Duplicates the contents of a Sed_cell, returning a newly allocated cell or
NULL if an error occured.  The return value should be freed when no longer
in needed.

\param n the number of Sediment types held in the cell.
\return sed_dup_cell will return a newly created cell that is a copy s.  NULL
is returned if there was a problem allocating the memory.

\see Sed_cell , sed_destroy_cell
*/
Sed_cell sed_cell_dup( Sed_cell c )
{
   Sed_cell dup_cell = NULL;

   eh_require( c )
   {
      dup_cell = sed_cell_copy( NULL , c );
   }

   return dup_cell;
}

/** \brief Create a Cell_list

Create a list of Sed_cell s.  The elements of the array will be pointers
to Sed_cell s and will be NULL terminated.  A newly created Cell_list is 
returned.  When this Cell_list is no longer used it should be freed with
sed_destroy_cell_list .

\param size     The number of Sed_cell s to put in the list.
\param n_grains The number of sediment types in each Sed_cell .

\return A new list of Cell_list .

\see Cell_list, sed_destroy_cell_list
*/
Sed_cell *sed_cell_list_new( gssize len , gssize n )
{
   Sed_cell *c_list = NULL;

   eh_require( len>0 )
   {
      gssize i;

      c_list = eh_new( Sed_cell , len+1 );

      for ( i=0 ; i<len ; i++ )
         c_list[i] = sed_cell_new( n );
      c_list[len]=NULL;
   }

   return c_list;
}

/** \brief Destroy a Cell_list
 Free resources used for a Cell_list.

\param c_list A Cell_list that was created with sed_create_cell_list .

\see Cell_list , sed_create_cell_list .
*/
Sed_cell* sed_cell_list_destroy(Sed_cell *c_list)
{
   eh_require( c_list )
   {
      gssize i;

      for ( i=0 ; c_list[i] ; i++ )
         c_list[i] = sed_cell_destroy( c_list[i] );
      eh_free(c_list);
   }

   return NULL;
}

/** Destroy a Sed_cell.

Frees the memory used by a Sed_cell.  The pointer to the Sed_cell itself is 
also freed.

\param c A pointer to the Sed_cell to be freed. 

*/
Sed_cell sed_cell_destroy( Sed_cell c )
{
   if ( c )
   {
      eh_free( c->f );
      eh_free( c );
   }
   
   return NULL;   
}

void sed_cell_fprint( FILE* fp , const Sed_cell c )
{
   if ( c )
   {
      gssize n;

      fprintf( fp , "Thickness : %f\n" , c->t );
      fprintf( fp , "Age       : %f\n" , c->age );
      fprintf( fp , "Fraction: : %f"   , c->f[0] );
      for ( n=1 ; n<sed_cell_n_types(c) ; n++ )
         fprintf( fp , ", %f" , c->f[n] );
      fprintf( fp , "\n" );
   }
   else
      fprintf( fp , "( null )\n" );
}

/** Remove the sediment from a cell.

Remove all of the sediment from a cell.  This clears the Sed_cell so that it
looks just like a newly allocated cell created by sed_create_cell.

\param c A pointer to the cell to clear.
\param n The number of sediment types in the cell.

\return A pointer to the cleared cell, c.

\see Sed_cell, sed_create_cell.

*/
Sed_cell sed_cell_clear( Sed_cell c )
{
   eh_require( c )
   {
      gssize n;
   
      for ( n=0 ; n<sed_cell_n_types(c) ; n++ )
         c->f[n] = 0;

      c->t        = 0;
      c->t_0      = 0;
      c->age      = 0;
      c->pressure = 0.;
      c->facies   = S_FACIES_NOTHING;
   }
   
   return c;
}

/** Clear each of the cells of sediment in an array of Sed_cells.

Clear each of the cell of sediment in an (NULL terminated) array of Sed_cells.
The function sed_clear_cell is called for for each element of the array.

\param v    The NULL terminated array of Sed_cell s.
\param low  The index into v to start clearing from.
\param high The index into v to stop clearing at.
\param len  The length of the array, v.
\param n    The number of sediment types in each Sed_cell.

\see Sed_cell , sed_clear_cell

*/
Sed_cell *sed_cell_clear_vector(Sed_cell *vec,int low,int high )
{
   int i;
   for (i=low;i<=high;i++)
      sed_cell_clear( vec[i] );
   return vec;
}

/** Copy a Sed_cell.

Copy the Sed_cell src to dest.  The pointers of sediment type fractions are not
copied, only their elements.  Thus, the number of sediment types allocated for
by each Sed_cell must be the same (ie the parameter n in sed_create_cell ).


\param dest A pointer to the destination Sed_cell .
\param src  A pointer to the Sed_cell to be copied.
\param n    The number of sediment types in each Sed_cell .

\see Sed_cell , sed_create_cell .
*/
Sed_cell sed_cell_copy( Sed_cell dest , const Sed_cell src )
{
   eh_require( src )

   if ( dest != src )
   {
      if ( !dest )
         dest = sed_cell_new( sed_cell_n_types(src) );
      else
         eh_require( sed_cell_is_compatible( dest , src ) );

      memcpy( dest->f , src->f , sed_cell_n_types(src)*sizeof(double) );

      dest->n        = src->n;
      dest->t_0      = src->t_0;
      dest->t        = src->t;
      dest->age      = src->age;
      dest->pressure = src->pressure;
      dest->facies   = src->facies;

   }
   
   return dest;
}

/** Set Sed_cell members.

Use these functions to set the members of a Sed_cell .

\see Sed_cell .
*/

//@{
/** Set the age of a cell.
\param c   The Sed_cell to set.
\param val The new age value.
\return The Sed_cell that was set.
*/
Sed_cell sed_cell_set_age( Sed_cell c , double age )
{
   eh_require( c );
   c->age = age;
   return c;
}

/** Sed the thickness of a cell.
\param c   The Sed_cell to set.
\param val The new thickness value.
\return The Sed_cell that was set.
*/
Sed_cell sed_cell_set_thickness( Sed_cell c , double t )
{
   eh_require( c );
   c->t = t;
   return c;
}

/** Sed the pressure of a cell.
\param c   The Sed_cell to set.
\param val The new pressure value.
\return The Sed_cell that was set.
*/
Sed_cell sed_cell_set_pressure( Sed_cell c , double p )
{
   eh_require( c );
   c->pressure = p;
   return c;
}

/** Sed the facies designation of a cell.
\param c   The Sed_cell to set.
\param val The new facies to add to the cell.
\return The Sed_cell that was set.
*/
Sed_cell sed_cell_set_facies( Sed_cell c , Sed_facies f )
{
   eh_require( c );
   c->facies = f;
   return c;
}

Sed_cell sed_cell_add_facies( Sed_cell c , Sed_facies f )
{
   eh_require( c );
   c->facies |= f;
   return c;
}

/** Set the fractions of each sediment type found in a cell.

Set each of the fractions of sediment types within a Sed_cell .  Each of the 
elements of f are copied to c.  Thus, f is free to be freed after 
sed_set_cell_fraction is called.

\param c   The Sed_cell to set.
\param val A vector of new fractions.
\param n   The number of sediment types in the Sed_cell .
\return The Sed_cell that was set.
*/
Sed_cell sed_cell_set_fraction(Sed_cell c , double f[] )
{
   eh_require( c );
   eh_require( f );

   {
      memcpy( c->f , f , sed_cell_n_types(c)*sizeof(double) );
   }
   
   return c;   
}

/** Set the fractions of each grain to be uniform.
\param[in,out] a The sediment cell
\param[in] n_grains The number of grain types in the cell
\return The pointer to the input cell.
*/
Sed_cell sed_cell_set_equal_fraction( Sed_cell c )
{
   eh_require( c );

   {
      gssize n, len = sed_cell_n_types(c);
      for ( n=0 ; n<len ; n++ )
         c->f[n] = 1./len;
   }
   
   return c;   
}

//@}

/** Add one Sed_cell to another.

Add the Sed_cell b to a.  Both a and b must contain the same number of sediment
types, n.  The Sed_cell a will contain the total amount of sediment while b
remains unchanged.  The new age (pressure) of a will be a weighted average of
the ages (pressures) of a and b based on their relative amounts.  The new 
facies will be a combination of the facies of the two cells.  That is, the new
facies of a will include both the facies of a and b.

\param a The destination cell.
\param b The source cell.
\param n The number of sediment types in each cell.

\see Sed_cell.
*/
Sed_cell sed_cell_add( Sed_cell a , const Sed_cell b )
{
   eh_require( a );
//   eh_require( b );

   {
      if ( b && !sed_cell_is_empty(b) )
      {
         gssize n;
         gssize len = sed_cell_n_types( b );
         double ratio = a->t / b->t;
      
         eh_require( sed_cell_is_compatible( a , b ) );

         for ( n=0 ; n<len ; n++ )
            a->f[n] = (a->f[n]*ratio + b->f[n])/(ratio+1.);

         a->t        = a->t   + b->t;
         a->t_0      = a->t_0 + b->t_0;
         a->age      = (a->age*ratio + b->age) / (ratio + 1.);
         a->pressure = (a->pressure*ratio + b->pressure) / (ratio + 1.);
         a->facies   = a->facies | b->facies;
      }
   }

   return a;
}

gboolean sed_cell_is_empty( Sed_cell c )
{
   return c->t < 1e-12;
}

gboolean sed_cell_is_clear( Sed_cell c )
{
   gboolean is_clear = FALSE;

   if ( c )
   {
      if ( sed_cell_is_empty(c) )
      {
         gssize n;
         gssize len = sed_cell_n_types(c);

         is_clear = TRUE;

         for ( n=0 ; is_clear && n<len ; n++ )
            if ( c->f[n] > 1e-12 )
               is_clear = FALSE;
      }
   }
   else
      is_clear = TRUE;

   return is_clear;
}

gboolean sed_cell_is_size( Sed_cell c , double t )
{
   return eh_compare_dbl( sed_cell_size(c) , t , 1e-12 );
}

gboolean sed_cell_is_age( Sed_cell c , double a )
{
   return eh_compare_dbl( sed_cell_age(c) , a , 1e-12 );
}

gboolean sed_cell_is_mass( Sed_cell c , double m )
{
   return eh_compare_dbl( sed_cell_mass(c) , m , 1e-12 );
}

gboolean sed_cell_is_size_class( Sed_cell c , Sed_size_class size )
{
   return sed_cell_size_class( c )&size;
}

gboolean sed_cell_is_compatible( Sed_cell a , Sed_cell b )
{
   return sed_cell_n_types(a) == sed_cell_n_types(b);
}

/** Remove sediment from one cell and put it in another by specifying the amount of each type

Remove sediment from the source cell by specifying the amount of each type of sediment sediment
to remove.  The sediment is removed with the same make-up of the original cell.

\note If out_cell is non-NULL, the cell is cleared before any sediment is added to it.

\param a The source cell
\param thickness The amount of each sediment type to be removed
\param n_grains The number of grain types in each cell
\param out_cell The location to put the removed sediment.  If NULL, a new cell is created.

\return The destination cell.
*/
Sed_cell sed_cell_separate_amount( Sed_cell in ,
                                   double t[]  ,
                                   Sed_cell out )
{
   eh_require( in );
   eh_require( t  );

   {
      if ( !sed_cell_is_empty( in ) )
      {
         gssize n;
         gssize len   = sed_cell_n_types( in );
         double *f    = eh_new0( double , len );
         double total = sed_cell_size( in );

         for ( n=0 ; n<len ; n++ )
         {
            if ( in->f[n]>0. )
               f[n] = t[n]/(total*in->f[n]);
//            f[n] = t[n]/total;
            eh_clamp( f[n] , 0. , 1. );
         }

         out = sed_cell_separate_fraction( in , f , out );

         eh_free( f );
      }
      else
      {
         out = sed_cell_copy( out , in );
         sed_cell_resize( out , 0. );
      }
   }

   return out;
}

/** Remove an amount of sediment from one cell and put it in another

Remove sediment from the source cell by specifying the amount of sediment to remove.  The sediment
is removed with the same make-up of the original cell.

\note If out_cell is non-NULL, the cell is cleared before any sediment is added to it.

\param a The source cell
\param thickness The total amount of sediment to be removed
\param n_grains The number of grain types in each cell
\param out_cell The location to put the removed sediment.  If NULL, a new cell is created.

\return The destination cell.
*/

#define eh_bound( x , low , high ) ( (x<low)?(low):( (x>high)?(high):(x) ) )

Sed_cell sed_cell_separate_thickness( Sed_cell in ,
                                      double t    ,
                                      Sed_cell out )
{
   eh_require( in );

   {
      double total    = sed_cell_thickness( in );
      double in_size  = eh_bound( total - t , 0 , total );
      double out_size = total - in_size;

      out = sed_cell_copy( out , in );

      sed_cell_resize( in  , in_size  );
      sed_cell_resize( out , out_size );
   }

   return out;
}

/** Remove fractions of sediment from one cell and put it in another

Remove sediment from the source cell by specifying fractions of each grain type to remove.

\note If out_cell is non-NULL, the cell is cleared before any sediment is added to it.

\param a The source cell
\param fraction The fraction of each grain type to remove
\param n_grains The number of grain types in each cell
\param out_cell The location to put the removed sediment.  If NULL, a new cell is created.

\return The destination cell.

*/
Sed_cell sed_cell_separate_fraction( Sed_cell in ,
                                     double f[] ,
                                     Sed_cell out )
{
   eh_require( in );
   eh_require( f  );

   {
      out = sed_cell_copy( out , in );

      if ( !sed_cell_is_empty( in ) )
      {
         gssize n;
         gssize len     = sed_cell_n_types( in );
         double* in_t   = eh_new( double , len );
         double* out_t  = eh_new( double , len );
         double in_size = sed_cell_size( in );

         for ( n=0 ; n<len ; n++ )
         {
            out_t[n] = f[n]     *in_size*in->f[n];
            in_t[n]  = (1.-f[n])*in_size*in->f[n];
         }

         sed_cell_set_amount( in  , in_t  );
         sed_cell_set_amount( out , out_t );

         eh_free( in_t  );
         eh_free( out_t );
      }
   }

   return out;
}

Sed_cell sed_cell_set_amount( Sed_cell c , const double* t )
{
   eh_return_val_if_fail( c , NULL );
   eh_return_val_if_fail( t , c    );

   {
      gssize n;
      gssize len = sed_cell_n_types(c);
      double sum = eh_dbl_array_sum( t , len );

      if ( sum>0 )
      {
         for ( n=0 ; n<len ; n++ )
            c->f[n] = t[n] / sum;

         sed_cell_resize( c , sum );
      }
      else
         sed_cell_clear( c );
   }

   return c;
}

/** \brief Add a vector of amounts of sediment to a cell.

 Add a vector that gives the amounts of each sediment type to a Sed_cell.
The length of the vector, t, must match the number of sediment types in the
Sed_cell c.  We assume that the sediment being added is uncompacted and so,

thickness == uncompacted_thickness

for the new sediment.

\param c The Sed_cell to add the sediment to.
\param t A vector giving the amount of each grain type to add to, c.
\param n The number of sediment types in the Sed_cell.

\return A pointer to the Sed_cell, c.
*/
Sed_cell sed_cell_add_amount( Sed_cell a , const double t[] )
{
   eh_return_val_if_fail( a , NULL );
   eh_return_val_if_fail( t , a    );

   {
      gssize len = sed_cell_n_types(a);
      double sum = eh_dbl_array_sum( t , len );

      if ( sum > 0 )
      {
         gssize n;
         double new_t = sum + a->t;

         for ( n=0 ; n<len ; n++ )
            a->f[n] = ( a->f[n]*a->t + t[n] ) / new_t;

         a->t   += sum;
         a->t_0 += sum;
      }
   }

   return a;   
}

/** Remove sediment from one cell based on another cell

Sediment is removed from the source cell based on the fraction and amount of sediment
in another cell.

\param[out] a       The source cell from which sediment is removed
\param[in] b        Sediment is removed based on the make up of this cell
\param[in] n_grains The number of grain types in the cell
*/
Sed_cell sed_cell_separate_cell( Sed_cell in ,
                                 Sed_cell out )
{
   sed_cell_destroy( sed_cell_separate( in , out->f , sed_cell_size(out) , NULL ) );

   return in;
}

/** Remove sediment from one cell and place it into another

Sediment is removed from the source cell and put into the new cell.

\param[in,out] a The cell to be separated
\param[in] fraction The fraction of each grain type to remove
\param[in] thickness The amount of each grain type to remove
\param[in] n_grains The number of grain types in the cell

\return A new cell that contains the removed sediment.  The new cell should be destroyed with
sed_destroy_cell.
*/
Sed_cell sed_cell_separate( Sed_cell in ,
                            double f[]  ,
                            double t    ,
                            Sed_cell out )
{
   eh_require( in   );
   eh_require( f    );
   eh_require( t>=0 );

   out = sed_cell_copy( out , in );

   if ( !sed_cell_is_empty( in ) )
   {
      gssize n;
      gssize len   = sed_cell_n_types( in );
      double* t_rem = eh_new( double , len );
      double sum;

      for ( n=0,sum=0 ; n<len ; n++ )
         sum += f[n];

      for ( n=0 ; n<len ; n++ )
         t_rem[n] = f[n]/sum*t;

      out = sed_cell_separate_amount( in , t_rem , out );

      eh_free( t_rem );
   }

   return out;
}

/** \brief Move sediment from one cell into another.

Moves a given amount of sediment (specified as thickness) from one cell
and adds it to another.  The difference from the sed_separate functions is that
the sed_move_sediment functions add the removed sediment to any existing
sediment that is already in the destination cell.

\param dest      The destination cell where sediment is added to.
\param src       The source cell where sediment is removed from.
\param thickness The amount of sediment to transfer.
\param n_grains  The number of sediment types in the the cells.

\see sed_move_sediment_to_cell_by_fraction, sed_move_sediment_cell, sed_separate_cell_by_thickness.
*/
void sed_cell_move_thickness( Sed_cell src , Sed_cell dest , double t )
{
   eh_require( src );
   eh_require( dest );
   eh_require( sed_cell_is_compatible( src , dest ) );

   if ( t>0 )
   {
      Sed_cell temp = sed_cell_separate_thickness( src , t , NULL );
      sed_cell_add( dest , temp );
      temp = sed_cell_destroy( temp );
   }
}

/** Move sediment from one cell into another.

Moves a given amount of sediment (specified by fraction of each grain size)
from one cell and adds it to another.  The difference from the sed_separate
functions is that the sed_move_sediment functions add the removed sediment to
any existing sediment that is already in the destination cell.

\param dest      The destination cell where sediment is added to.
\param src       The source cell where sediment is removed from.
\param fraction  The fraction of each grain size to move.
\param n_grains  The number of sediment types in the the cells.

\see sed_move_sediment_to_cell_by_thickness, sed_move_sediment_cell, sed_separate_cell_by_fraction.
*/
void sed_cell_move_fraction( Sed_cell src  ,
                             Sed_cell dest ,
                             double f[] )
{
   eh_require( src  );
   eh_require( dest );
   eh_require( f    );
   eh_require( sed_cell_is_compatible( src , dest ) );

   {
      Sed_cell temp = sed_cell_separate_fraction( src , f , NULL );
      sed_cell_add( dest , temp );
      temp = sed_cell_destroy( temp );
   }
}

/** Move a Sed_cell by fractions of a thickness into another.

Move a Sed_Cell by specifying a maximum thickness to remove and then 
specifying individual fractions of that thickness to remove. That is, if
fraction is an array of ones, then it is equivalent to:
sed_move_sediment_to_cell_by_thickness( dest , src , thickness , n_grains );
If however, fraction is an array of zeros then it is equivalent to removing
nothing from src and not changing dest at all.

\param dest      The destination cell where sediment is added to.
\param src       The source cell where sediment is removed from.
\param thickness The amount of sediment to transfer.
\param n_grains  The number of sediment types in the the cells.

\see sed_move_sediment_to_cell_by_thickness, sed_move_sediment_to_cell_by_fraction, sed_separate_cell.
*/
void sed_cell_move( Sed_cell src  ,
                    Sed_cell dest ,
                    double f[]    ,
                    double t )
{
   Sed_cell temp1 = sed_cell_separate_thickness( src   , t , NULL );
   Sed_cell temp2 = sed_cell_separate_fraction ( temp1 , f , NULL );

   sed_cell_add( src  , temp1 );
   sed_cell_add( dest , temp2 );

   temp1 = sed_cell_destroy( temp1 );
   temp2 = sed_cell_destroy( temp2 );
}

///////////////////////////////////////////////////////////////////
//
// Define functions used to calculate bulk sediment properties for
// a cell of sediment.  For now these functions just average the
// the properties of each sediment type within a cell.
//

/** \name Sediment Cell Property Functions

These functions are used to get various sediment properties from a Sed_cell.

@{
*/

/** \brief Get a bulk sediment property from a Sed_cell .

Returns a bulk sediment property from a Sed_cell .  The bulk property
is calculted simply by averaging the property for each sediment type.  The
average is weighted by the relative amounts of each sediment type that is
contained in the Sed_cell .  A pointer to the function that calculates the
property for each sediment type is given as the parameter, f.

\param f   A pointer to a function that is used to calculate the sediment 
property from a single sediment type.
\param c   The Sed_cell being examined.
\param sed The type of sediment that is in the Sed_cell.

\return Returns the bulk property of the Sed_cell .

\see Sed_property_func, sed_cell_property_with_load.
*/
/*
double sed_cell_sediment_property( const Sed_cell c , const Sed_property prop , ... )
{
   double property = 0.;

   eh_require( c );

   if ( !sed_cell_is_empty( c ) )
   {
      gssize n;
      gssize n_grains = sed_cell_n_types(c);

      if ( prop.f_type == 0 )
      {
         if ( prop.id > S_PROPERTY_ID_FRACTION )
            property = (*(prop.f.f_avg))( c   ,
                                          sed ,
                                          prop.id-S_PROPERTY_ID_FRACTION );
         else
            sed_sediment_foreach( sed , prop.f.f_avg , c );
      }
      else if ( prop.f_type == 1  || prop.f_type == 3 )
      {
         double load;
         va_list arg_list;
         va_start( arg_list , sed );

         load = va_arg( arg_list , double );
         va_end( arg_list );
         if ( prop.f_type == 1 )
            for ( n=0 ; n<n_grains ; n++ )
               property += c->f[n]*(*(prop.f.f_avg_load))(c,load,sed,n);
         else
            property = (*(prop.f.f_bulk_load))( c , load , sed );
      }
      else if ( prop.f_type == 2 )
         property = (*(prop.f.f_bulk))( c , sed );
      else
         eh_require_not_reached();

   }

   return property;
}
*/
/*
double sed_cell_property( const Sed_property prop , const Sed_cell c , const Sed_sediment sed , ... )
{
   double property = 0.;

   eh_require( c   );
   eh_require( sed );

   if ( !sed_cell_is_empty( c ) )
   {
      gssize n;
      gssize n_grains = sed_cell_n_types(c);

      if ( prop.f_type == 0 )
      {
         if ( prop.id > S_PROPERTY_ID_FRACTION )
            property = (*(prop.f.f_avg))( c   ,
                                          sed ,
                                          prop.id-S_PROPERTY_ID_FRACTION );
         else
            for ( n=0 ; n<n_grains ; n++ )
               property += c->f[n]*(*(prop.f.f_avg))(c,sed,n);
      }
      else if ( prop.f_type == 1  || prop.f_type == 3 )
      {
         double load;
         va_list arg_list;
         va_start( arg_list , sed );

         load = va_arg( arg_list , double );
         va_end( arg_list );
         if ( prop.f_type == 1 )
            for ( n=0 ; n<n_grains ; n++ )
               property += c->f[n]*(*(prop.f.f_avg_load))(c,load,sed,n);
         else
            property = (*(prop.f.f_bulk_load))( c , load , sed );
      }
      else if ( prop.f_type == 2 )
         property = (*(prop.f.f_bulk))( c , sed );
      else
         eh_require_not_reached();

   }

   return property;
}
*/

/** Get a bulk sediment property that requires the overlying load from a Sed_cell.

 Returns a bulk sediment property from a Sed_cell .  Use this function
when the load felt by the cell is needed to calculate the bulk property.  
The bulk property is calculted simply by averaging the property for each
sediment type.  The average is weighted by the relative amounts of each
sediment type that is contained in the Sed_cell .  A pointer to the function
that calculates the property for each sediment type is given as the parameter,
\p f.

\param f    A pointer to a function that is used to calculate the sediment 
property from a single sediment type.
\param c    The Sed_cell being examined.
\param sed  The type of sediment that is in the Sed_cell.
\param load The load felt by the Sed_cell .

\return Returns the bulk property of the Sed_cell .

\see Sed_property_func , sed_cell_property .
*/
/*
double sed_cell_property_with_load( Sed_avg_property_with_load_func f ,
                                    const Sed_cell c                  ,
                                    const Sediment sed                ,
                                    double load )
{
   double property = 0.;

   eh_require( f );
   eh_require( c );

   if ( !sed_cell_is_empty( c ) )
   {
      gssize n;
      gssize n_grains=sed_cell_n_types(c);

      for ( n=0 ; n<n_grains ; n++ )
         property += c->f[n]*(*f)(c,load,sed,n);
   }

   return property;
}
*/
//
// End of bulk property functions
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
// These are the pre-defined S_property_func's
//
//   functions are of the form:
//   double f(Sed_cell*,sediment,n)
//

/** \brief Get the density of a Sed_cell .
\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The number of sediment types in the cell.

\return Returns the bulk density of the cell.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_density_0( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_density_0 );
}

/** \brief Get the density of a sediment type in a Sed_cell .

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the density of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_grain_density( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_rho_grain );
}

/** \brief Get the closest packed density of a sediment type in a Sed_cell .

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the closest packed density of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_max_density( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_rho_max );
}

/** \brief Get the mean grain size of a sediment type in a  Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the mean grain size of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_grain_size( const Sed_cell c )
{
   double g = 0;

   if ( c )
      g = sed_sediment_property_avg( NULL , c->f , &sed_type_grain_size );

   return g;
}

/** \brief Get the mean grain size (in phi units) of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the mean grain size of the sediment type in phi units.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_grain_size_in_phi( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_grain_size_in_phi );
}

double sed_cell_sand_fraction( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_is_sand );
}

double sed_cell_silt_fraction( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_is_silt );
}

double sed_cell_clay_fraction( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_is_clay );
}

double sed_cell_mud_fraction( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_is_mud );
}

double sed_cell_nth_fraction( const Sed_cell c , gssize n )
{
   return sed_cell_fraction( c , n );
}

double* sed_cell_fraction_ptr( const Sed_cell c )
{
   return c->f;
}

/** Copy the fraction infrormation of a Sed_cell

Copy the fraction of each grain type held in a Sed_cell.  If the input pointer
is NULL, a newly created array is allocated.

@param f A pointer to the location to copy data (or NULL)
@param c A Sed_cell

@return A pointer to the copied data
*/
double* sed_cell_copy_fraction( double* f , const Sed_cell c )
{
   eh_require( c );

   if ( !f )
      f = eh_new( double , sed_cell_n_types(c) );
   memcpy( f , c->f , sizeof(double)*sed_cell_n_types(c) );

   return f;
}

/** \brief Get the coefficient of consolidation for a sediment type in a cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the coefficient consolidation of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_c_consolidation( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_c_consolidation );
}

/** \brief Get the velocity of water of a sediment type in a  Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the velocity of water of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_velocity( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_velocity );
}

/** \brief Get the viscosity of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the viscosity of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_viscosity( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_viscosity );
}

/** \brief Get the relative density of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the relative density of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_relative_density( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_relative_density );
}

/** \brief Get the porosity of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The number of sediment types in the cell.

\return Returns the porosity of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_porosity( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_porosity );
}

/** \brief Get the maximum porosity of a sediment type in a Sed_cell.

 Returns the porosity of a Sed_cell when it is in its uncompacted state.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the maximum porosity of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_porosity_max( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_porosity_max );
}

/** \brief Get the minimum porosity of a sediment type in a Sed_cell.

 Returns the porosity of a cell if it were in its closest packed state.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the minumum porosity of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_porosity_min( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_porosity_min );
}

/** \brief Get the plastic index of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the plastic index of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_plastic_index( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_plastic_index );
}

#define S_F 1.25

/** \brief Get the permeability of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the permeability of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_permeability( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_permeability );
}

/** \brief Get the hydraulic conductivity of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the hydraulic conductivity of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_hydraulic_conductivity( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_hydraulic_conductivity );
}

/** \brief Get the permeability of a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.

\return Returns the permeability of the cell.
*/
double sed_cell_bulk_permeability( const Sed_cell c )
{ 
   double s;
   double e = sed_cell_void_ratio( c );

   s = 6.*sed_sediment_property_avg( NULL , c->f , &sed_type_inv_grain_size_in_meters );

  return 1./(5.*S_F*s*s)*(pow(e,3.)/(1+e));
}

double sed_cell_bulk_log_permeability( const Sed_cell c )
{
   double k = sed_cell_bulk_permeability( c );
   return ( k<=0 )?eh_nan():log(k);
}

double sed_cell_bulk_hydraulic_conductivity( const Sed_cell c )
{
   return sed_cell_bulk_permeability( c ) * S_GAMMA_WATER / S_ETA_WATER;
}

/** \brief Get the void ratio of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the void ratio of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_void_ratio( const Sed_cell c )
{
   double e = sed_sediment_property_avg( NULL , c->f , &sed_type_void_ratio );
   return ( c->t/c->t_0 ) * ( 1.+e ) - 1.;
}

/** \brief Get the minimum void ratio of a sediment type in a Sed_cell.

 Returns the void ratio of a Sed_cell if it were in its closest packed state.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the minimum void ratio of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_void_ratio_min( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_void_ratio_min );
}

/** \brief Get the maximum void ratio of a sediment type in a Sed_cell.

 Returns the void ratio of a cell if it were in its uncompacted state.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the maximum void ratio of the sediment type.

\see sed_cell_property , sed_cell_property_with_load .
*/
double sed_cell_void_ratio_max( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_void_ratio_max );
}

/** \brief Get the Coulomb friction angle of a sediment type in a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the Coulomb friction angle of the sediment type.
*/
double sed_cell_friction_angle( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_friction_angle );
}

double sed_cell_cc( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_c_consolidation );
}

double sed_cell_compressibility( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_compressibility );
}

/** \brief Get the yield strength of a sediment type in a Sed_cell.


this formlation is taken from 'erosion and sedimentation' by pierre julien
(p. 190). the yieled strength (in pa) of a debris flow.  that is, it should
be the remolded yield strength of the sediment which is different from the
yield strength of the sediment before failure. the concentration here is 
the volume concentration of sediment.  that is, the volume of sediment 
divided by the total volume (of sediment plus water); this is the same as
one minus the porosity.

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the yield strength of the sediment type.
*/
double sed_cell_yield_strength( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_yield_strength );
}

/** \brief Get the bulk yield strength of a Sed_cell.

\param c   The cell to measure.
\param sed The type of sediment in the cell.

\return Returns the bulk yield strength of the cell.
*/
double sed_cell_bulk_yield_strength( const Sed_cell c )
{
   double a, f_sand;
   double conc = 1. - sed_cell_porosity( c );

   f_sand = sed_cell_sand_fraction( c );

   if ( f_sand > .99 )
      a = 3.;
   else
   {
      double f_clay = sed_cell_clay_fraction( c );
      double f_silt = sed_cell_silt_fraction( c );

      conc *= (f_silt+f_clay);

      if ( f_silt/(f_silt+f_clay) > .95 )
         a = 13.;
      else
         a = 23;
   }

   conc = 1. - sed_cell_porosity( c );
   a = 13.;

   return .1*exp( a*( conc - .05 ) );
}

/** \brief Get the dynamic viscosity of a sediment type in a Sed_cell .


this formlation is taken from 'erosion and sedimentation' by pierre 
julien (p. 190).

\param c   The cell to measure.
\param sed The type of sediment in the cell.
\param n   The sediment type to measure.

\return Returns the dynamic viscosity of the sediment type.
*/
double sed_cell_dynamic_viscosity( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_dynamic_viscosity );
}

/** \brief Get the bulk dynamic viscosity of a Sed_cell .

\param c   The cell to measure.
\param sed The type of sediment in the cell.

\return Returns the bulk dynamic viscosity of the Sed_cell .
*/
double sed_cell_bulk_dynamic_viscosity( const Sed_cell c )
{
   double a, f_sand;
   double conc = 1.-sed_cell_porosity( c );

   f_sand = sed_cell_sand_fraction( c );

   if ( f_sand > .95 )
      a = 10.;
   else
   {
      conc *= (1.-f_sand);
      a = 23.;
   }

   conc = 1.-sed_cell_porosity( c );
   a = 10.;

   return S_ETA_WATER*( 1. + 2.5*conc + exp( a*( conc - .05 ) ) );
}

double sed_cell_relative_pressure( const Sed_cell c , double load )
{
   double pressure = sed_cell_pressure( c );

   if ( load <= 0 )
      return 0;
   else
      return pressure / load;
}

/** \brief Get the Wentworth size class for a Sed_cell .


Returns the Wentworth size class for the sediment in a cell.  The mean grain
size of the sediment contained in a Sed_cell is averaged (in terms of phi
units).  The mean grain size is then placed into a sand (phi<4), silt (4<phi<9), or clay (phi>9) class.

\param c   The Sed_cell to measure.
\param sed The type of sediment in the Sed_cell.

\see sed_get_size_class .

*/
Sed_size_class sed_cell_size_class( const Sed_cell c )
{
   Sed_size_class class = S_SED_TYPE_NONE;

   if ( c )
   {
      double d_mean = sed_sediment_property_avg( NULL , c->f , &sed_type_grain_size_in_phi );
      class = sed_size_class( d_mean );
   }

   return class;
}

/** \brief Return the percentage of a certain size class with a Sed_cell.

 Retruns the fraction of a given size class (from the Wentworth scale)
contained in a Sed_cell.

\param sed_type The size class to look for.
\param c        The Sed_cell to examine.
\param sed      The type of sediment in the Sed_cell.

\return The percentage of the size class in the Sed_Cell.

\see sed_cell_size_class , sed_get_size_class , Sed_size_class .
*/
double sed_cell_size_class_percent( const Sed_cell c , Sed_size_class size )
{
   double p = 0.;

   if ( c )
      p = sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_is_size_class_with_data , &size );
   return p;
}

Sed_size_class sed_cell_size_classes( const Sed_cell c )
{
   Sed_size_class size = S_SED_TYPE_NONE;

   if ( c )
      sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_sum_size_classes_with_data , &size );
   return size;
}

double sed_cell_density( const Sed_cell c )
{
   double d = c->t / c->t_0;
   return sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_density_compacted , &d );
}

double sed_cell_sediment_volume( const Sed_cell c )
{
   double v = 0.;

   if ( c )
      v = c->t / ( sed_cell_void_ratio(c) + 1 );

   return v;
}

double sed_cell_sediment_mass( const Sed_cell c )
{
   double m = 0;

   if ( c )
      m = sed_cell_grain_density(c)*sed_cell_sediment_volume(c);

   return m;
}

//
// End of S_property_func's
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
// These are the pre-defined S_property_with_load_func's
//
//   functions are of the form:
//   double f(Sed_cell*,double,sediment,n)
//

double sed_cell_mv( const Sed_cell c )
{
/*
   double e=sed_cell_void_ratio(c,sed,n);
   double Cc=sed_cell_cc(c,sed,n);
   return .435 / (1+e) * ( Cc / load );
*/
   return sed_sediment_property_avg( NULL , c->f , &sed_type_compressibility );
}

double sed_cell_cv( const Sed_cell c )
{
   return sed_sediment_property_avg( NULL , c->f , &sed_type_cv );
}

double sed_cell_bulk_cv( const Sed_cell c )
{
   double mv = sed_cell_mv( c );
   return sed_cell_bulk_hydraulic_conductivity( c ) / ( S_GAMMA_WATER * mv );
}

/** \brief Get the shear strength of a sediment type in a Sed_cell .

\param c    The cell to measure.
\param load The load on the cell.
\param sed  The type of sediment in the cell.
\param n    The sediment type to measure.

\return Returns the shear strength of the sediment type.
*/
double sed_cell_shear_strength( const Sed_cell c , double load )
{
   return sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_shear_strength_with_data , &load );
}

/** \brief Get the cohesion of a sediment type in a Sed_cell .

\param c    The cell to measure.
\param load The load on the cell.
\param sed  The type of sediment in the cell.
\param n    The sediment type to measure.

\return Returns the cohesion of the sediment type.
*/
double sed_cell_cohesion(const Sed_cell c, double load )
{
   load -= c->pressure;
   eh_lower_bound( load , 0 );

   return sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_cohesion_with_data , &load );
//   cohesion = load * tan(sed_cell_friction_angle(c,sed,n)*M_PI/180.) - sed_cell_shear_strength(c,load,sed,n);
}

/** \brief Get the degree of consolidation of a sediment type in a Sed_cell .


this is taken from bardet, experimental soil mechanics, p. 312.
get the degree of consolidation for a cell of sediment.  the excess porewater
pressure, u_e is related to the degree of consolidation as,
   u_e = s ( 1 - u )
where s is the load on the cell.

\param c         The cell to measure.
\param time_now  The current time.
\param sed       The type of sediment in the cell.
\param n         The sediment type to measure.

\return Returns the degree of consolidation.
*/
double sed_cell_consolidation( const Sed_cell c , double time_now )
{
   double dt = time_now - sed_cell_age_in_years( c );
   double d  = sed_cell_thickness(c);
   double data[2];

   data[0] = d;
   data[1] = dt;

   return sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_consolidation_with_data , data );
}

/* now defined in sed_property.c
double sed_calculate_avg_consolidation( double c_v , double d , double t )
{
   double t_v = c_v*t/(d*d);
   if ( d <= 0 )
      return 1.;
   if ( t_v < .2827 )
      return ( sqrt(4./G_PI*t_v) );
   else
      return ( 1-8/(G_PI*G_PI)*exp(-G_PI*G_PI/4.*t_v) );
}

double sed_calculate_consolidation( double c_v , double d , double z , double t )
{
   double t_v = c_v*t/(d*d);
   double n, eps = G_MAXDOUBLE;
   double u, u_0;

   if ( d <= 0 )
      return 1.;
   if ( t <= 0 )
      return 0;

   for ( n=1,u=0 ; eps>1e-3 ; n+=2 )
   {
      u_0 = u;
      u += 1./n*sin( n*G_PI*z/d )*exp( -pow(n*G_PI*.5,2)*t_v );
      eps = fabs( ( u_0 - u ) / u );
   }
   u *= 4/G_PI;

   return 1-u;
}
*/

double sed_cell_consolidation_rate( const Sed_cell c , double time_now )
{
   double dt = time_now - sed_cell_age_in_years( c );
   double d  = sed_cell_thickness(c);
   double data[2];

   data[0] = d;
   data[1] = dt;

   return sed_sediment_property_avg_with_data( NULL , c->f , &sed_type_consolidation_rate_with_data , data );
}

//@}

//
// End of S_property_with_load_func's
//
///////////////////////////////////////////////////////////////////

//@{
/** Get the current thickness of a cell.
\param c The Sed_cell to query.
*/
double sed_cell_thickness(const Sed_cell c)
{
   if ( G_LIKELY(c) )
      return c->t;
   else
      return 0;
}

double sed_cell_size( const Sed_cell c )
{
   if ( G_LIKELY(c) )
      return c->t;
   else
      return 0;
}

double sed_cell_size_0( const Sed_cell c )
{
   if ( G_LIKELY(c) )
      return c->t_0;
   else
      return 0;
}

/** Get the fraction of a cell composed of a sediment type.
\param c The Sed_cell to query.
\param n The sediment type to query.
*/
double sed_cell_fraction( const Sed_cell c , gssize n )
{
   eh_require( c );
   eh_require( n>=0 );
   eh_require( n<sed_cell_n_types(c) );

   return c->f[n];
}

/** Get the pressure felt by a Sed_cell .
\param The Sed_cell to query.
*/
double sed_cell_pressure( const Sed_cell c )
{
   eh_require( c );
   return c->pressure;
}

double sed_cell_excess_pressure( const Sed_cell c   ,
                                 double hydro_static )
{
   double pressure;
   eh_require( c );
   
   pressure = sed_cell_pressure(c) - hydro_static;

   return (pressure<0)?0:pressure;
}

/** Get the age of a Sed_cell .
\param c The Sed_cell to query.
*/
double sed_cell_age(const Sed_cell c)
{
   eh_require( c );
   return c->age;
}

gssize sed_cell_n_types( const Sed_cell c )
{
   eh_require( c );
   return c->n;
}

/** Get the age of a Sed_cell in years.
\param c The Sed_cell to query.
*/
double sed_cell_age_in_years( const Sed_cell c )
{
   eh_require( c );
   return c->age;
}

/** Get the facies designation of a cell.
\param c The Sed_cell to query.
*/
Sed_facies sed_cell_facies( const Sed_cell c )
{
   eh_require( c );
   return c->facies;
}

/** Get the mass of a Sed_cell .

Calculate the mass of sediment in a cell.  mass is in units of kg/m^2.

\param c The Sed_cell to query.
\param s The type of Sediment in the cell.
*/
double sed_cell_mass( const Sed_cell c )
{
   double mass = 0;

   eh_require( c )
   {
      if ( !sed_cell_is_empty(c) )
         mass = c->t*sed_cell_density( c );
   }

   return mass;
}

/** Get the load of a Sed_cell .

Get the load that a cell of sediment exerts.  load is in N/m/m or Pa.

\param c The Sed_cell to query.
\param s The type of Sediment in the cell.
*/
double sed_cell_load( const Sed_cell c )
{
   return sed_cell_mass(c)*sed_gravity();
}

double sed_cell_sediment_load( const Sed_cell c )
{
   return sed_cell_sediment_mass(c)*sed_gravity();
}
//@}

/*******

 Remove a fraction of each grain size from a sediment cell.

*******/
/*
double sed_seperate_cell(Sed_cell *s,const double fraction[],int n_grain,Sed_cell *remCell)
{
   double sumLeft, sumRem;
   long n;
   double total_thickness, total_uncompacted_thickness;
   
   total_thickness = s->thickness;
   total_uncompacted_thickness = s->uncompacted_thickness;
   
   for (n=0,sumLeft=0.,sumRem=0.;n<n_grain;n++)
   {
      remCell->fraction[n] = s->fraction[n]*fraction[n];
      s->fraction[n]       = s->fraction[n]*(1.-fraction[n]);
      sumLeft += s->fraction[n];
      sumRem  += remCell->fraction[n];
   }
   
   remCell->thickness             = total_thickness*sumRem;
   remCell->uncompacted_thickness = total_uncompacted_thickness*sumRem;
   remCell->age                   = s->age;
   remCell->facies                = s->facies;
   remCell->pressure              = s->pressure;

   s->thickness             = total_thickness*sumLeft;
   s->uncompacted_thickness = total_uncompacted_thickness*sumLeft;
   
   if ( sumRem == 0 )
      memset(remCell->fraction,0,n_grain*sizeof(double));
   else
      for (n=0;n<n_grain;n++)
         remCell->fraction[n] /= sumRem;
         
   if ( sumLeft == 0 )
      memset(s->fraction,0,n_grain*sizeof(double));
   else
      for (n=0;n<n_grain;n++) 
         s->fraction[n]      /= sumLeft;
   
   return sumLeft;
}
*/

/** \brief Dump the contents of a Sed_cell to a binary file.

 Dump the contents of a Sed_cell to a binary file.  The contents can 
be read back into the Sed_cell from a file using sed_load_cell .

\param fp       A file pointer to the file to dump the information.
\param c        The Sed_cell to dump.
\param n_grains The number of sediment types in the Sed_cell .

\see sed_load_cell .
*/
gssize sed_cell_write( FILE *fp, const Sed_cell c )
{
   gssize n = 0;

   eh_require( fp );
   eh_require( c  );

   n += fwrite( &c->n        , sizeof(gssize)        , 1    , fp );
   n += fwrite( c->f         , sizeof(double)        , c->n , fp );
   n += fwrite( &c->t_0      , sizeof(double)        , 1    , fp );
   n += fwrite( &c->t        , sizeof(double)        , 1    , fp );
   n += fwrite( &c->age      , sizeof(double)        , 1    , fp );
   n += fwrite( &c->pressure , sizeof(double)        , 1    , fp );
   n += fwrite( &c->facies   , sizeof(unsigned char) , 1    , fp );

   eh_require( n == 6+c->n );

   return n;
}

/** \brief Read the contents of a Sed_cell from a binary file.

 Read the contents of a Sed_cell from a binary file.  The information
must have been written with sed_dump_cell .  A newly created Sed_cell is 
returned that contains the information read from the file.  This Sed_cell
should be destroyed with sed_destroy_cell when no longer in use.

\param fp       A pointer to a file to read the data from.
\param n_grains The number of sediment types in the Sed_cell .

\return A newly created Sed_cell containing the contents from the file.

\see sed_dump_cell .
*/
Sed_cell sed_cell_read( FILE *fp )
{
   Sed_cell c;

   eh_require( fp );

   {
      gssize n;

      fread( &n , sizeof(gssize) , 1 , fp );

      eh_require( n>0 );

      c = sed_cell_new( n );

      fread( c->f         , sizeof(double)        , n , fp );
      fread( &c->t_0      , sizeof(double)        , 1 , fp );
      fread( &c->t        , sizeof(double)        , 1 , fp );
      fread( &c->age      , sizeof(double)        , 1 , fp );
      fread( &c->pressure , sizeof(double)        , 1 , fp );
      fread( &c->facies   , sizeof(Sed_facies)    , 1 , fp );
   }

   return c;
}

/** \brief Change the size of a cell.

Change the size of a Sed_cell .

\note The current thickness of a Sed_cell is changed <em>while retaining its
degree of compactedness </em>.

\param[in,out] c The Sed_cell to resize.
\param[in] t The new thickness of the Sed_cell .
\return The resized Sed_cell.

\see sed_compact_cell .
*/
Sed_cell sed_cell_resize( Sed_cell c , double t )
{
   eh_require( c );

   if ( t>0 )
   {
      if ( c->t > 0 )
      {
         double ratio = c->t_0 / c->t;
         c->t   = t;
         c->t_0 = c->t*ratio;
      }
      else
      {
         c->t   = t;
         c->t_0 = t;
      }
   }
   else
      sed_cell_clear( c );

   return c;
}

/** \brief Compact a cell of sediment.

Compact a Sed_cell.

\note The current thickness of the Sed_cell is changed <em> but the uncompacted
thickness of the cell is unaltered </em>.

\param c The Sed_cell to compact.
\param t The new thickness of the Sed_cell.

\return The compacted Sed_cell.

\see sed_resize_cell .
*/
Sed_cell sed_cell_compact( Sed_cell c , double new_t )
{
   eh_require( c );
   eh_require( new_t>=0 );

   eh_lower_bound( new_t , 0 );

//   c->pressure  *= c->t/new_t;
   c->t  = new_t;

   return c;
}

/** \brief Create a two-dimensional array of Sed_cell's.

 Create a two-dimensional array of pointer to Sed_cell's.  Only resources
needed to hold the pointers are allocated, the pointers are not initialized.
To create pointers to Sed_cell's, use sed_init_cell_grid once the grid has
been created.

\param n_x The size of the slow-dimension.
\param n_y The size of the fast-dimension.

\return A pointer to a newly allocated grid.

\see Sed_cell_grid , sed_init_cell_grid , sed_free_cell_grid .
*/
Sed_cell_grid sed_cell_grid_new( gsize n_x , gsize n_y )
{
   Sed_cell_grid g = eh_grid_new( Sed_cell , n_x , n_y );
   return g;
}

/** \brief Initalize a two-dimensional array of Sed_cell's.

 Initialize a Sed_cell_grid with new Sed_cell's.  The input grid must first
have been created with sed_create_cell_grid .

\param g        A pointer to a Sed_cell_grid.
\param n_grains The number of grain types for each Sed_cell.

\return A pointer to a newly initialized grid.

\see Sed_cell_grid , sed_create_cell_grid , sed_free_cell_grid .
*/
Sed_cell_grid sed_cell_grid_init( Sed_cell_grid g , gssize n_grains )
{
   eh_require( g );
   eh_require( n_grains>0 );

   eh_return_val_if_fail( g , NULL )
   {
      Sed_cell* data = eh_grid_data_start(g);
      gssize i, n_i = eh_grid_n_el( g );

      for ( i=0 ; i<n_i ; i++ )
         data[i] = sed_cell_new( n_grains );

   }

   return g;
}

/** \brief Free the resources used for each element of a Sed_cell_grid.

 Destroy each element of a Sed_cell_grid.  The resources used for the grid
itself is not freed.  To free the Sed_cell_grid as well, use sed_destroy_grid .

\param g A pointer to a Sed_cell_grid.

\see Sed_cell_grid , sed_destroy_grid .
*/
void sed_cell_grid_free( Sed_cell_grid g )
{
   eh_return_if_fail( g )
   {
      Sed_cell* data = eh_grid_data_start(g);
      gssize i, n_i = eh_grid_n_el( g );

      for ( i=0 ; i<n_i ; i++ )
         data[i] = sed_cell_destroy( data[i] );

   }
}

void sed_cell_grid_free_data( Sed_cell_grid g )
{
   eh_require( g )
   {
      gssize i;
      gssize n_i  = eh_grid_n_el( g );
      Sed_cell* c = eh_grid_data_start( g );

      for ( i=0 ; i<n_i ; i++ )
         sed_cell_destroy( c[i] );
   }

   return;
}

Sed_cell sed_cell_grid_val( Sed_cell_grid g , gssize i , gssize j )
{
   return ((Sed_cell*)eh_grid_row(g,i))[j];
}

Sed_cell_grid sed_cell_grid_add( Sed_cell_grid g_1 , Sed_cell_grid g_2 )
{
   gssize i;
   gssize n_i    = eh_grid_n_el( g_1 );
   Sed_cell* c_1 = eh_grid_data_start( g_1 );
   Sed_cell* c_2 = eh_grid_data_start( g_2 );

   eh_require( eh_grid_is_compatible( g_1 , g_2 ) )
   {
      for ( i=0 ; i<n_i ; i++ )
         sed_cell_add( c_1[i] , c_2[i] );
   }

   return g_1;
}

Sed_cell_grid sed_cell_grid_copy_data( Sed_cell_grid dest , Sed_cell_grid src )
{
   gssize i;
   gssize n_i    = eh_grid_n_el( dest );
   Sed_cell* c_1 = eh_grid_data_start( dest );
   Sed_cell* c_2 = eh_grid_data_start( src  );

   eh_require( eh_grid_is_compatible( dest , src ) )
   {
      for ( i=0 ; i<n_i ; i++ )
         sed_cell_copy( c_1[i] , c_2[i] );
   }

   return dest;
}

Sed_cell_grid sed_cell_grid_clear( Sed_cell_grid g )
{
   eh_require( g )
   {
      gssize i;
      gssize n_i  = eh_grid_n_el( g );
      Sed_cell* c = eh_grid_data_start( g );

      for ( i=0 ; i<n_i ; i++ )
         sed_cell_clear( c[i] );
   }

   return g;
}

double sed_cell_grid_mass( Sed_cell_grid g )
{
   double sum = 0;

   eh_require( g )
   {
      gssize i;
      gssize n_i  = eh_grid_n_el( g );
      Sed_cell* c = eh_grid_data_start( g );

      for ( i=0 ; i<n_i ; i++ )
         sum += sed_cell_mass( c[i] );
   }

   return sum;
}

/** The data part of the grid

Get the data member of a Sed_cell_grid

\param g A Sed_cell_grid

\see Sed_cell_grid , sed_destroy_grid .
*/
Sed_cell** sed_cell_grid_data( Sed_cell_grid g )
{
   return (Sed_cell**)eh_grid_data( g );
}

gboolean sed_cell_is_same( Sed_cell a , Sed_cell b )
{
   gboolean same = TRUE;

   eh_require( a );
   eh_require( b );

   if ( a != b )
   {
      if (    sed_cell_n_types(a) != sed_cell_n_types(b)
           || fabs( sed_cell_thickness(a)-sed_cell_thickness(b) )>1e-6
           || fabs( sed_cell_age(a) - sed_cell_age(b) ) >1e-6
           || sed_cell_facies(a) != sed_cell_facies(b) )
         same = FALSE;
      else
      {
         gssize n;
         gssize len = sed_cell_n_types(a);

         for ( n=0 ; same && n<len ; n++ )
         {
            if ( fabs( a->f[n]-b->f[n] ) > 1e-6 )
               same = FALSE;
         }
      }
   }

   return same;
}

gboolean sed_cell_is_valid( Sed_cell c )
{
   gboolean is_valid = TRUE;

   if ( c )
   {
      if ( sed_cell_n_types(c)<=0 || sed_cell_size(c)<0 )
         is_valid = FALSE;
      else
      {
         gssize n;
         gssize len = sed_cell_n_types(c);
         double sum = 0;

         for ( n=0 ; is_valid && n<len ; n++ )
         {
            if ( c->f[n]<0. || c->f[n]>1. )
               is_valid = FALSE;
            else
               sum += c->f[n];
         }

         if ( is_valid && fabs( sum-1. ) > 1e-6 && !sed_cell_is_clear(c) )
            is_valid = FALSE;
      }
   }
   else
      is_valid = FALSE;

   return is_valid;
}




