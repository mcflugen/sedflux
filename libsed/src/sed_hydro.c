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
#include "sed_hydro.h"
#include "sed_hydrotrend.h"

CLASS ( Sed_hydro )
{
   double velocity;
   double width;
   double depth;
   double bedload;
   double *conc;
   gint32 n_grains;
   double duration;
   double t;
};

CLASS ( Sed_hydro_file )
{
   FILE *fp;
   gchar* file;
   Sed_hydro_file_type type;
   gboolean wrap_is_on;
   Sed_hydro *buf_set;
   Sed_hydro *buf_cur;
   int buffer_len;
   int n_sig_values;
   size_t header_start;
   size_t data_start;
   size_t data_size;
   Sed_hydrotrend_header *hdr;
   Hydro_read_record_func read_record;
   Hydro_read_header_func read_hdr;
};

GQuark
sed_hydro_error_quark( void )
{
   return g_quark_from_static_string( "sed-hydro-error-quark" );
}


//Hydro_header* _hydro_read_inline_header           ( Sed_hydro_file fp );
static Sed_hydro              _hydro_read_inline_record           ( Sed_hydro_file fp );
static Sed_hydrotrend_header* _hydro_read_hydrotrend_header       ( Sed_hydro_file fp );
static Sed_hydro              _hydro_read_hydrotrend_record       ( Sed_hydro_file fp );
static Sed_hydro              _hydro_read_hydrotrend_record_buffer( Sed_hydro_file fp );

void
sed_hydro_fprint_default_inline_file( FILE *fp )
{
   char *text[]=DEFAULT_HYDRO_INLINE_FILE;
   char **p;
   for ( p = text ; *p ; p++ )
      fprintf( fp , "%s\n" , *p );
}

gssize
sed_hydro_array_fprint( FILE* fp , Sed_hydro* rec_a )
{
   gssize n = 0;

   eh_return_val_if_fail( rec_a    , 0 );
   eh_return_val_if_fail( rec_a[0] , 0 );

   if ( fp && rec_a )
   {
      Sed_hydro* rec;
      gint       n_rec = 0;

      for ( rec=rec_a ; *rec ; rec++ )
      {
         n += fprintf( fp , "[ Record %d ]\n" , n_rec++ );
         n += sed_hydro_fprint( fp , *rec );
      }
   }

   return n;
}

gssize
sed_hydro_fprint( FILE* fp , Sed_hydro rec )
{
   gssize n = 0;

   eh_return_val_if_fail( rec , 0 );

   if ( fp && rec )
   {
      gssize i;
      n += fprintf( fp , "duration (day)         : %f\n" , rec->duration );
      n += fprintf( fp , "time                   : %f\n" , rec->t        );
      n += fprintf( fp , "velocity (m/s)         : %f\n" , rec->velocity );
      n += fprintf( fp , "width (m)              : %f\n" , rec->width    );
      n += fprintf( fp , "depth (m)              : %f\n" , rec->depth    );
      n += fprintf( fp , "bedload (kg/s)         : %f\n" , rec->bedload  );
      n += fprintf( fp , "concentration (kg/m^3) : " );
      for ( i=0 ; i<rec->n_grains-1 ; i++ )
         n += fprintf( fp , "%f, " , rec->conc[i] );
      n += fprintf( fp , "%f\n" , rec->conc[rec->n_grains-1] );
   }

   return n;
}
/*
Sed_hydro
sed_hydro_init( char *file )
{
   char *name_used;
   FILE *fp;
   Sed_hydro record;

   if ( !file )
      name_used = g_strdup( SED_HYDRO_TEST_INLINE_FILE );
   else
      name_used = g_strdup( file );

   fp = fopen( name_used , "r" );

   record = sed_hydro_scan( fp );

   fclose( fp );

   eh_free( name_used );

   return record;
}
*/

/** Scan the hydro records from a file.

\param file    The name of the file to scan
\param error   A return location for errors.

\return A NULL-terminated array of Sed_hydro's.
*/
Sed_hydro*
sed_hydro_scan( const gchar* file , GError** error )
{
   return sed_hydro_scan_n_records( file , -1 , error );
}

Sed_hydro*
sed_hydro_scan_n_records( const gchar* file , gint n_recs , GError** error )
{
   Sed_hydro* hydro_arr = NULL;

   eh_require( error==NULL || *error==NULL );

   if ( !file )
      file = SED_HYDRO_TEST_INLINE_FILE;

   if ( n_recs!=0 )
   {
      gchar*          name_used = g_strdup( file );
      GError*         tmp_err   = NULL;
      Eh_key_file     key_file  = NULL;

      key_file  = eh_key_file_scan( name_used , &tmp_err );

      if ( key_file )
      {
         gint            i;
         Eh_symbol_table group;
         const gint      len = eh_key_file_size( key_file );

         if      ( n_recs<0   ) n_recs = len;
         else if ( n_recs>len ) n_recs = len;

         hydro_arr = eh_new0( Sed_hydro , n_recs+1 );

         for ( group = eh_key_file_pop_group( key_file ), i=0 ;
               group && !tmp_err && i<n_recs ;
               group = eh_key_file_pop_group( key_file ), i++ )
         {
            hydro_arr[i] = sed_hydro_new_from_table( group , &tmp_err );

            if ( !tmp_err ) sed_hydro_check( hydro_arr[i] , &tmp_err );

            eh_symbol_table_destroy( group );
         }
         hydro_arr[i] = NULL;

         sed_hydro_array_set_time( hydro_arr , 0. );

         if ( tmp_err )
         {
            g_propagate_error( error , tmp_err );

            for ( i=0 ; hydro_arr[i] ; i++ )
               sed_hydro_destroy( hydro_arr[i] );
            eh_free( hydro_arr );
            hydro_arr = NULL;
         }

      }
      else
         g_propagate_error( error , tmp_err );

      eh_free            ( name_used );
      eh_key_file_destroy( key_file  );
   }

   return hydro_arr;
}



#define SED_HYDRO_LABEL_DURATION        "Duration"
#define SED_HYDRO_LABEL_BEDLOAD         "Bedload"
#define SED_HYDRO_LABEL_SUSPENDED_CONC  "Suspended load concentration"
#define SED_HYDRO_LABEL_VELOCITY        "Velocity"
#define SED_HYDRO_LABEL_WIDTH           "Width"
#define SED_HYDRO_LABEL_DEPTH           "Depth"

static gchar* required_labels[] =
{
   SED_HYDRO_LABEL_DURATION         ,
   SED_HYDRO_LABEL_BEDLOAD          ,
   SED_HYDRO_LABEL_SUSPENDED_CONC   ,
   SED_HYDRO_LABEL_VELOCITY         ,
   SED_HYDRO_LABEL_WIDTH            ,
   SED_HYDRO_LABEL_DEPTH            ,
   NULL 
};

#include <string.h>

/** Create a Sed_hydro, initializing it with a symbol table

\param t        An Eh_symbol_table with the initialization data.
\param error    A return location for errors.

\return A newly-created Sed_hydro.
*/
Sed_hydro
sed_hydro_new_from_table( Eh_symbol_table t , GError** error )
{
   Sed_hydro r = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( t )
   {
      GError* tmp_err = NULL;

      //if ( eh_symbol_table_has_labels( t , required_labels ) )
      if ( eh_symbol_table_require_labels( t , required_labels , &tmp_err ) )
      {
         gint    n_susp_grains;
         double* c = eh_symbol_table_dbl_array_value( t , SED_HYDRO_LABEL_SUSPENDED_CONC , &n_susp_grains , NULL );

         r = sed_hydro_new( n_susp_grains );

         r->conc      = memcpy( r->conc , c , sizeof(double)*n_susp_grains );

         r->duration  = eh_symbol_table_time_value( t , SED_HYDRO_LABEL_DURATION  );
         r->bedload   = eh_symbol_table_dbl_value ( t , SED_HYDRO_LABEL_BEDLOAD   );
         r->velocity  = eh_symbol_table_dbl_value ( t , SED_HYDRO_LABEL_VELOCITY  );
         r->width     = eh_symbol_table_dbl_value ( t , SED_HYDRO_LABEL_WIDTH     );
         r->depth     = eh_symbol_table_dbl_value ( t , SED_HYDRO_LABEL_DEPTH     );
      
         r->duration *= S_DAYS_PER_YEAR;

         r->t         = 0.;

         eh_free( c );
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         r = sed_hydro_destroy( r );
      }
   }

   return r;
}

gboolean
sed_hydro_check( Sed_hydro a , GError** err )
{
   gboolean is_ok = TRUE;

   eh_return_val_if_fail( err==NULL || *err==NULL , FALSE );
   eh_return_val_if_fail( a                       , TRUE  );

   if ( a )
   {
      gchar** err_s = NULL;

      eh_check_to_s( sed_hydro_width   (a)>0.  , "River width > 0"    , &err_s );
      eh_check_to_s( sed_hydro_depth   (a)>0.  , "River depth > 0"    , &err_s );
      eh_check_to_s( sed_hydro_velocity(a)>0.  , "River velocity > 0" , &err_s );
      eh_check_to_s( sed_hydro_bedload (a)>=0. , "Bedload flux > 0"   , &err_s );
      eh_check_to_s( sed_hydro_duration(a)>0.  , "Duration > 0"       , &err_s );
      eh_check_to_s( eh_dbl_array_each_ge( 0.  , a->conc , a->n_grains ) ,
                                                 "Suspended load concentration > 0" , &err_s );

      if ( err_s )
      {
         gchar* str = g_strjoinv( "\n" , err_s );

         is_ok = FALSE;

         g_set_error( err ,
                      SED_HYDRO_ERROR ,
                      SED_HYDRO_ERROR_BAD_PARAMETER ,
                      "%s" , str );

         eh_free( str );
         g_strfreev( err_s );
      }

   }

   return is_ok;
}

const gchar __hydro_unknown_type_s[]       = "Unknown";
const gchar __hydro_inline_type_s[]        = "Inline";
const gchar __hydro_hydrotrend_type_s[]    = "Hydrotrend";
const gchar __hydro_hydrotrend_be_type_s[] = "Hydrotrend (big-endian)";
const gchar __hydro_hydrotrend_le_type_s[] = "Hydrotrend (little-endian)";

const gchar*
sed_hydro_type_to_s( Sed_hydro_file_type t )
{
   const gchar* s;

   switch ( t )
   {
      case SED_HYDRO_INLINE        : s = __hydro_inline_type_s ; break;
      case SED_HYDRO_HYDROTREND    : s = __hydro_hydrotrend_type_s ; break;
      case SED_HYDRO_HYDROTREND_BE : s = __hydro_hydrotrend_be_type_s ; break;
      case SED_HYDRO_HYDROTREND_LE : s = __hydro_hydrotrend_le_type_s ; break;
      default                      : s = __hydro_unknown_type_s;
   }

   return s;
}

Sed_hydro_file_type
sed_hydro_str_to_type( const gchar* type_s )
{
   Sed_hydro_file_type t = SED_HYDRO_UNKNOWN;

   if ( type_s )
   {
      if      (    g_ascii_strcasecmp( type_s , "SEASON" )==0
                || g_ascii_strcasecmp( type_s , "INLINE" )==0 )
         t = SED_HYDRO_INLINE;
      else if (    g_ascii_strcasecmp( type_s , "HYDROTREND" )==0
                || g_ascii_strcasecmp( type_s , "EVENT"      )==0
                || g_ascii_strcasecmp( type_s , "BUFFER"     )==0 )
         t = SED_HYDRO_HYDROTREND;
   }

   return t;
}

Sed_hydro_file_type
sed_hydro_file_guess_type( const gchar* file , GError** error )
{
   Sed_hydro_file_type type = SED_HYDRO_UNKNOWN;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( file )
   {
      GError* tmp_err = NULL;
      FILE*   fp      = eh_fopen_error( file , "r" , &tmp_err );

      if ( !tmp_err )
      {
         gchar      str[1000];
         gboolean   is_ascii = TRUE;
         gint       n        = fread( str , sizeof(gchar) , 1000 , fp );
         Sed_hydro* arr      = NULL;
         gint       i;

         /* Look for binary characters */
         for ( i=0 ; i<n && is_ascii ; i++ )
         {
            if ( !g_ascii_isgraph(str[i]) && !g_ascii_isspace(str[i]) )
               is_ascii = FALSE;
         }

         fclose( fp );

         if ( is_ascii ) /* Try to read it as a key value file */
         {
            arr = sed_hydro_scan_n_records( file , 10 , &tmp_err );
            if ( !tmp_err ) type = SED_HYDRO_INLINE;
         }
         else /* Try to read it as a hydrotrend file */
         {
            arr = sed_hydrotrend_read_n_recs( file , 10 , G_BIG_ENDIAN , NULL , &tmp_err );
            if ( arr ) type = SED_HYDRO_HYDROTREND_BE;
            else
            {
               arr = sed_hydrotrend_read_n_recs( file , 10 , G_LITTLE_ENDIAN , NULL , &tmp_err );
               if ( arr ) type = SED_HYDRO_HYDROTREND_LE;
            }
         }

         sed_hydro_array_destroy( arr );
      }
   }

   return type;
}

/*
Sed_hydro
sed_hydro_scan( FILE *fp )
{
   Sed_hydro river_data;
   int n_grains=9999;
   
   if ( read_int_vector(fp,&n_grains,1)!=1 )
      return NULL;
   
   eh_require( n_grains>=2 );

   river_data = sed_hydro_new( n_grains-1 );
   
   read_time_vector  ( fp , &river_data->duration , 1 );
   read_double_vector( fp , &river_data->bedload  , 1 );
   read_double_vector( fp ,  river_data->conc     , (int)(n_grains-1) );
   read_double_vector( fp , &river_data->velocity , 1 );
   read_double_vector( fp , &river_data->width    , 1 );
   read_double_vector( fp , &river_data->depth    , 1 );

   river_data->duration *= S_DAYS_PER_YEAR;

   return river_data;
}
*/
/*
Hydro_header *sed_hydro_scan_inline_header( FILE *fp )
{
   int where;
   Sed_hydro rec;
   Hydro_header *hdr=eh_new( Hydro_header , 1 );

   where = ftell(fp);
   rec = sed_hydro_scan( fp );
   fseek( fp , where , SEEK_SET );
   
   hdr->comment = NULL;
   hdr->n_grains  = rec->n_grains;
   hdr->n_seasons = -1;
   hdr->n_samples = -1;

   rec = sed_hydro_destroy( rec );

   return hdr;
}
*/

/** Create a new Sed_hydro

\param n_grains The number of suspended grain sizes

\return A newly-created Sed_hydro.  Use sed_hydro_destroy to free.
*/
Sed_hydro
sed_hydro_new( gssize n_grains )
{
   Sed_hydro rec;

   NEW_OBJECT( Sed_hydro , rec );

   rec->conc = eh_new0( double , n_grains );
   rec->duration = 1.;
   rec->t        = 0.;
   rec->n_grains = n_grains;

   rec->velocity = 0.;
   rec->width    = 0.;
   rec->depth    = 0.;
   rec->bedload  = 0.;

   return rec;
}

#include <string.h>

Sed_hydro sed_hydro_copy( Sed_hydro dest , Sed_hydro src )
{
   eh_return_val_if_fail( src , NULL );

   if ( !dest )
      dest = sed_hydro_new( src->n_grains );

   sed_hydro_resize( dest , sed_hydro_size(src) );

   dest->velocity = src->velocity;
   dest->width    = src->width;
   dest->depth    = src->depth;
   dest->bedload  = src->bedload;
   dest->n_grains = src->n_grains;
   dest->duration = src->duration;
   dest->t        = src->t;

   g_memmove( dest->conc , src->conc , sizeof(double)*src->n_grains );

   return dest;
}

Sed_hydro
sed_hydro_dup( Sed_hydro src )
{
   return sed_hydro_copy( NULL , src );
}

Sed_hydro*
sed_hydro_array_dup( Sed_hydro* src )
{
   Sed_hydro* dest = NULL;

   if ( src )
   {
      gint       i;
      gint       len = g_strv_length( src );
      Sed_hydro* p   = src;

      dest = eh_new( Sed_hydro , len+1 );

      for ( i=0 ; *p ; p++,i++ ) dest[i] = sed_hydro_dup( *p );
      dest[len] = NULL;
   }

   return dest;
}

/** Compare two Sed_hydro values

Note that this does not compare the times of the Sed_hydro values.

\param a   A Sed_hydro
\param b   A Sed_hydro

\return TRUE if both values are the same
*/
gboolean
sed_hydro_is_same( Sed_hydro a , Sed_hydro b )
{
   eh_return_val_if_fail( a , FALSE );
   eh_return_val_if_fail( b , FALSE );

   return    a->n_grains == b->n_grains
          && eh_compare_dbl( a->bedload  , b->bedload  , 1e-12 )
          && eh_compare_dbl( a->width    , b->width    , 1e-12 )
          && eh_compare_dbl( a->depth    , b->depth    , 1e-12 )
          && eh_compare_dbl( a->velocity , b->velocity , 1e-12 )
          && eh_compare_dbl( a->duration , b->duration , 1e-12 );

}

Sed_hydro sed_hydro_resize( Sed_hydro a , gssize n )
{
   if ( !a )
      a = sed_hydro_new( n );
   else
   {
      if ( n>0 )
      {
         a->conc = eh_renew( double , a->conc , n );
         a->n_grains = n;
      }
      else
         a = sed_hydro_destroy(a);
   }

   return a;
}

double
sed_hydro_time( Sed_hydro a )
{
   eh_require( a );
   return a->t;
}

gssize
sed_hydro_size( Sed_hydro a )
{
   eh_require( a );
   return a->n_grains;
}

Sed_hydro sed_hydro_destroy( Sed_hydro rec )
{
   if ( rec )
   {
      eh_free(rec->conc);
      eh_free(rec);
   }

   return NULL;
}

Sed_hydro*
sed_hydro_array_destroy( Sed_hydro* arr )
{
   if ( arr )
   {
      Sed_hydro* p;

      for ( p=arr ; *p ; p++ )
         sed_hydro_destroy( *p );
      eh_free( arr );
   }

   return NULL;
}

gssize
sed_hydro_write_to_byte_order( FILE *fp , Sed_hydro a , gint order )
{
   gssize n = 0;

   if ( a && fp )
   {
      gint32 n_grains = a->n_grains;

      if ( order == G_BYTE_ORDER )
      {
         n += fwrite( &(a->velocity) , sizeof(double) , 1 , fp );
         n += fwrite( &(a->width   ) , sizeof(double) , 1 , fp );
         n += fwrite( &(a->depth   ) , sizeof(double) , 1 , fp );
         n += fwrite( &(a->bedload ) , sizeof(double) , 1 , fp );
         n += fwrite( &(n_grains)    , sizeof(gint32) , 1 , fp );
         n += fwrite(   a->conc      , sizeof(double) , a->n_grains , fp );
         n += fwrite( &(a->duration) , sizeof(double) , 1 , fp );
         n += fwrite( &(a->t)        , sizeof(double) , 1 , fp );
      }
      else
      {
         n += eh_fwrite_dbl_swap  ( &(a->velocity) , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(a->width   ) , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(a->depth   ) , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(a->bedload ) , sizeof(double) , 1 , fp );
         n += eh_fwrite_int32_swap( &(n_grains)    , sizeof(gint32) , 1 , fp );
         n += eh_fwrite_dbl_swap  (   a->conc      , sizeof(double) , a->n_grains , fp );
         n += eh_fwrite_dbl_swap  ( &(a->duration) , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(a->t)        , sizeof(double) , 1 , fp );
      }
   }

   return n;
}

gssize
sed_hydro_write( FILE *fp , Sed_hydro a )
{
   return sed_hydro_write_to_byte_order(fp,a,G_BYTE_ORDER);
}

Sed_hydro
sed_hydro_read( FILE *fp )
{
   Sed_hydro a = NULL;

   if ( fp )
   {
      gint32 len;

      a = sed_hydro_new( 1 );

      fread( &(a->velocity) , sizeof(double) , 1   , fp );
      fread( &(a->width)    , sizeof(double) , 1   , fp );
      fread( &(a->depth)    , sizeof(double) , 1   , fp );
      fread( &(a->bedload)  , sizeof(double) , 1   , fp );
      fread( &len           , sizeof(gint32) , 1   , fp );

      sed_hydro_resize( a , len );

      fread( a->conc        , sizeof(double) , len , fp );
      fread( &(a->duration) , sizeof(double) , 1   , fp );
      fread( &(a->t)        , sizeof(double) , 1   , fp );
   }

   return a;
}

/** Copy the sdiment concentration from a Sed_hydro

@param dest A poiter to the location to put the concentration (or NULL)
@param a    A Sed_hydro containing the concentrations to copy.

@return A pointer to the copied data
*/
double* sed_hydro_copy_concentration( double* dest , Sed_hydro a )
{
   if ( !dest )
      dest = eh_new( double , a->n_grains );
   memcpy( dest , a->conc , sizeof(double)*a->n_grains );

   return dest;
}

double sed_hydro_nth_concentration( Sed_hydro a , gssize n )
{
   eh_return_val_if_fail( a             , 0. );
   eh_return_val_if_fail( n>=0          , 0. );
   eh_return_val_if_fail( n<a->n_grains , 0. );

   return a->conc[n];
}

/** The fluid density including suspended sediment

\param a     A Sed_hydro
\param rho   Density of the fluid without sediment

\return The density of the fluid (kg/m^3)    
*/
double
sed_hydro_flow_density( Sed_hydro a , double rho )
{
   return sed_hydro_suspended_concentration( a ) + rho;
}

gboolean
sed_hydro_is_hyperpycnal( Sed_hydro a )
{
   gboolean is_hyper = FALSE;

   eh_require( a );

   if ( sed_hydro_flow_density( a , sed_rho_fresh_water() ) > sed_rho_sea_water() )
      is_hyper = TRUE;
   else
      is_hyper = FALSE;

   return is_hyper;
}

double*
sed_hydro_fraction( Sed_hydro a )
{
   double* f = NULL;

   eh_return_val_if_fail( a , NULL );

   if ( a )
   {
      double total_c = sed_hydro_suspended_concentration(a);

      f = sed_hydro_copy_concentration( NULL , a );
      if ( total_c > 0. )
         eh_dbl_array_mult( f , sed_hydro_size(a) , 1./total_c );
      else
         eh_dbl_array_mult( f , sed_hydro_size(a) , 0. );
   }

   return f;
}

double
sed_hydro_nth_fraction( Sed_hydro a , gssize n )
{
   eh_return_val_if_fail( a             , 0. );
   eh_return_val_if_fail( n>=0          , 0. );
   eh_return_val_if_fail( n<a->n_grains , 0. );

   return a->conc[n] / sed_hydro_suspended_concentration(a);
}

double sed_hydro_suspended_concentration( Sed_hydro a )
{
   double total_conc=0;

   eh_return_val_if_fail( a , 0 );

   {
      gssize n;
      for ( n=0 ; n<a->n_grains ; n++ )
         total_conc += a->conc[n];
   }

   return total_conc;
}

double sed_hydro_suspended_flux( Sed_hydro a )
{
   double total_mass_flux=0;

   eh_return_val_if_fail( a , 0 );

   {
      gssize n;
      for ( n=0 ; n<a->n_grains ; n++ )
         total_mass_flux += a->conc[n];
      total_mass_flux *= a->velocity * a->width * a->depth;
   }

   return total_mass_flux;
}

double sed_hydro_suspended_volume_flux( Sed_hydro a )
{
   double total=0;

   eh_return_val_if_fail( a , 0 );

   {
      gssize n;
      Sed_sediment sed = sed_sediment_env();

      for ( n=0 ; n<a->n_grains ; n++ )
         total += a->conc[n]/sed_type_rho_sat( sed_sediment_type(sed,n+1) );
      total *= a->velocity * a->width * a->depth;
   }

   return total;
}

double sed_hydro_water_flux( Sed_hydro a )
{
   eh_return_val_if_fail( a , 0 );
   return a->velocity * a->width * a->depth;
}

double sed_hydro_suspended_load( Sed_hydro a )
{
   eh_return_val_if_fail( a , 0 );
   return sed_hydro_suspended_flux(a)*sed_hydro_duration_in_seconds(a);
}

double
sed_hydro_total_load( Sed_hydro a )
{
   eh_return_val_if_fail( a , 0 );
   return sed_hydro_suspended_load( a ) + a->bedload*sed_hydro_duration_in_seconds(a);
}

double
sed_hydro_array_suspended_load( Sed_hydro* arr )
{
   double load = 0.;
   if ( arr )
   {
      Sed_hydro* r;
      for ( r=arr ; *r ; r++ )
         load += sed_hydro_suspended_load( *r );
   }
   return load;
}

double
sed_hydro_array_total_load( Sed_hydro* arr )
{
   double load = 0.;
   if ( arr )
   {
      Sed_hydro* r;
      for ( r=arr ; *r ; r++ )
         load += sed_hydro_total_load( *r );
   }
   return load;
}

Sed_hydro sed_hydro_set_nth_concentration( Sed_hydro a , gssize n , double val )
{
   eh_return_val_if_fail( a             , NULL );
   eh_return_val_if_fail( n>=0          , NULL );
   eh_return_val_if_fail( n<a->n_grains , NULL );

   a->conc[n] = val;

   return a;
}

Sed_hydro
sed_hydro_adjust_mass( Sed_hydro a , double f )
{
   if ( a )
   {
      gint n;
      gint len = sed_hydro_size(a);
      eh_lower_bound( f , 0. );
      for ( n=0 ; n<len ; n++ )
         a->conc[n] *= f;
      a->bedload *= f;
   }
   return a;
}

Sed_hydro sed_hydro_set_velocity( Sed_hydro a , double val )
{
   a->velocity = val;
   return a;
}

Sed_hydro sed_hydro_set_width( Sed_hydro a , double val )
{
   a->width = val;
   return a;
}

Sed_hydro sed_hydro_set_depth( Sed_hydro a , double val )
{
   a->depth = val;
   return a;
}

Sed_hydro sed_hydro_set_bedload( Sed_hydro a , double val )
{
   a->bedload = val;
   return a;
}

Sed_hydro
sed_hydro_set_duration( Sed_hydro a , double val )
{
   eh_require( a );
   a->duration = val;
   return a;
}

Sed_hydro
sed_hydro_set_time( Sed_hydro a , double val )
{
   eh_require( a );
   a->t = val;
   return a;
}

Sed_hydro*
sed_hydro_array_set_time( Sed_hydro* arr , double t_0 )
{
   if ( arr )
   {
      Sed_hydro* a = NULL;
      double     t = t_0;

      for ( a=arr ; *a ; a++ )
      {
         sed_hydro_set_time( *a , t );

         t += sed_hydro_duration( *a );
      }
   }
   return arr;
}

double sed_hydro_velocity( Sed_hydro a )
{
   return a->velocity;
}

double sed_hydro_width( Sed_hydro a )
{
   return a->width;
}

double sed_hydro_depth( Sed_hydro a )
{
   return a->depth;
}

double sed_hydro_bedload( Sed_hydro a )
{
   return a->bedload;
}

double sed_hydro_duration( Sed_hydro a )
{
   return a->duration;
}

double sed_hydro_duration_in_seconds( Sed_hydro a )
{
   return sed_hydro_duration(a)*S_SECONDS_PER_DAY;
}

/** Add a cell of sediment to river discharge

Add sediment to river discharge.

@param a             A Sed_hydro discharge record
@param s             A Sed_cell to add
@param volume_in_m3  Volume (in m^3) of the Sed_cell

@return The input Sed_hydro record
*/
Sed_hydro
sed_hydro_add_cell( Sed_hydro a         ,
                    const Sed_cell s    )
{
   eh_return_val_if_fail( a , NULL );
   eh_return_val_if_fail( s , a    );

   eh_require( (sed_hydro_size(a)+1)==sed_cell_n_types(s) );

   if ( sed_cell_size(s)>0 )
   {
      double       q;
      double       volume_in_m3 = sed_cell_size( s );
      Sed_sediment sed          = sed_sediment_env();
      double       dt_in_secs   = sed_hydro_duration_in_seconds(a);

      // calculate the water flux.
      q = a->velocity*a->width*a->depth;

      eh_require( q>0 );

      // calculate the bedload flux to add.  remember that the flux
      // is measured in kg/s.
      a->bedload += sed_cell_fraction(s,0)
                  * volume_in_m3
                  * sed_type_rho_sat( sed_sediment_type(sed,0) ) / dt_in_secs;

      // calculate the suspended load flux to add.  remember that
      // the concentrations are in kg/m^3.
      {
         gssize i;
         gssize len = sed_sediment_n_types(sed)-1;
         for ( i=0 ; i<len ; i++ )
            a->conc[i] += sed_cell_fraction(s,i+1)
                        * volume_in_m3
                        * sed_type_rho_sat( sed_sediment_type(sed , i+1) )
                        / ( q*dt_in_secs );
      }
   }

   return a;
}

Sed_hydro sed_hydro_subtract_cell( Sed_hydro a         ,
                                   const Sed_cell s    )
{

   eh_return_val_if_fail( a , NULL );
   eh_return_val_if_fail( s , a    );

   eh_require( (sed_hydro_size(a)+1)==sed_cell_n_types(s) );

   if ( sed_cell_size(s)>0 )
   {
      double       q;
      double       volume_in_m3 = sed_cell_size( s );
      Sed_sediment sed          = sed_sediment_env();
      double       dt_in_secs   = sed_hydro_duration_in_seconds(a);

      // calculate the water flux.
      q = a->velocity*a->width*a->depth;

      eh_require( q>0 );

      // calculate the bedload flux to subtract.  remember that the flux
      // is measured in kg/s.
      a->bedload -= sed_cell_nth_fraction(s,0)
                  * volume_in_m3
                  * sed_type_rho_sat( sed_sediment_type(sed,0) ) / dt_in_secs;
      eh_lower_bound( a->bedload , 0 );

      // calculate the suspended load flux to subtract.  remember that
      // the concentrations are in kg/m^3.
      {
         gssize i;
         gssize len = sed_sediment_n_types(sed)-1;
         for ( i=0 ; i<len ; i++ )
         {
            a->conc[i] -= sed_cell_nth_fraction(s,i+1)
                        * volume_in_m3
                        * sed_type_rho_sat( sed_sediment_type(sed,i+1) )
                        / ( q*dt_in_secs );
            eh_lower_bound( a->conc[i] , 0 );
         }
      }
   }

   return a;
}

Sed_hydro sed_hydro_average_records( Sed_hydro* rec , gssize n_recs )
{
   Sed_hydro mean_rec;

   eh_return_val_if_fail( rec      , NULL                  );
   eh_return_val_if_fail( rec[0]   , NULL                  );
   eh_return_val_if_fail( n_recs>0 , NULL                  );
   eh_return_val_if_fail( n_recs>1 , sed_hydro_dup(rec[0]) );

   mean_rec = sed_hydro_new( rec[0]->n_grains );

   {
      gssize i, j;
      double total_q=0;
      double *total_load;

      total_load = eh_new0( double , rec[0]->n_grains );

      mean_rec->duration = 0;
      for ( i=0 ; i<n_recs ; i++ )
      {
         mean_rec->velocity += rec[i]->velocity;
         mean_rec->width    += rec[i]->width;
         mean_rec->depth    += rec[i]->depth;
         mean_rec->bedload  += rec[i]->bedload;
         mean_rec->duration += rec[i]->duration;
         for ( j=0 ; j<mean_rec->n_grains; j++ )
         {
            mean_rec->conc[j] += rec[i]->conc[j];
            total_load[j]     += rec[i]->conc[j]*sed_hydro_water_flux(rec[i]);
         }
         total_q += sed_hydro_water_flux( rec[i] )*rec[i]->duration;
      }
      mean_rec->velocity /= (double)n_recs;
      mean_rec->width    /= (double)n_recs;
      mean_rec->depth    /= (double)n_recs;
      mean_rec->bedload  /= (double)n_recs;
      for ( j=0 ; j<mean_rec->n_grains ; j++ )
         mean_rec->conc[j] /= (double)n_recs;

      // adjust the mean width to conserve the amount of water.
      mean_rec->width  = total_q/mean_rec->duration/(mean_rec->velocity*mean_rec->depth);

      // adjust the concentrations to conserve the amount of each sediment type.
      for ( j=0 ; j<mean_rec->n_grains ; j++ )
         mean_rec->conc[j] = total_load[j]/total_q;

      eh_free( total_load );
   }

   return mean_rec;
}

double sed_hydro_sum_durations( Sed_hydro* rec , gssize n_recs )
{
   double duration=0;

   eh_return_val_if_fail( rec      , 0 );
   eh_return_val_if_fail( n_recs>0 , 0 );

   {
      gssize i;
      for ( i=0 ; i<n_recs ; i++ )
         duration += rec[i]->duration;
   }

   return duration;
}

typedef struct
{
   double val;
   int    ind;
}
Hydro_sort_st;

gint cmp_hydro_sort_vals( Hydro_sort_st *a , Hydro_sort_st *b ) G_GNUC_INTERNAL;
gint cmp_hydro_sort_inds( Hydro_sort_st *a , Hydro_sort_st *b ) G_GNUC_INTERNAL;

/** Reduce a Sed_hydro array to a number of events

\param rec_a      A NULL-terminated Sed_hydro array
\param n_events   Number of events to retain

\return A newly-allocated, NULL-terminated Sed_hydro array
*/
Sed_hydro*
sed_hydro_array_eventize_number( Sed_hydro* rec_a , gssize n_events )
{
   Sed_hydro* event_a = NULL;

   if ( rec_a && n_events>=0 )
      event_a = sed_hydro_process_records( rec_a , g_strv_length( (gchar**)rec_a ) , n_events , TRUE );

   return event_a;
}

/** Reduce a Sed_hydro array to a number of events based on a fraction of the total events

\param rec_a      A NULL-terminated Sed_hydro array
\param fraction   Fraction of events to retain

\return A newly-allocated, NULL-terminated Sed_hydro array
*/
Sed_hydro*
sed_hydro_array_eventize_fraction( Sed_hydro* rec_a , double fraction )
{
   Sed_hydro* event_a = NULL;

   if ( rec_a && rec_a[0] && fraction>0 && fraction<=1 )
   {
      gssize len      = g_strv_length( (gchar**)rec_a);
      double dummy;
      gssize n_events = fraction*len;
      double fraction = modf( fraction*len , &dummy );

      if ( g_random_double_range( 0 , 1 ) < fraction )
         n_events++;
      
      event_a = sed_hydro_process_records( rec_a , len , n_events , TRUE );
   }

   return event_a;
}

/** Reduce a Sed_hydro array to a number of events based on fraction of total sediment load

\param rec_a              A NULL-terminated Sed_hydro array
\param f                  Fraction of total sediment load to retain
\param insert_mean_values If TRUE, mean values are inserted between events

\return A newly-allocated, NULL-terminated Sed_hydro array
*/
Sed_hydro*
sed_hydro_array_eventize( Sed_hydro* rec_a , double f , gboolean insert_mean_values )
{
   Sed_hydro* event_a = NULL;

   if ( rec_a && rec_a[0] && f>0 && f<=1 )
   {
      gssize     n_events  = 0;
      double     load      = sed_hydro_array_suspended_load( rec_a );
      double     threshold = load*f;
      Sed_hydro* sorted_a  = sed_hydro_array_sort_by_suspended_load( rec_a );
      Sed_hydro* r;
      double     sum;

      for ( r=sorted_a,sum=0. ; *r && sum<threshold ; r++ )
      {
         sum += sed_hydro_suspended_load( *r );
         n_events++;
      }

      eh_free( sorted_a );

      event_a = sed_hydro_process_records( rec_a                         ,
                                           g_strv_length((gchar**)rec_a) ,
                                           n_events                      ,
                                           insert_mean_values );
   }
   else if ( eh_compare_dbl( f , 1. , 1e-12 ) )
      event_a = sed_hydro_array_dup( rec_a );

   return event_a;
}

gint
sed_hydro_cmp_suspended_load( Sed_hydro* a , Sed_hydro* b )
{
   gint cmp = 0;

   eh_require( a && *a );
   eh_require( b && *b );

   if ( *a && *b )
   {
      double a_load = sed_hydro_suspended_load( *a );
      double b_load = sed_hydro_suspended_load( *b );

      if      ( a_load < b_load )
         cmp = +1;
      else if ( a_load > b_load )
         cmp = -1;
      else
         cmp =  0;
   }

   return cmp;
}

gint
sed_hydro_cmp_total_load( Sed_hydro* a , Sed_hydro* b )
{
   gint cmp = 0;

   eh_require( a && *a );
   eh_require( b && *b );

   if ( *a && *b )
   {
      double a_load = sed_hydro_total_load( *a );
      double b_load = sed_hydro_total_load( *b );

      if      ( a_load < b_load )
         cmp = +1;
      else if ( a_load > b_load )
         cmp = -1;
      else
         cmp =  0;
   }

   return cmp;
}

gint
sed_hydro_cmp_time( Sed_hydro *a , Sed_hydro *b )
{
   gint cmp = 0;

   eh_require( a && *a );
   eh_require( b && *b );

   if ( *a && *b )
   {
      double a_t = sed_hydro_time( *a );
      double b_t = sed_hydro_time( *b );

      if      ( a_t < b_t )
         cmp = -1;
      else if ( a_t > b_t )
         cmp = +1;
      else
         cmp =  0;
   }

   return cmp;
}

Sed_hydro*
sed_hydro_array_sort( Sed_hydro* rec_a , GCompareFunc f )
{
   Sed_hydro* sorted_a = NULL;

   if ( rec_a )
   {
      gint       len           = g_strv_length( (gchar**)rec_a );
      GPtrArray* sorted_parray = g_ptr_array_sized_new( len+1 );
      Sed_hydro* r;

      for ( r=rec_a ; *r ; r++ )
      {
         g_ptr_array_add( sorted_parray , *r );
      }

      g_ptr_array_sort( sorted_parray , f    );
      g_ptr_array_add ( sorted_parray , NULL );

      sorted_a = (Sed_hydro*)g_ptr_array_free( sorted_parray , FALSE );
   }

   return sorted_a;
}

Sed_hydro*
sed_hydro_array_sort_by_time( Sed_hydro* arr )
{
   return sed_hydro_array_sort( arr , (GCompareFunc)sed_hydro_cmp_time );
}

Sed_hydro*
sed_hydro_array_sort_by_suspended_load( Sed_hydro* arr )
{
   return sed_hydro_array_sort( arr , (GCompareFunc)sed_hydro_cmp_suspended_load );
}

Sed_hydro*
sed_hydro_array_sort_by_total_load( Sed_hydro* arr )
{
   return sed_hydro_array_sort( arr , (GCompareFunc)sed_hydro_cmp_total_load );
}

Sed_hydro*
sed_hydro_process_records( Sed_hydro* rec      ,
                           gssize n_recs       ,
                           gssize n_sig_values ,
                           gboolean insert_mean_values )
{
   Sed_hydro* return_ptr;

   eh_return_val_if_fail( rec      , NULL   );
   eh_return_val_if_fail( rec[0]   , NULL   );

   {
      GSList*    top_n   = NULL;
      GPtrArray* new_rec = NULL;

      // Create a list to record the top n events
      {
         Hydro_sort_st* new_val;
         gint           i;

         for ( i=0 ; i<n_sig_values ; i++ )
         {
            new_val      = eh_new( Hydro_sort_st , 1 );
            new_val->val = G_MINDOUBLE;
            new_val->ind = -1;

            top_n        = g_slist_append( top_n , new_val );
         }
      }

      // Insert to top n events into the list
      if ( n_sig_values!=0 )
      {
         Hydro_sort_st* lowest_val;
         Hydro_sort_st* new_val;
         gint           i;
         double         val;

         lowest_val = (Hydro_sort_st*)g_slist_nth_data( top_n , 0 );
         for ( i=0 ; i<n_recs ; i++ )
         {
            val = sed_hydro_suspended_flux(rec[i]);
            if ( val>lowest_val->val )
            {
               new_val = eh_new( Hydro_sort_st , 1 );
               new_val->val = val;
               new_val->ind = i;
               top_n = g_slist_insert_sorted( top_n ,
                                              new_val ,
                                              (GCompareFunc)cmp_hydro_sort_vals );
               top_n = g_slist_remove( top_n , lowest_val );
               eh_free( lowest_val );
               lowest_val = (Hydro_sort_st*)g_slist_nth_data( top_n , 0 );
            }
         }
      }

      // sort these records by time (or index)
      top_n = g_slist_sort( top_n , (GCompareFunc)cmp_hydro_sort_inds );

      new_rec = g_ptr_array_new();

      // Create the new list of records with mean values put in between the
      // largest events.
      {
         gint      j;
         gint      ind;
         gint      ind_last;
         Sed_hydro mean_rec;
         Sed_hydro real_rec;

         for ( j=0,ind=0,ind_last=-1 ; j<n_sig_values ; j++,ind_last=ind )
         {
            ind = ((Hydro_sort_st*)g_slist_nth_data( top_n , j ))->ind;

            if ( insert_mean_values && ind != ind_last+1 )
            {
               mean_rec = sed_hydro_average_records( &(rec[ind_last+1]) , ind-ind_last-1 );

               // Add the mean record.
               g_ptr_array_add( new_rec , mean_rec );
            }

            real_rec = sed_hydro_dup( rec[ind] );

            // Add the real record.
            g_ptr_array_add( new_rec , real_rec );
         }

         if ( insert_mean_values && ind<n_recs-1 )
         {
            ind = n_recs;
            mean_rec = sed_hydro_average_records( &(rec[ind_last+1]) , ind-ind_last-1 );

            // Add the mean record.
            g_ptr_array_add( new_rec , mean_rec );
         }

         // Create a NULL-terminated array
         g_ptr_array_add( new_rec , NULL );
      }

      return_ptr = (Sed_hydro*)g_ptr_array_free( new_rec , FALSE );

      g_slist_foreach( top_n , &eh_free_slist_data , NULL );
      g_slist_free   ( top_n );
   }

   return return_ptr;
}

gint cmp_hydro_sort_vals( Hydro_sort_st *a , Hydro_sort_st *b )
{
   if ( a->val < b->val )
      return -1;
   else if ( a->val > b->val )
      return 1;
   else
      return 0;
}

gint cmp_hydro_sort_inds( Hydro_sort_st *a , Hydro_sort_st *b )
{
   if ( a->ind < b->ind )
      return -1;
   else if ( a->ind > b->ind )
      return 1;
   else
      return 0;
}

Sed_hydro_file sed_hydro_file_set_wrap( Sed_hydro_file fp , gboolean wrap_is_on )
{
   fp->wrap_is_on = wrap_is_on;
   return fp;
}

Sed_hydro_file sed_hydro_file_set_buffer_length( Sed_hydro_file fp , gssize len )
{
   int whence = fp->buf_cur - fp->buf_set;

   fp->buf_set      = g_renew( Sed_hydro , fp->buf_set , len+1 );
   fp->buf_set[len] = NULL;
   fp->buf_cur      = fp->buf_set + whence;
   fp->buffer_len   = len;

   return fp;
}

Sed_hydro_file sed_hydro_file_set_sig_values( Sed_hydro_file fp , int n_sig_values )
{
   fp->n_sig_values = n_sig_values;
   return fp;
}

Sed_hydrotrend_header *sed_hydro_file_header( Sed_hydro_file fp )
{
   return fp->hdr;
}

Sed_hydro
sed_hydro_file_read_record( Sed_hydro_file fp )
{
   return (*(fp->read_record))( fp );
}

/** Create a new Sed_hydro_file

\param filename       The name of the Sed_hydro_file
\param type           The file type.
\param wrap_is_on     If TRUE, when EOF is encountered, the file is rewound

\return A new Sed_hydro_file.  Use sed_hydro_file_destroy to free.
*/
Sed_hydro_file
sed_hydro_file_new( const char*         filename     ,
                    Sed_hydro_file_type type         ,
                    gboolean            buffer_is_on ,
                    gboolean            wrap_is_on   ,
                    GError** error )
{
   Sed_hydro_file fp;

   eh_require( error==NULL || *error==NULL );

   if ( filename )
   {
      GError* tmp_err = NULL;

      NEW_OBJECT( Sed_hydro_file , fp );

      // the 'b' (binary) option for fopen is supposed to be ignored.  however,
      // on some windows machines, it seems to be necessary.
      if ( strcmp( filename , "-" )==0 )
         fp->fp = stdin;
      else
      {
         if ( type==SED_HYDRO_INLINE ) fp->fp = eh_fopen_error( filename , "r"  , &tmp_err );
         else                          fp->fp = eh_fopen_error( filename , "rb" , &tmp_err );
      }

      if ( !tmp_err )
      {
         fp->file = g_strdup( filename );

         // there are two types of river files (currently) - inline and hydrotrend.
         // if using a hydrotrend file, there are two methods for reading - buffered
         // or unbuffered.
         if ( type==SED_HYDRO_INLINE )
         {
            fp->type         = SED_HYDRO_INLINE;
            fp->buf_set      = NULL;
            fp->buf_cur      = NULL;
            fp->buffer_len   = 0;
            fp->n_sig_values = 0;
//            fp->read_hdr     = (Hydro_read_header_func)&_hydro_read_inline_header;
            fp->read_hdr     = NULL;
            fp->read_record  = (Hydro_read_record_func)&_hydro_read_inline_record;

            fp->header_start = ftell(fp->fp);
            fp->hdr          = NULL;
            fp->data_start   = ftell(fp->fp);
         }
         else if (    type==SED_HYDRO_HYDROTREND 
                   || type==SED_HYDRO_HYDROTREND_BE
                   || type==SED_HYDRO_HYDROTREND_LE )
         {
            fp->type            = SED_HYDRO_HYDROTREND;

            if ( buffer_is_on )
            {
               fp->buf_set      = eh_new0( Sed_hydro , HYDRO_BUFFER_LEN+1 );
               fp->buf_cur      = fp->buf_set;
               fp->buffer_len   = HYDRO_BUFFER_LEN;
               fp->n_sig_values = HYDRO_N_SIG_VALUES;
               fp->read_hdr     = (Hydro_read_header_func)&_hydro_read_hydrotrend_header;
               fp->read_record  = (Hydro_read_record_func)&_hydro_read_hydrotrend_record_buffer;
            }
            else
            {
               fp->buf_set      = NULL;
               fp->buf_cur      = NULL;
               fp->buffer_len   = 0;
               fp->n_sig_values = 0;
               fp->read_hdr     = (Hydro_read_header_func)&_hydro_read_hydrotrend_header;
               fp->read_record  = (Hydro_read_record_func)&_hydro_read_hydrotrend_record;
            }

            fp->header_start = ftell(fp->fp);
            fp->hdr          = (*(fp->read_hdr))(fp);
            fp->data_start   = ftell(fp->fp);

         }
         else
            eh_require_not_reached();

         fp->data_size    = sizeof(float);
         fp->wrap_is_on   = wrap_is_on;
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         eh_free( fp );
         fp = NULL;
      }
   }

   return fp;
}

/** Destroy a Sed_hydro_file

\param fp A Sed_hydro_file to destroy

\return NULL.
*/
Sed_hydro_file sed_hydro_file_destroy( Sed_hydro_file fp )
{
   eh_return_val_if_fail( fp , NULL );

   if ( fp )
   {
      fclose( fp->fp );
      eh_free( fp->file );

      if ( fp->buf_set )
      {
         gssize i;
         for ( i=0 ; i<fp->buffer_len ; i++ )
            fp->buf_set[i] = sed_hydro_destroy( fp->buf_set[i] );
         eh_free(fp->buf_set);
      }

      /* If the input file was of type 'inline', there is NULL */
      if ( fp->hdr )
      {
         eh_free( fp->hdr->comment );
         eh_free( fp->hdr );
      }
      eh_free( fp );
   }

   return NULL;
}
/*
Hydro_header *_hydro_read_inline_header( Sed_hydro_file fp )
{
   return sed_hydro_scan_inline_header( fp->fp );
}
*/

static Sed_hydro
_hydro_read_inline_record( Sed_hydro_file fp )
{
   Sed_hydro rec = NULL;

   eh_require( fp );

   /* If the buffer hasn't already been set */
   if ( !(fp->buf_set) )
   {
      /* Fill the buffer with all the records from the file */
      fp->buf_set = sed_hydro_scan( fp->file , NULL );
      fp->buf_cur = fp->buf_set;
   }

   eh_require( fp->buf_set );

   if ( fp->buf_cur )
   {
      /* Duplicate the record at the top of the queue */
      rec = sed_hydro_dup( *(fp->buf_cur) );

      /* Update the pointer to the current record */
      fp->buf_cur++;

      /* Rewind the buffer (the buffer is NULL-terminated) */
      if ( *(fp->buf_cur)==NULL )
      {
         if ( fp->wrap_is_on ) fp->buf_cur = fp->buf_set;
         else                  fp->buf_cur = NULL;
      }
   }

   return rec;
}

Sed_hydrotrend_header*
_hydro_read_hydrotrend_header( Sed_hydro_file fp )
{
   return sed_hydrotrend_read_header( fp->fp );
}

Sed_hydro _hydro_read_hydrotrend_record( Sed_hydro_file fp )
{
   Sed_hydro rec;
   
   // read the record using the appropriate function.  if we encounter the end of
   // the file, start reading from the beginning of the data.  if wrap is off,
   // return with an error.
   rec = sed_hydrotrend_read_next_rec( fp->fp , fp->hdr->n_grains );
   if ( feof(fp->fp) )
   {
      if ( fp->wrap_is_on )
      {
         clearerr(fp->fp);
         fseek( fp->fp , fp->data_start , SEEK_SET );
         rec = (*(fp->read_record))( fp );
      }
      else
         eh_error( "Encountered end of file");
   }

   return rec;
}

Sed_hydro _hydro_read_hydrotrend_record_buffer( Sed_hydro_file fp )
{
   // if we are at the end of the buffer we must create a new buffer.
   // otherwise just return the next record in the buffer.
   if ( *(fp->buf_cur)==NULL )
   {
      sed_hydro_file_fill_buffer( fp );
   }
   fp->buf_cur += 1;

   return sed_hydro_dup(fp->buf_cur[-1]);
}

Sed_hydro *sed_hydro_file_fill_buffer( Sed_hydro_file fp )
{
   int i;
   int buffer_len         = fp->buffer_len;
   int n_sig_values       = fp->n_sig_values;
   Sed_hydro *temp_buffer = eh_new( Sed_hydro , buffer_len );
   Sed_hydro *buf_set;

   for ( i=0 ; i<buffer_len ; i++ )
   {
      fp->buf_set[i] = sed_hydro_destroy( fp->buf_set[i] );
   }

   for ( i=0 ; i<buffer_len ; i++ )
   {
      temp_buffer[i] = sed_hydrotrend_read_next_rec( fp->fp , fp->hdr->n_grains );

      if ( feof(fp->fp) )
      {
         if ( fp->wrap_is_on )
         {
            clearerr(fp->fp);
            fseek( fp->fp , fp->data_start , SEEK_SET );
            temp_buffer[i] = sed_hydrotrend_read_next_rec( fp->fp , fp->hdr->n_grains );
         }
         else
            eh_error( "Encountered end of the file");
      }
   }

   buf_set = sed_hydro_process_records( temp_buffer , buffer_len , n_sig_values , TRUE );

   for ( i=0 ; buf_set[i] ; i++ )
   {
      fp->buf_set[i] = buf_set[i];
   }
   fp->buf_set[i] = NULL;

   for ( i=0 ; i<buffer_len ; i++ )
      temp_buffer[i] = sed_hydro_destroy( temp_buffer[i] );
   eh_free(temp_buffer);
   eh_free(buf_set);

   fp->buf_cur = fp->buf_set;

   return fp->buf_set;
}

