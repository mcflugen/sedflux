#include "sed_property.h"

typedef struct
{
   const gchar* name;   ///< The name of the property
   const gchar* ext;    ///< The file extension for the property
   Sed_cell_property_func f;      ///< The function used to get the property from a Sed_cell
   gssize n_args; ///< Number of args for the property function
} Sed_property_static;

/** Class to describe a sediment property of a Sed_cell
*/
CLASS( Sed_property )
{
   gchar*                 name;   ///< The name of the property
   gchar*                 ext;    ///< The file extension for the property
   Sed_cell_property_func f;      ///< The function used to get the property from a Sed_cell
   gssize                 n_args; ///< Number of args for the property function
};

//const Sed_property S_AGE = sed_property_new_full( "age" , "age" , &sed_cell_age , 1 );

#ifdef __cplusplus
static Sed_property_static all_properties[41] = {
   {"age", "age", sed_cell_age, 1},
   {"shear_strength", "sheer", sed_cell_shear_strength, 2},
   {"facies", "facies", (Sed_cell_property_func_0)sed_cell_facies, 1},
   {"pressure", "press", sed_cell_pressure, 1},
   {"density", "bulk", sed_cell_density, 1},
   {"grain_density", "rhograin", sed_cell_grain_density, 1 } ,
   {"max_density", "rho_max", sed_cell_max_density, 1 } ,
   {"grain", "grain", sed_cell_grain_size_in_phi, 1 } ,
   {"grain_in_meters", "grain", sed_cell_grain_size, 1 } ,
   {"sand", "sand"     , sed_cell_sand_fraction, 1 } ,
   {"silt", "silt"     , sed_cell_silt_fraction, 1 } ,
   {"clay", "clay"     , sed_cell_clay_fraction, 1 } ,
   {"mud", "mud"      , sed_cell_mud_fraction, 1 } ,
   {"velocity", "vel"      , sed_cell_velocity, 1 } ,
   {"viscosity", "visc"     , sed_cell_viscosity, 1 } ,
   {"relative_density", "dr"       , sed_cell_relative_density, 1 } ,
   {"porosity", "por"      , sed_cell_porosity, 1 } ,
   {"porosity_min", "pormin"   , sed_cell_porosity_min, 1 } ,
   {"porosity_max", "pormax"   , sed_cell_porosity_max, 1 } ,
   {"pi", "pi"       , sed_cell_plastic_index, 1 } ,
   {"permeability", "perm"     , sed_cell_permeability, 1 } ,
   {"void_ratio", "void"     , sed_cell_void_ratio, 1 } ,
   {"void_ratio_min", "emin"     , sed_cell_void_ratio_min, 1 } ,
   {"void_ratio_max", "emax"     , sed_cell_void_ratio_max, 1 } ,
   {"friction_angle", "angle"    , sed_cell_friction_angle, 1 } ,
   {"consolidation", "cc"       , sed_cell_cc, 1 } ,
   {"yield_strength", "yield"    , sed_cell_yield_strength, 1 } ,
   {"dynamic_viscosity", "nu"       , sed_cell_dynamic_viscosity, 1 } ,
   {"mv", "mv"       , sed_cell_compressibility, 1 } ,
   {"cv", "cv"       , sed_cell_cv, 1 } ,
   {"cv", "cv"       , sed_cell_bulk_cv, 1 } ,
   {"hydraulic_con", "hydro"    , sed_cell_bulk_hydraulic_conductivity, 1 } ,
   {"shear_strength", "sheer", sed_cell_shear_strength, 2},
   {"cohesion", "cohesion", sed_cell_cohesion, 2},
   {"consolidation", "con", sed_cell_consolidation, 2},
   {"consolidation_rate", "du", sed_cell_consolidation_rate, 2},
   {"excess_pressure", "excess", sed_cell_excess_pressure, 2},
   {"relative_pressure", "rel", sed_cell_relative_pressure, 2},
   {"fraction", "fraction", (Sed_cell_property_func_0)sed_cell_fraction, 2},
   {NULL, NULL, (Sed_cell_property_func_0)NULL, 0}
};

#else
/*#ifndef __cplusplus*/
static Sed_property_static all_properties[41] = {
   {"age", "age",
    {(Sed_cell_property_func_0)(&sed_cell_age)}, 1},
   {"facies", "facies",
    {(Sed_cell_property_func_0)(&sed_cell_facies)}, 1},
   { "pressure"           , "press"    , {&sed_cell_pressure}          , 1 } ,
   { "density"            , "bulk"     , {&sed_cell_density}           , 1 } ,
   { "grain_density"      , "rhograin" , {&sed_cell_grain_density}     , 1 } ,
   { "max_density"        , "rho_max"  , {&sed_cell_max_density}       , 1 } ,
   { "grain"              , "grain"    , {&sed_cell_grain_size_in_phi} , 1 } ,
   { "grain_in_meters"    , "grain"    , {&sed_cell_grain_size}        , 1 } ,
   { "sand"               , "sand"     , {&sed_cell_sand_fraction}     , 1 } ,
   { "silt"               , "silt"     , {&sed_cell_silt_fraction}     , 1 } ,
   { "clay"               , "clay"     , {&sed_cell_clay_fraction}     , 1 } ,
   { "mud"                , "mud"      , {&sed_cell_mud_fraction}      , 1 } ,
   { "velocity"           , "vel"      , {&sed_cell_velocity}          , 1 } ,
   { "viscosity"          , "visc"     , {&sed_cell_viscosity}         , 1 } ,
   { "relative_density"   , "dr"       , {&sed_cell_relative_density}  , 1 } ,
   { "porosity"           , "por"      , {&sed_cell_porosity}          , 1 } ,
   { "porosity_min"       , "pormin"   , {&sed_cell_porosity_min}      , 1 } ,
   { "porosity_max"       , "pormax"   , {&sed_cell_porosity_max}      , 1 } ,
   { "pi"                 , "pi"       , {&sed_cell_plastic_index}     , 1 } ,
   { "permeability"       , "perm"     , {&sed_cell_permeability}      , 1 } ,
   { "void_ratio"         , "void"     , {&sed_cell_void_ratio}        , 1 } ,
   { "void_ratio_min"     , "emin"     , {&sed_cell_void_ratio_min}    , 1 } ,
   { "void_ratio_max"     , "emax"     , {&sed_cell_void_ratio_max}    , 1 } ,
   { "friction_angle"     , "angle"    , {&sed_cell_friction_angle}    , 1 } ,
   { "consolidation"      , "cc"       , {&sed_cell_cc}                , 1 } ,
   { "yield_strength"     , "yield"    , {&sed_cell_yield_strength}    , 1 } ,
   { "dynamic_viscosity"  , "nu"       , {&sed_cell_dynamic_viscosity} , 1 } ,
   { "mv"                 , "mv"       , {&sed_cell_compressibility}   , 1 } ,
   { "cv"                 , "cv"       , {&sed_cell_cv}                , 1 } ,
   { "cv"                 , "cv"       , {&sed_cell_bulk_cv}           , 1 } ,
   { "hydraulic_con"      , "hydro"    , {&sed_cell_bulk_hydraulic_conductivity} , 1 } ,
   {"shear_strength", "sheer",
    {(Sed_cell_property_func_1)(sed_cell_shear_strength)}, 2},
   {"cohesion", "cohesion",
    {(Sed_cell_property_func_2)&sed_cell_cohesion}, 2},
   {"consolidation", "con",
    {(Sed_cell_property_func_2)&sed_cell_consolidation}, 2},
   {"consolidation_rate", "du",
    {(Sed_cell_property_func_2)&sed_cell_consolidation_rate}, 2},
   {"excess_pressure", "excess",
    {(Sed_cell_property_func_2)&sed_cell_excess_pressure}, 2},
   {"relative_pressure", "rel",
    {(Sed_cell_property_func_2)&sed_cell_relative_pressure}, 2},
   {"fraction", "fraction",
    {(Sed_cell_property_func_2)&sed_cell_fraction}, 2},
   {NULL, NULL, {NULL}, 0}};
#endif

Sed_property sed_property_new_full( char* name , char* ext , Sed_cell_property_func f , gssize n_args )
{
   Sed_property p;

   NEW_OBJECT( Sed_property , p );

   p->name   = g_strdup( name );
   p->ext    = g_strdup( ext  );
   p->f      = f;
   p->n_args = n_args;

   return p;
}

/**
   \brief Get a sediment property by its name.

   Valid sediment property names are defined by the S_*_INIT macros.

   \param name The name of the sediment property.

   \return The sediment property.
*/
Sed_property
sed_property_new( const char *name )
{
   Sed_property prop = NULL;

   eh_require( name );

   if ( name )
   {
      gssize   i;
      gboolean found      = FALSE;
      gchar**  split_name = g_strsplit( name , "=" , -1 );

      for ( i=0 ; !found && all_properties[i].name ; i++ )
         if ( g_ascii_strcasecmp( split_name[0] , all_properties[i].name )==0 )
            found = TRUE;
      i--;

      if ( !found )
      {
         eh_error( "unknown property name: %s" , split_name[0] );
         prop = NULL;
      }
      else
         prop = sed_property_dup ((Sed_property)&(all_properties[i]) );

      g_strfreev( split_name );
   }

   return prop;
}

Sed_property sed_property_copy( Sed_property dest , Sed_property src )
{
   eh_require( src );

   if ( src )
   {
      if ( !dest )
         dest = sed_property_new_full( src->name , src->ext , src->f , src->n_args );
      else
      {
         eh_free( dest->name );
         eh_free( dest->ext  );

         dest->name   = g_strdup( src->name );
         dest->ext    = g_strdup( src->ext  );
         dest->f      = src->f;
         dest->n_args = src->n_args;
      }
   }
   else
      dest = NULL;

   return dest;
}

Sed_property sed_property_dup( Sed_property src )
{
   return sed_property_copy( NULL , src );
}

gboolean sed_property_is_named( Sed_property p , const char* name )
{
   return g_ascii_strcasecmp( p->name , name )==0;
}

char* sed_property_extension( Sed_property p )
{
   char* ext = NULL;

   if ( p )
      ext = g_strdup( p->ext );

   return ext;
}

char* sed_property_name( Sed_property p )
{
   char* name = NULL;

   if ( p )
      name = g_strdup( p->name );

   return name;
}

gssize sed_property_n_args( Sed_property p )
{
   eh_return_val_if_fail( p , 0 );
   return p->n_args;
}

double sed_property_measure( Sed_property p , Sed_cell c , ... )
{
   double val = 0.;

   eh_require( p );
   eh_require( c );

   if ( p && c )
   {
      if ( p->n_args == 1 )
      {
         val = (*(p->f.f_0))( c );
      }
      else
      {
         double arg_1, arg_2;
         va_list arg_list;
         va_start( arg_list , c );

         arg_1 = va_arg( arg_list , double );

         if ( p->n_args==2 )
            val = (*(p->f.f_1))( c , arg_1 );
         else
         {
            arg_2 = va_arg( arg_list , double );
            val = p->f.f_2( c , arg_1 , arg_2 );
         }

         va_end( arg_list );
      }
   }

   return val;
}

Sed_property sed_property_destroy( Sed_property p )
{
   if ( p )
   {
      eh_free( p->name );
      eh_free( p->ext  );
      eh_free( p );
   }

   return NULL;
}

gchar**
sed_property_all_names (void)
{
  gchar** names = NULL;

  {
    int len = 0;
    int i = 0;

    for (len=0; all_properties[len].name; len++);

    names = g_new (gchar*, len+1);
    for (i=0; i<len; i++)
      names[i] = g_strdup (all_properties[i].name);
    names[len] = NULL;
  }

  return names;
}

