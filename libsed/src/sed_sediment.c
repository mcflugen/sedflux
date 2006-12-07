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

#include <stdio.h>
#include <glib.h>

#include "utils.h"

#include "sed_sediment.h"

CLASS ( Sed_type )
{
   double rho_sat;
   double rho_grain;
   double gz;
   double pi;
   double void_min;
   double diff_coef;
   double lambda;
   double c_v;
   double c;

   double w_s;
   double rho_max;
   double gz_in_m;
   double gz_in_phi;
   Sed_size_class class;
   double viscosity;
   double dynamic_viscosity;
   double relative_density;
   double void_ratio;
   double void_ratio_max;
   double porosity;
   double porosity_min;
   double permeability;
};

CLASS ( Sed_sediment )
{
   Sed_type* l;
   gssize len;
};

static Sed_sediment sed_env        = NULL;
static gboolean     sed_env_is_set = FALSE;

Sed_sediment sed_sediment_new( )
{
   Sed_sediment p;

   NEW_OBJECT( Sed_sediment , p );

   p->len = 0;
   p->l   = NULL;

   return p;
}

Sed_sediment sed_sediment_new_sized( gssize n )
{
   Sed_sediment p = NULL;

   if ( n>=0 )
   {
      p = sed_sediment_new();
      if ( n>0 )
         p = sed_sediment_resize( p , n );
   }

   return p;
}

Sed_sediment sed_sediment_set_env( Sed_sediment s )
{
   if ( !sed_env_is_set )
   {
      sed_env = sed_sediment_dup( s );
      sed_env_is_set = TRUE;
   }

   return sed_env;
}

Sed_sediment sed_sediment_unset_env( )
{
   sed_env = sed_sediment_destroy( sed_env );
   sed_env_is_set = FALSE;

   return NULL;
}

Sed_sediment sed_sediment_env( )
{
   return sed_env;
}

gboolean sed_sediment_env_is_set( )
{
   return sed_env_is_set;
}

Sed_sediment sed_sediment_copy( Sed_sediment dest , const Sed_sediment src )
{
   eh_require( src );

   if ( src )
   {
      gssize i;
      gssize n_grains = sed_sediment_n_types( src );

      if ( !dest )
         dest = sed_sediment_new( );

      dest->l   = eh_new( Sed_type , n_grains );
      dest->len = n_grains;

      for ( i=0 ; i<n_grains ; i++ )
         dest->l[i] = sed_type_dup( src->l[i] );
   }
   else
      dest = NULL;

   return dest;
}

Sed_sediment sed_sediment_dup( const Sed_sediment src )
{
   return sed_sediment_copy( NULL , src );
}

gssize sed_sediment_n_types( const Sed_sediment sed )
{
   if ( sed )
      return sed->len;
   else
      return sed_sediment_env_size();
}

gssize sed_sediment_env_size( )
{
   gssize n = 0;

   if ( sed_sediment_env_is_set() )
      n = sed_sediment_env()->len;
//   else
//      eh_warning( "A sediment environment has not been set" );

   return n;
}

Sed_sediment sed_sediment_resize( Sed_sediment s , gssize new_len )
{
   if ( new_len>0 )
   {

      if ( !s )
         s = sed_sediment_new();

      s->l   = eh_renew( Sed_type , s->l , new_len );
      s->len = new_len;
   }
   else
      s = sed_sediment_destroy( s );

   return s;
}

Sed_sediment sed_sediment_add_type( Sed_sediment sed , const Sed_type new_type )
{
   eh_require( sed      );
   eh_require( new_type );

   if ( sed && new_type )
   {
      if ( !sed_sediment_has_type(sed,new_type) )
      {
         sed = sed_sediment_resize( sed , sed->len+1 );
         sed = sed_sediment_append( sed , new_type   );
//         sed = sed_sediment_insert_sorted( sed , new_type   );
      }
   }

   return sed;
}

gboolean sed_sediment_has_type( Sed_sediment s , Sed_type t )
{
   gboolean found = FALSE;

   if ( t )
   {
      gssize i;

      for ( i=0 ; !found && i<s->len ; i++ )
         found = sed_type_is_same( s->l[i] , t );
   }

   return found;
}

Sed_sediment sed_sediment_insert_sorted( Sed_sediment s , Sed_type t )
{
   eh_require( s );
   eh_require( t );

   if ( s && t )
   {
      gssize i;
      double gz = t->gz;
      gboolean is_inserted = FALSE;

      eh_require( s->l[s->len-1] == NULL );

      if ( s->len == 1 )
      {
         s->l[0] = sed_type_dup(t);
         is_inserted = TRUE;
      }

      for ( i=s->len-2 ; !is_inserted && i>=0 ; i-- )
      {
         if ( gz > s->l[i]->gz )
         {
            s->l[i+1] = s->l[i];
            if ( i==0 )
            {
               s->l[0] = sed_type_dup(t);
               is_inserted = TRUE;
            }
         }
         else
         {
            s->l[i+1] = sed_type_dup(t);
            is_inserted = TRUE;
         }
      }
   }
   else
      s = NULL;

   return s;
}

gssize sed_sediment_fprint( FILE* fp , Sed_sediment s )
{
   gssize n = 0;

   if ( s )
   {
      gssize i;
      for ( i=0 ; i<s->len ; i++ )
      {
         fprintf( fp , "Sediment type id: %d\n" , i );
         n += sed_type_fprint( fp , s->l[i] );
      }
   }
   else
      fprintf( fp , "( null )\n" );

   return n;
}

Sed_sediment sed_sediment_append( Sed_sediment s , Sed_type t )
{
   eh_require( s );
   eh_require( t );

   if ( s && t )
      s->l[s->len-1] = sed_type_dup( t );
   else
      s = NULL;

   return s;
}

void sed_sediment_foreach( Sed_sediment s , GFunc f , gpointer user_data )
{
   eh_require( s );

   if ( s )
   {
      gssize i;
      for ( i=0 ; i<s->len ; i++ )
         (*f)( s->l[i] , user_data );
   }
}

double* sed_sediment_property( Sed_sediment s , Sed_type_property_func_0 f )
{
   double* x;

   if ( !s )
      s = sed_sediment_env();

   {
      gssize i;
      gssize len = sed_sediment_n_types(s);

      x = eh_new( double , len );

      for ( i=0 ; i<len ; i++ )
         x[i] = (*f)( s->l[i] );
   }

   return x;
}

double sed_sediment_property_avg( Sed_sediment s , double* f , Sed_type_property_func_0 p_func )
{
   double val = 0;

   {
      gssize i;

      if ( !s )
         s = sed_sediment_env();

      for ( i=0 ; i<s->len ; i++ )
         val += f[i] * (*p_func)( s->l[i] );
   }

   return val;
}

double sed_sediment_property_avg_1( Sed_sediment s , double* f , double arg_1 , Sed_type_property_func_1 p_func )
{
   double val = 0;

   {
      gssize i;

      if ( !s )
         s = sed_sediment_env();

      for ( i=0 ; i<s->len ; i++ )
         val += f[i] * (*p_func)( s->l[i] , arg_1 );
   }

   return val;
}

double sed_sediment_property_avg_2( Sed_sediment s , double* f , double arg_1 , double arg_2 , Sed_type_property_func_2 p_func )
{
   double val = 0;

   {
      gssize i;

      if ( !s )
         s = sed_sediment_env();

      for ( i=0 ; i<s->len ; i++ )
         val += f[i] * (*p_func)( s->l[i] , arg_1 , arg_2 );
   }

   return val;
}

double sed_sediment_property_avg_with_data( Sed_sediment s , double* f , Sed_type_property_func_with_data p_func , gpointer data )
{
   double val = 0;

   {
      gssize i;

      if ( !s )
         s = sed_sediment_env();

      for ( i=0 ; i<s->len ; i++ )
      {
         if ( f[i]>1e-12 )
            val += f[i] * (*p_func)( s->l[i] , data );
      }
   }

   return val;
}

Sed_type sed_sediment_type( const Sed_sediment s , gssize id )
{
   Sed_type t = NULL;

   {
      const Sed_sediment real_s = (s)?(s):(sed_sediment_env());

      eh_require( id>=0     );
      eh_require( id<real_s->len );

      if ( real_s->len>0 )
         t = real_s->l[id];
   }

   return t;
}

Sed_type sed_sediment_bedload( const Sed_sediment s )
{
   return s->l[0];
}

Sed_sediment sed_sediment_destroy( Sed_sediment s )
{
   if ( s )
   {
      gssize i;
      for ( i=0 ; i<s->len ; i++ )
         sed_type_destroy( s->l[i] );
      eh_free( s->l );
      eh_free( s    );
   }

   return NULL;
}

Sed_type sed_type_new( )
{
   Sed_type t;
   NEW_OBJECT( Sed_type , t );
   return t;
}

Sed_type sed_type_destroy( Sed_type t )
{
   if ( t )
   {
      eh_free( t );
   }

   return t;
}

#include <string.h>

Sed_type sed_type_copy( Sed_type dest , Sed_type src )
{
   eh_require( src );

   if ( !dest )
      dest = sed_type_new();

   memcpy( dest , src , sizeof( *dest ) );
   
   return dest;
}

Sed_type sed_type_dup( Sed_type src )
{
   return sed_type_copy( NULL , src );
}

gssize sed_type_fprint( FILE* fp , Sed_type t )
{
   gssize n = 0;

   if ( t )
   {
      n += fprintf( fp , "Saturated density (kg/m^3) : %f\n" , t->rho_sat   );
      n += fprintf( fp , "Grain density (kg/m^3)     : %f\n" , t->rho_grain );
      n += fprintf( fp , "Grain size (um)            : %f\n" , t->gz        );
      n += fprintf( fp , "Plastic incex (-)          : %f\n" , t->pi        );
      n += fprintf( fp , "Minimum void ratio (-)     : %f\n" , t->void_min  );
      n += fprintf( fp , "Diffusion coefficient (-)  : %f\n" , t->diff_coef );
      n += fprintf( fp , "Removal rate (1/day)       : %f\n" , t->lambda    );
      n += fprintf( fp , "Consolidation (?)          : %f\n" , t->c_v       );
      n += fprintf( fp , "Compressibility (?)        : %f\n" , t->c         );
      n += fprintf( fp , "Settling velocity (m/day)  : %f\n" , t->w_s       );
   }
   else
      n += fprintf( fp , "( null )\n" );

   return n;
}

gssize sed_type_write( FILE* fp , Sed_type t )
{
   gssize n = 0;

   eh_require( fp );
   eh_require( t  );

   if ( fp && t )
   {
      n += fwrite( t , sizeof(Sed_type) , 1 , fp );
   }

   return n;
}

Sed_type sed_type_read( FILE* fp )
{
   Sed_type t = NULL;

   if ( fp )
   {
      t = sed_type_new();

      fread( t , sizeof(Sed_type) , 1 , fp );
   }

   return t;
}

double sed_sediment_bedload_rho(const Sed_sediment s)
{
   return sed_type_rho_sat( s->l[0] );
}

gssize sed_sediment_write( FILE *fp , const Sed_sediment s )
{
   gssize n = 0;

   if ( s )
   {
      gssize i;
      gssize n_grains = sed_sediment_n_types(s);

      n += fwrite( &n_grains , sizeof(gssize) , 1 , fp );
      for ( i=0 ; i<n_grains ; i++ )
         n += sed_type_write( fp , s->l[i] );
   }

   return n;
}

Sed_sediment sed_sediment_read( FILE* fp )
{
   Sed_sediment s;

   if ( fp )
   {
      gssize n;
      gssize n_grains;
      Sed_type new_type;

      fread( &n_grains , sizeof(gint16) , 1 , fp );

      s = sed_sediment_new( );
      for ( n=0 ; n<n_grains ; n++ )
      {
         fread( &new_type , sizeof(Sed_type) , 1 , fp );
         sed_sediment_add_type( s , new_type );
      }
   }

   return s;
}

void sed_sediment_fprint_default( FILE *fp )
{
   eh_require( fp );

   if ( fp )
   {
      char *text[] = DEFAULT_SEDIMENT_FILE;
      char **p;
      for ( p = text ; *p ; p++ )
         fprintf( fp , "%s\n" , *p );
   }
}

/** Scan sediment information from a file

Scan sediment information from a file.  The file consists of a series of groups (one
for each type of sediment).  Each groups contains a key-value pairs describing the
sediment type.

An example group:
\dontinclude test.sediment
\skip start of the first group
\until rest of the groups
This is the start of the next group.

\note The order that the sediment groups appear in the file are the order they
      are placed into the Sed_sediment class.  They are no longer sorted by
      grain size.

\param file    The name of the file to scan

\return A new Sed_sediment constructed from the file.  Use sed_sediment_destroy to free.
*/
Sed_sediment sed_sediment_scan( const char *file )
{
   Sed_sediment sed = NULL;

   if ( !file )
      file = SED_SEDIMENT_TEST_FILE;

   {
      char* name_used = g_strdup( file );

      sed = sed_sediment_new();

//      eh_debug( "Read in the sediment types" );
      {
         Eh_key_file key_file = eh_key_file_scan( name_used );
         Eh_symbol_table group;
         Sed_type new_type;

         for ( group = eh_key_file_pop_group( key_file ) ;
               group ;
               group = eh_key_file_pop_group( key_file ) )
         {
            new_type = sed_type_init( group );

            sed      = sed_sediment_add_type( sed , new_type );

            group    = eh_symbol_table_destroy( group );
            new_type = sed_type_destroy       ( new_type );
         }

         eh_key_file_destroy( key_file );
      }

      eh_free( name_used );
   }

   return sed;
}

#define S_KEY_GRAIN_SIZE "grain size"
#define S_KEY_RHO_GRAIN  "grain density"
#define S_KEY_RHO_SAT    "saturated density"
#define S_KEY_VOID_MIN   "minimum void ratio"
#define S_KEY_PI         "plastic index"
#define S_KEY_DIFF_COEF  "diffusion coefficient"
#define S_KEY_LAMBDA     "removal rate"
#define S_KEY_C_V        "consolidation coefficient"
#define S_KEY_C          "compaction coefficient"

Sed_type sed_type_init( Eh_symbol_table t )
{
   Sed_type s;

   s = sed_type_new();

   s->gz        = eh_symbol_table_dbl_value( t , S_KEY_GRAIN_SIZE  );
   s->rho_grain = eh_symbol_table_dbl_value( t , S_KEY_RHO_GRAIN   );
   s->rho_sat   = eh_symbol_table_dbl_value( t , S_KEY_RHO_SAT     );
   s->void_min  = eh_symbol_table_dbl_value( t , S_KEY_VOID_MIN    );
   s->diff_coef = eh_symbol_table_dbl_value( t , S_KEY_DIFF_COEF   );
   s->lambda    = eh_symbol_table_dbl_value( t , S_KEY_LAMBDA      );
   s->c_v       = eh_symbol_table_dbl_value( t , S_KEY_C_V         );
   s->c         = eh_symbol_table_dbl_value( t , S_KEY_C           );

   s->w_s       = sed_removal_rate_to_settling_velocity( s->lambda );

   eh_require( s->rho_grain >= sed_rho_sea_water() );
   eh_require( s->rho_grain <= sed_rho_quartz()    );
   eh_require( s->rho_sat   >= sed_rho_sea_water() );
   eh_require( s->rho_sat   <= sed_rho_quartz()    );
   eh_require( s->rho_sat   <= s->rho_grain        );
   eh_require( s->void_min  >= 0.                  );
   eh_require( s->diff_coef >= 0.                  );
   eh_require( s->diff_coef <= 1.                  );

   return s;
}

gboolean sed_type_is_same( Sed_type t_1 , Sed_type t_2 )
{
   gboolean same = TRUE;

   eh_require( t_1 );
   eh_require( t_2 );

   if ( t_1!=t_2 )
   {
      same =  fabs( t_1->rho_grain - t_2->rho_grain ) < 1e-12
           && fabs( t_1->rho_sat   - t_2->rho_sat   ) < 1e-12
           && fabs( t_1->gz        - t_2->gz        ) < 1e-12
           && fabs( t_1->pi        - t_2->pi        ) < 1e-12
           && fabs( t_1->void_min  - t_2->void_min  ) < 1e-12
           && fabs( t_1->diff_coef - t_2->diff_coef ) < 1e-12
           && fabs( t_1->lambda    - t_2->lambda    ) < 1e-12
           && fabs( t_1->c_v       - t_2->c_v       ) < 1e-12
           && fabs( t_1->c         - t_2->c         ) < 1e-12
           && fabs( t_1->w_s       - t_2->w_s       ) < 1e-12;
   }

   return same;
}

double __gravity         = S_GRAVITY;
double __rho_sea_water   = S_RHO_SEA_WATER;
double __rho_fresh_water = S_RHO_FRESH_WATER;
double __salinity_sea    = S_SALINITY_SEA;
double __rho_grain       = S_RHO_GRAIN;
double __rho_mantle      = S_RHO_MANTLE;

double sed_gravity( )
{
   extern double __gravity;
   return __gravity;
}
double sed_gravity_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return __gravity;
      case UNITS_CGS:
         return __gravity*100.;
      case UNITS_IMPERIAL:
         return __gravity*3.2808399;
      default:
         eh_require_not_reached();
         return __gravity;
   }
}
double sed_set_gravity( double new_val )
{
   extern double __gravity;
   return __gravity = new_val;
}
double sed_rho_sea_water()
{
   extern double __rho_sea_water;
   return __rho_sea_water;
}
double sed_rho_sea_water_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return sed_rho_sea_water();
      case UNITS_CGS:
         return sed_rho_sea_water()*.001;
      case UNITS_IMPERIAL:
         return sed_rho_sea_water()*0.062428;
      default:
         eh_require_not_reached();
         return sed_rho_sea_water();
   }
}
double sed_set_rho_sea_water( double new_val )
{
   extern double __rho_sea_water;
   return __rho_sea_water = new_val;
}
double sed_rho_fresh_water()
{
   extern double __rho_fresh_water;
   return __rho_fresh_water;
}
double sed_rho_fresh_water_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return sed_rho_fresh_water();
      case UNITS_CGS:
         return sed_rho_fresh_water()*.001;
      case UNITS_IMPERIAL:
         return sed_rho_fresh_water()*0.062428;
      default:
         eh_require_not_reached();
         return sed_rho_fresh_water();
   }
}
double sed_set_rho_fresh_water( double new_val )
{
   extern double __rho_fresh_water;
   return __rho_fresh_water = new_val;
}
double sed_sea_salinity()
{
   extern double __salinity_sea;
   return __salinity_sea;
}
double sed_sea_salinity_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return sed_sea_salinity()*.001;
      case UNITS_CGS:
         return sed_sea_salinity()*.001;
      case UNITS_IMPERIAL:
         return sed_sea_salinity()*.001;
      default:
         eh_require_not_reached();
         return sed_sea_salinity()*.001;
   }
}
double sed_set_sea_salinity( double new_val )
{
   extern double __salinity_sea;
   return __salinity_sea = new_val;
}

double sed_rho_quartz()
{
   extern double __rho_grain;
   return __rho_grain;
}

double sed_rho_quartz_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return sed_rho_quartz();
      case UNITS_CGS:
         return sed_rho_quartz()*.001;
      case UNITS_IMPERIAL:
         return sed_rho_quartz()*0.062428;
      default:
         eh_require_not_reached();
         return sed_rho_quartz();
   }
}

double sed_set_rho_quartz( double new_val )
{
   extern double __rho_grain;
   return __rho_grain = new_val;
}

double sed_rho_mantle()
{
   extern double __rho_mantle;
   return __rho_mantle;
}

double sed_rho_mantle_units( Sed_units units )
{
   switch ( units )
   {
      case UNITS_MKS:
         return sed_rho_mantle();
      case UNITS_CGS:
         return sed_rho_mantle()*.001;
      case UNITS_IMPERIAL:
         return sed_rho_mantle()*0.062428;
      default:
         eh_require_not_reached();
         return sed_rho_mantle();
   }
}

double sed_set_rho_mantle( double new_val )
{
   extern double __rho_mantle;
   return __rho_mantle = new_val;
}

Sed_type sed_type_set_rho_sat( Sed_type t , double rho_sat )
{
   t->rho_sat = rho_sat;
   return t;
}

Sed_type sed_type_set_rho_grain( Sed_type t , double rho_grain )
{
   t->rho_grain = rho_grain;
   return t;
}

Sed_type sed_type_set_grain_size( Sed_type t , double gz )
{
   t->gz = gz;
   return t;
}

Sed_type sed_type_set_plastic_index( Sed_type t , double pi )
{
   t->pi = pi;
   return t;
}

Sed_type sed_type_set_void_ratio_min( Sed_type t , double void_min )
{
   t->void_min = void_min;
   return t;
}

Sed_type sed_type_set_diff_coef( Sed_type t , double k )
{
   t->diff_coef = k;
   return t;
}

Sed_type sed_type_set_lambda( Sed_type t , double l )
{
   t->lambda = l;
   t->w_s    = sed_removal_rate_to_settling_velocity( t->lambda );
   return t;
}

Sed_type sed_type_set_settling_velocity( Sed_type t , double w )
{
   t->w_s    = w;
   t->lambda = sed_settling_velocity_to_removal_rate( t->w_s );
   return t;
}

Sed_type sed_type_set_c_consolidation( Sed_type t , double c_v )
{
   t->c_v = c_v;
   return t;
}

Sed_type sed_type_set_compressibility( Sed_type t , double c )
{
   t->c = c;
   return t;
}

double sed_type_rho_sat( const Sed_type t )
{
   return t->rho_sat;
}

double sed_type_rho_grain( const Sed_type t )
{
   return t->rho_grain;
}

double sed_type_grain_size( const Sed_type t )
{
   return t->gz;
}

double sed_type_plastic_index( const Sed_type t )
{
   return t->pi;
}

double sed_type_void_ratio_min(const Sed_type t )
{
   return t->void_min;
}

double sed_type_diff_coef(const Sed_type t )
{
   return t->diff_coef;
}

double sed_type_lambda( const Sed_type t )
{
   return t->lambda;
}

double sed_type_lambda_in_per_seconds( const Sed_type t )
{
   return t->lambda*S_DAYS_PER_SECOND;
}

/** Settling velocity of a grain class.

Return the settling velocity for a grain class.  The settling velocity is
converted from removal rates.  The conversion comes from Bursik (1995).

Bursik, M.I., 1995.  Theory of the sedimentation of suspended particles from
fluvial plumes.  Sedimentology, v. 42, pp. 831-838.

@param s A sedflux sediment structure.
@param n The index of the grain class.

@return The settling velocity of the grain class in m/day.
*/
#define SED_BURSIK_CONST_A_3 (1.74)
#define SED_BURSIK_CONST_H   (7.5)
double sed_type_settling_velocity( const Sed_type t )
{
   return t->w_s;
}

double sed_removal_rate_to_settling_velocity( double l )
{
   return l * SED_BURSIK_CONST_A_3 * SED_BURSIK_CONST_H;
}

double sed_settling_velocity_to_removal_rate( double w_s )
{
   return w_s / ( SED_BURSIK_CONST_A_3 * SED_BURSIK_CONST_H );
}

double sed_type_c_consolidation( const Sed_type t )
{
   return t->c_v;
}

double sed_type_compressibility( const Sed_type t )
{
   return t->c;
}

double sed_type_density_0( const Sed_type t )
{
/*
   double e = sed_type_void_ratio( t );
   return ( sed_type_rho_grain(t) + e*sed_rho_sea_water() ) / (1+e);
*/
   return sed_type_rho_sat(t);
}

double sed_type_rho_max( const Sed_type t )
{
   double e = sed_type_void_ratio_min(t);
   return (sed_type_rho_grain(t) + e*sed_rho_sea_water())/(e+1.);
}

double sed_type_grain_size_in_meters( const Sed_type t )
{
   return sed_type_grain_size(t)*1e-6;
}

double sed_type_inv_grain_size_in_meters( const Sed_type t )
{
   return 1./( sed_type_grain_size(t)*1e-6 );
}

double sed_type_grain_size_in_phi( const Sed_type t )
{
   return -log2(t->gz/1000.);
}

double sed_type_is_sand( const Sed_type t )
{
   if ( (Sed_size_class)
           (S_SED_TYPE_SAND & sed_type_size_class( t ) ) )
      return TRUE;
   else
      return FALSE;
}

double sed_type_is_silt( const Sed_type t )
{
   if ( (Sed_size_class)
           (S_SED_TYPE_SILT & sed_type_size_class( t ) ) )
      return TRUE;
   else
      return FALSE;
}

double sed_type_is_clay( const Sed_type t )
{
   if ( (Sed_size_class)
           (S_SED_TYPE_CLAY & sed_type_size_class( t ) ) )
      return TRUE;
   else
      return FALSE;
}

double sed_type_is_mud( const Sed_type t )
{
   if ( (Sed_size_class)
           (S_SED_TYPE_MUD & sed_type_size_class( t )) )
      return TRUE;
   else
      return FALSE;
}

Sed_size_class sed_type_grain_size_in_wentworth( const Sed_type t )
{
   return sed_size_class( sed_type_grain_size_in_phi( t ) );
}

Sed_size_class sed_type_size_class( const Sed_type t )
{
   return sed_size_class( sed_type_grain_size_in_phi( t ) );
}

double sed_type_velocity( const Sed_type t )
{
   double e = sed_type_void_ratio( t );
   return ( e*S_VELOCITY_IN_WATER + S_VELOCITY_IN_ROCK)/(e+1);
}

double sed_type_viscosity( const Sed_type t )
{
   double mu = 0;
   double r  = sed_type_void_ratio( t ) / sed_type_void_ratio_min( t );

   if ( r > .8 )
      mu = S_ETA_WATER * 25;
   else
      mu = S_ETA_WATER * pow( 1-r , -2 );

   return mu;
}

double sed_type_dynamic_viscosity( const Sed_type t )
{
   double            c = 1. - sed_type_porosity( t );
   Sed_size_class size = sed_type_size_class( t );
   double a;

   switch ( size )
   {
      case S_SED_TYPE_SAND:
         a = 10.;
         break;
      default:
         a = 23;
   }

   return S_MU_WATER*( 1. + 2.5*c + exp( a*(c-.05) ) );
}

double sed_type_relative_density( const Sed_type t )
{
   double e     = sed_type_void_ratio    ( t );
   double e_min = sed_type_void_ratio_min( t );
   double e_max = sed_type_void_ratio_max( t );

   return ( e_max-e ) / ( e_max-e_min );
}

double sed_type_void_ratio( Sed_type t )
{
   double rho_sat   = sed_type_rho_sat  ( t );
   double rho_grain = sed_type_rho_grain( t );

   return (rho_grain - rho_sat)/(rho_sat - sed_rho_sea_water());
}

double sed_type_void_ratio_max( Sed_type t )
{
   double p_max = sed_type_porosity_max( t );
   return p_max / ( 1. - p_max );
}

double sed_type_porosity( const Sed_type t )
{
   double e = sed_type_void_ratio( t );
   return e / (1+e);
}

double sed_type_porosity_min( const Sed_type t )
{
   double e_min = sed_type_void_ratio_min(t);
   return e_min / ( 1 + e_min );
}

double sed_type_porosity_max(const Sed_type t )
{
   return sed_type_porosity(t);
}

#define S_F (1.25)

double sed_type_permeability( const Sed_type t )
{
   double e = sed_type_void_ratio          ( t );
   double d = sed_type_grain_size_in_meters( t );
   double s = 6./d; // An assumed shape factor

   return ( 1./(5.*S_F*s*s) )*(pow(e,3)/(1+e));
}

double sed_type_hydraulic_conductivity( const Sed_type t )
{
   return sed_type_permeability( t ) * S_GAMMA_WATER / S_MU_WATER;
}

double sed_type_water_content( const Sed_type t )
{
   double e         = sed_type_void_ratio( t );
   double rho_grain = sed_type_rho_grain ( t );
   return e*(sed_rho_sea_water()/rho_grain);
}

double sed_specific_gravity( const Sed_type t )
{
   return sed_type_rho_grain(t)/sed_rho_sea_water();
}

/*
double sed_type_friction_angle( const Sed_sediment t )
{
   double phi        = 36.;
   double grain_size = sed_type_grain_size( t );

   if ( grain_size >= 2e-3 )
      phi -= 11;
   else if ( grain_size >= .6e-3 )
      phi -= 9;
   else if ( grain_size >= .2e-3 )
      phi -= 4;

   return phi;
}
*/

double sed_type_friction_angle( const Sed_type t )
{
   double phi        = 36.;
   double grain_size = sed_type_grain_size_in_meters( t );
   double rho_rel    = sed_type_relative_density    ( t );

   if ( grain_size >= 2e-3 )
      phi -= 11;
   else if ( grain_size >= .6e-3 )
      phi -= 9;
   else if ( grain_size >= .2e-3 )
      phi -= 4;

   if ( rho_rel <= .5 )
      phi -= 1;
   else if ( rho_rel > .75 )
      phi += 4;

   return phi;
}

double sed_type_yield_strength( const Sed_type t )
{
   double a;
   Sed_size_class size = sed_type_size_class( t );
   double            c = 1. - sed_type_porosity( t );

   switch ( size )
   {
      case S_SED_TYPE_SAND:
         a = 3.;
         break;
      case S_SED_TYPE_SILT:
         a = 13.;
         break;
      case S_SED_TYPE_CLAY:
         a = 23.;
         break;
      default:
         eh_require_not_reached();
   }

   return .1*exp( a *( c - .05 ) );
}

double sed_type_mv( const Sed_type t )
{
   return sed_type_compressibility( t );
}

double sed_type_cv( const Sed_type t )
{
   double mv = sed_type_compressibility( t );
   return sed_type_hydraulic_conductivity(t) / ( S_GAMMA_WATER * mv );
}

double sed_type_shear_strength( const Sed_type t , double load )
{
   return load*( .11 + .0037 * sed_type_plastic_index(t) );
}

double sed_type_shear_strength_with_data( const Sed_type t , gpointer data )
{
   return sed_type_shear_strength( t , *((double*)data) );
}

double sed_type_cohesion( const Sed_type t , double load )
{
   double a = .69;
   double m = .6;
   
   return ( a*(1-m)*pow(load*1e-6,m) )*1e6;
// return load * tan(sed_cell_friction_angle(c,sed,n)*M_PI/180.) - sed_cell_shear_strength(c,load,sed,n);
}

double sed_type_cohesion_with_data( const Sed_type t , gpointer data )
{
   return sed_type_cohesion( t , *((double*)data) );
}

double sed_type_consolidation( const Sed_type t , double d , double dt )
{
   double c_v = sed_type_c_consolidation( t );

   return sed_calculate_avg_consolidation( c_v , d , dt );
}

double sed_type_consolidation_with_data( const Sed_type t , gpointer data )
{
   return sed_type_consolidation_rate( t , ((double*)data)[0] , ((double*)data)[1] );
}

double sed_type_consolidation_rate( const Sed_type t , double d , double dt )
{
   double u;
   double c_v = sed_type_c_consolidation( t );
   double t_v = c_v*dt/(d*d);

   if ( t_v < .2827 )
      u = .5/sqrt(4/G_PI*dt);
   else
      u = 2*exp( -G_PI*G_PI*.25*t_v );

   return u;
}

double sed_type_consolidation_rate_with_data( const Sed_type t , gpointer data )
{
   return sed_type_consolidation_rate( t , ((double*)data)[0] , ((double*)data)[1] );
}

double sed_calculate_avg_consolidation( double c_v , double d , double t )
{
   double t_v = c_v*t/(d*d);
   if ( d <= 0 )
      return 1.;
   if ( t_v < .2827 )
      return ( sqrt(4./G_PI*t_v) );
   else
      return ( 1.-8./(G_PI*G_PI)*exp(-G_PI*G_PI/4.*t_v) );
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

double sed_type_is_size_class( Sed_type t , Sed_size_class size )
{
   return size & sed_type_size_class( t );
}

double sed_type_is_size_class_with_data( Sed_type t , gpointer size )
{
   return sed_type_is_size_class( t , *((Sed_size_class*)size) );
}

double sed_type_sum_size_classes_with_data( Sed_type t , gpointer size )
{
   Sed_size_class* c = (Sed_size_class*)size;

   *c |= sed_type_size_class( t );

   return 1.0;
}

double sed_type_void_ratio_compacted( Sed_type t , gpointer d )
{
   return (*(double*)d)*(1+sed_type_void_ratio(t)) - 1;
}

double sed_type_density_compacted( Sed_type t , gpointer d )
{
   double e = sed_type_void_ratio_compacted(t,d);
   return ( sed_type_rho_grain(t) + e*sed_rho_sea_water() ) / (1.+e);
}

/** \brief Return the Wentworth size class from a grain size.

Returns the Wentworth size class for a grain size in phi units.

\param phi    The grain size is phi units.

\return       The grain size as a Wentworth size class.

\see sed_cell_size_class , Sed_size_class .
*/
Sed_size_class sed_size_class( const double phi )
{
   if ( phi <= S_SED_TYPE_VERY_FINE_SAND_PHI )
      return S_SED_TYPE_SAND;
   else if ( phi <= S_SED_TYPE_VERY_FINE_SILT_PHI )
      return S_SED_TYPE_SILT;
   else
      return S_SED_TYPE_CLAY;
}

