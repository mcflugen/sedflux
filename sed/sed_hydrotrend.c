#include <stdio.h>
#include <glib.h>

#include "utils/utils.h"
#include "sed_hydrotrend.h"


GQuark
sed_hydrotrend_error_quark( void )
{
   return g_quark_from_static_string( "sed-hydrorend-error-quark" );
}

// Functions to read/write a standard HydroTrend output file.
/** Read the header of a HydroTrend file.

\param fp   The file to read.

\return The header information
*/
Sed_hydrotrend_header*
sed_hydrotrend_read_header( FILE *fp )
{
   return sed_hydrotrend_read_header_from_byte_order( fp , G_BYTE_ORDER );
}

/** Read the header of a HydroTrend file with a specific byte order

\param fp    The file to read.
\param order The byte order of the file

\return The header information
*/
Sed_hydrotrend_header*
sed_hydrotrend_read_header_from_byte_order( FILE *fp , gint order )
{
   Sed_hydrotrend_header *hdr = NULL;

   if ( fp )
   {
      gint n;
      size_t (*fread_int)(void*,size_t,size_t,FILE*);

      if ( order==G_BYTE_ORDER ) fread_int = fread;
      else                       fread_int = eh_fread_int32_swap;

      if ( fread_int( &n , sizeof(gint32)  , 1 , fp )==1 && (n>=0 && n<2048 ) )
      {
         gchar* str = eh_new( gchar , n+1 );

         hdr = eh_new( Sed_hydrotrend_header , 1 );

         fread( str , sizeof(gchar) , n , fp );
         str[n] = '\0';

         hdr->comment = g_strescape( str , "\\" );
         eh_free( str );

         if (    fread_int( &(hdr->n_grains ) , sizeof(gint)  , 1 , fp )!=1 || hdr->n_grains <=0
              || fread_int( &(hdr->n_seasons) , sizeof(gint)  , 1 , fp )!=1 || hdr->n_seasons<=0
              || fread_int( &(hdr->n_samples) , sizeof(gint)  , 1 , fp )!=1 || hdr->n_samples<=0 )
         {
            hdr = sed_hydrotrend_header_destroy( hdr );
         }
      }

      if ( !hdr )
      {
         eh_debug( "Trouble reading hydrotrend header." );
         eh_debug( "Is the byte of the hydrotrend file the same as that" );
         eh_debug( "on the machine you are running sedflux?" );
         eh_debug( "The byte order of your system is %s" ,
                   (G_BYTE_ORDER==G_BIG_ENDIAN)?"big-endian":"little-endian" );
      }
   }

   return hdr;
}

Sed_hydrotrend_header*
sed_hydrotrend_join_header_from_byte_order( FILE** fp_list , gint order , GError** err )
{
   Sed_hydrotrend_header *hdr = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( fp_list && *fp_list )
   {
      FILE**                 fp;
      Sed_hydrotrend_header* h;
      GError*                tmp_err = NULL;

      hdr = sed_hydrotrend_read_header_from_byte_order( fp_list[0] , order );
      rewind( fp_list[0] );

      for ( fp=fp_list+1 ; *fp && !tmp_err ; fp++ )
      {
         h = sed_hydrotrend_read_header_from_byte_order( *fp , order );
         rewind( *fp );

         if ( h )
         {
            if ( h->n_grains != hdr->n_grains && !tmp_err )
               g_set_error( &tmp_err , SED_HYDROTREND_ERROR , SED_HYDROTREND_ERROR_BAD_HEADER ,
                            "Number of grain sizes in Hydrotrend files do not match (%d!=%d)" , hdr->n_grains , h->n_grains );
            if ( h->n_seasons != hdr->n_seasons && !tmp_err )
               g_set_error( &tmp_err , SED_HYDROTREND_ERROR , SED_HYDROTREND_ERROR_BAD_HEADER ,
                            "Number of seasons in Hydrotrend files do not match (%d!=%d)" , hdr->n_seasons , h->n_seasons );

            if ( !tmp_err )
            {
//               gchar* tmp_str  = g_strdup( hdr->comment );
               gchar* tmp_str  = hdr->comment;

               hdr->comment    = g_strjoin( ";" , tmp_str , h->comment , NULL );
               hdr->n_samples += h->n_samples;

               eh_free( tmp_str );
            }

            eh_free( h->comment );
            eh_free( h          );
         }
      }

      if ( tmp_err )
      {
         g_propagate_error( err , tmp_err );
         eh_free( hdr->comment );
         eh_free( hdr          );
         hdr = NULL;
      }
   }

   return hdr;
}

Sed_hydrotrend_header*
sed_hydrotrend_header_destroy( Sed_hydrotrend_header* h )
{
   if ( h )
   {
      if ( h->comment ) eh_free( h->comment );
      eh_free( h );
   }
   return NULL;
}

gssize
sed_hydrotrend_write_header_to_byte_order( FILE*  fp        ,
                                           gint   n_grains  ,
                                           gint   n_seasons ,
                                           gint   n_samples ,
                                           gchar* comment_s ,
                                           gint   order )
{
   gssize n = 0;

   if ( fp )
   {
      gint len;
      gssize (*fwrite_int)(void*,size_t,size_t,FILE*);

      if ( !comment_s )
         comment_s = g_strdup( "No comment" );

      len = strlen( comment_s );

      if ( order==G_BYTE_ORDER ) fwrite_int = fwrite;
      else                       fwrite_int = eh_fwrite_int32_swap;

      n += fwrite_int( &len      , sizeof(int)  , 1   , fp );
      n += fwrite    ( comment_s , sizeof(char) , len , fp );

      n += fwrite_int( &n_grains  , sizeof(int)  , 1  , fp );
      n += fwrite_int( &n_seasons , sizeof(int)  , 1  , fp );
      n += fwrite_int( &n_samples , sizeof(int)  , 1  , fp );
   }

   return n;
}

/** Write the header of a HydroTrend file

\param fp        A HydroTrend file
\param n_grains  Number of suspended grains
\param n_seasons Number of "seasons" for the HydroTrend file
\param n_samples Number of records
\param comment_s String containing a comment (or NULL)

\return The number of bytes written
*/
gssize
sed_hydrotrend_write_header( FILE* fp       ,
                             gint n_grains  ,
                             gint n_seasons ,
                             gint n_samples ,
                             gchar* comment_s )
{
   gssize n = 0;

   if ( fp )
   {
      gint len;

      if ( !comment_s )
         comment_s = g_strdup( "No comment" );

      len = strlen( comment_s );

      n += fwrite( &len      , sizeof(int)  , 1   , fp );
      n += fwrite( comment_s , sizeof(char) , len , fp );

      n += fwrite( &n_grains  , sizeof(int)  , 1  , fp );
      n += fwrite( &n_seasons , sizeof(int)  , 1  , fp );
      n += fwrite( &n_samples , sizeof(int)  , 1  , fp );
   }

   return n;
}

gint
sed_hydrotrend_byte_order( const gchar* file , GError** error )
{
   gint order = 0;

   eh_require( error==NULL || *error==NULL );

   if ( file )
   {
      GError* tmp_err = NULL;
      FILE*   fp      = eh_fopen_error( file , "r" , &tmp_err );

      if ( !tmp_err )
      {
         Sed_hydrotrend_header* h = NULL;

         h = sed_hydrotrend_read_header_from_byte_order( fp , G_BIG_ENDIAN );
         if ( h )
            order = G_BIG_ENDIAN;
         else
         {
            rewind( fp );
            h = sed_hydrotrend_read_header_from_byte_order( fp , G_LITTLE_ENDIAN );
            order = G_LITTLE_ENDIAN;
         }

         if ( h )
         {
            eh_free( h->comment );
            eh_free( h          );
         }

         fclose( fp );
      }
   }

   return order;
}

gint
sed_hydrotrend_guess_byte_order( FILE* fp )
{
   gint byte_order = -1;

   if ( fp )
   {
      gint n;
      fread( &n , sizeof(gint)  , 1 , fp );

      if ( n>=0 || n<2048 )
         byte_order = G_BYTE_ORDER;
      else
      {
         rewind( fp );
         eh_fread_int32_swap( &n , sizeof(gint)  , 1 , fp );

         if ( n>=0 || n<2048 )
         {
            if ( G_BYTE_ORDER==G_BIG_ENDIAN ) byte_order = G_LITTLE_ENDIAN;
            else                              byte_order = G_BIG_ENDIAN;
         }
      }

      rewind( fp );
   }

   return byte_order;
}

/** Read one record from a HydroTrend file.

\param fp       The file to read
\param n_grains The number of suspended grains in the file

\return A newly-created Sed_hydro.  Use sed_hydro_destroy to free.
*/
Sed_hydro
sed_hydrotrend_read_next_rec( FILE* fp , int n_grains )
{
   return sed_hydrotrend_read_next_rec_from_byte_order( fp , n_grains , G_BYTE_ORDER );
}


/** Read a Sed_hydro from a HydroTrend file of a specific byte order

\param fp       A HydroTrend file
\param n_grains Number of suspended grains in the Sed_hydro
\param order    The byte order to write

\return A newly allocated Sed_hydro that contains the read data
*/
Sed_hydro
sed_hydrotrend_read_next_rec_from_byte_order( FILE *fp , int n_grains , gint order )
{
   int n;
   float* fval = eh_new( float , 4+n_grains );
   Sed_hydro rec = NULL;

   if ( order==G_BYTE_ORDER ) n = fread            ( fval , sizeof(float) , 4+n_grains , fp );
   else                       n = eh_fread_flt_swap( fval , sizeof(float) , 4+n_grains , fp );

   if ( n==4+n_grains )
   {
      gint i;

      rec = sed_hydro_new( n_grains );

      sed_hydro_set_velocity( rec , fval[0] );
      sed_hydro_set_width   ( rec , fval[1] );
      sed_hydro_set_depth   ( rec , fval[2] );
      sed_hydro_set_bedload ( rec , fval[3] );

      for (i=0 ; i<n_grains ; i++)
         sed_hydro_set_nth_concentration( rec , i , fval[i+4] );
   }
   eh_free( fval );

   return rec;
}

/** Read a range of records from a HydroTrend file

\param fp         A HydroTrend file
\param rec_0      The first record to read (starting from 0)
\param n_recs     The number of records to read
\param byte_order The byte order of the hydroTrend file
\param error      A GError

\return A NULL-terminated Sed_hydro array that contains the HydroTrend data
*/
Sed_hydro*
sed_hydrotrend_read_recs( FILE* fp , gint rec_0 , gint n_recs , gint byte_order , GError** error )
{
   Sed_hydro* rec_a = NULL;

   eh_require( rec_0  >= 0 );

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( fp || n_recs!=0 )
   {
      gint n;
      gint last     = sed_hydrotrend_fseek( fp , 0     , SEEK_END , byte_order );
      gint cur      = sed_hydrotrend_fseek( fp , rec_0 , SEEK_SET , byte_order );
      gint n_grains = sed_hydrotrend_n_grains( fp , byte_order , NULL );

      if ( last-cur < n_recs || n_recs<0 )
         n_recs = last - cur;

      rec_a = eh_new( Sed_hydro , n_recs+1 );

      for ( n=0 ; n<n_recs ; n++ )
         rec_a[n] = sed_hydrotrend_read_next_rec_from_byte_order( fp , n_grains , byte_order );
      rec_a[n] = NULL;
   }

   return rec_a;
}

/** Write a Sed_hydro to a HydroTrend file

\param fp    A HydroTrend file
\param rec   A Sed_hydro to write

\return The number of bytes written
*/
gssize
sed_hydrotrend_write_record( FILE *fp , Sed_hydro rec )
{
   return sed_hydrotrend_write_record_to_byte_order( fp , rec , G_BYTE_ORDER );
}

/** Write a Sed_hydro to a HydroTrend file with a specific byte order

\param fp    A HydroTrend file
\param rec   A Sed_hydro to write
\param order The byte order to write

\return The number of bytes written
*/
gssize
sed_hydrotrend_write_record_to_byte_order( FILE *fp , Sed_hydro rec , gint order )
{
   gssize n;

   if ( rec )
   {
      gint   i;
      gint   n_grains = sed_hydro_size( rec );
      float* fval     = eh_new( float , 4+n_grains );

      fval[0] = sed_hydro_velocity( rec );
      fval[1] = sed_hydro_width   ( rec );
      fval[2] = sed_hydro_depth   ( rec );
      fval[3] = sed_hydro_bedload ( rec );

      for (i=0 ; i<n_grains ; i++)
         fval[4+i] = sed_hydro_nth_concentration( rec , i );

      if ( order==G_BYTE_ORDER )
         n = fwrite            ( fval , sizeof(float) , 4+n_grains , fp );
      else
         n = eh_fwrite_flt_swap( fval , sizeof(float) , 4+n_grains , fp );

      eh_free( fval );
   }

   return n;
}

/** Read a series of HydroTrend records

\param fp       The file to read from
\param rec      The location to put the records
\param n_grains The number of suspended grain sizes.
\param n_recs   The number of records to read.

\return The number of records read.
*/
gssize
sed_hydrotrend_read_next_n_recs( FILE* fp , Sed_hydro* rec , int n_grains , int n_recs )
{
   gssize n = 0;
   
   do
   {
      rec[n] = sed_hydrotrend_read_next_rec( fp , n_grains );
   }
   while ( rec[n] && (++n)<n_recs );

   return n;
}

Sed_hydro*
sed_hydrotrend_read( const gchar* file , gint byte_order , int* n_seasons , GError** error )
{
   return sed_hydrotrend_read_n_recs( file , -1 , byte_order , n_seasons , error );
}

Sed_hydro*
sed_hydrotrend_read_n_recs( const gchar* file , gint n_recs , gint byte_order , int* n_seasons , GError** error )
{
   Sed_hydro* arr = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file && n_recs!=0 )
   {
      GError* err    = NULL;
      FILE*   fp     = eh_fopen_error( file , "r" , &err );

      if ( !err )
      {
         Sed_hydrotrend_header* h = NULL;

         h = sed_hydrotrend_read_header_from_byte_order( fp , byte_order );

         if ( h )
         {
            if      ( n_recs<0            ) n_recs = h->n_samples;
            else if ( n_recs>h->n_samples ) n_recs = h->n_samples;

            arr = sed_hydrotrend_read_recs( fp , 0 , n_recs , byte_order , &err );

            if ( n_seasons ) *n_seasons = h->n_seasons;

            if ( !err )
            {
               gint n_read = g_strv_length( (gchar**)arr );

               if ( n_read != n_recs )
               {
                  eh_warning( "Number of items read does not match number of items in header" );
                  eh_debug( "Number of items in header : %d" , h->n_samples );
                  eh_debug( "Number of items read      : %d" , n_read       );
               }
            }
            else
               g_propagate_error( error , err );

            eh_free( h->comment );
            eh_free( h          );
         }
      }
      else
         g_propagate_error( error , err );
   }

   return arr;
}

/** Write a complete HydroTrend file

\param file      The name of the file
\param rec_a     A NULL-terminated array of Sed_hydro records
\param n_seasons The number of "seasons" used for the HydroTrend file
\param comment_s A comment string (or NULL)
\param error     A GError

\return The number of bytes written
*/
gssize
sed_hydrotrend_write( gchar* file , Sed_hydro* rec_a , gint n_seasons , gchar* comment_s , GError** error )
{
   gssize n = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );
   eh_require( n_seasons>0 );

   if ( file && rec_a && *rec_a )
   {
      GError* err    = NULL;
      FILE* fp       = eh_fopen_error( file , "w" , &err );
      gint n_samples = g_strv_length( (gchar**)rec_a );
      gint n_grains  = sed_hydro_size(rec_a[0]);

      if ( !err )
      {
         sed_hydrotrend_write_header( fp , n_grains , n_seasons , n_samples , comment_s );
         sed_hydro_array_write_hydrotrend_records( fp , rec_a );
      }
      else
      {
         g_propagate_error( error , err );
         n = 0;
      }
   }

   return n;
}

gssize
sed_hydro_array_write_hydrotrend_records_to_byte_order( FILE* fp , Sed_hydro* rec_a , gint order )
{
   gssize n = 0;

   if ( fp && rec_a )
   {
      Sed_hydro* rec;

      for ( rec=rec_a ; *rec ; rec++ )
         sed_hydrotrend_write_record_to_byte_order( fp , *rec , order );
   }

   return n;
}

/** Write records to a HydroTrend file

\param fp   A HydroTrend file
\param rec_a A NULL-terminated array of Sed_hydro records

\return The number of bytes written.
*/
gssize
sed_hydro_array_write_hydrotrend_records( FILE* fp , Sed_hydro* rec_a )
{
   return sed_hydro_array_write_hydrotrend_records_to_byte_order( fp , rec_a , G_BYTE_ORDER );
}

/** Reposition a file-position indicator of a HydroTrend file

Set the file-position indicator for a HydroTrend file.  This
has two major differences from the standard fseek function. 
(1) The offset is measured in records rather than bytes.  
(2) The new position withing the file is returned (in records).

For example:

sed_hydro_fseek( fp , 0 , SEEK_SET )

moves the file-position indicator to the start of the data.

sed_hydro_fseek( fp , 365 , SEEK_SET )

moves the file-position indicator to the 365th record in
the file.

\param fp          A HydroTrend file
\param offset      The offset (in HydroTrend records) to move
\param whence      Where offset is measured from
\param byte_order  The byte order of the HydroTrend file

\return 0 on success, -1 otherwise

*/
gint
sed_hydrotrend_fseek( FILE* fp , gint offset , gint whence , gint byte_order )
{
   gint pos = -1;

   if ( fp )
   {
      gint rec_size   = sed_hydrotrend_record_size( fp , byte_order , NULL );
      gint data_start = sed_hydrotrend_data_start ( fp , byte_order , NULL );

      if ( whence == SEEK_SET )
         fseek( fp , rec_size*offset + data_start , SEEK_SET );
      else
         fseek( fp , rec_size*offset , whence );

      pos = ( ftell( fp ) - data_start ) / rec_size ;
   }

   return pos;
}

/** The size of a HydroTrend record in bytes

\param fp          A pointer to a HydroTrend file
\param byte_order  The byte order of the HydroTrend file
\param h           The header information if known, NULL otherwise

\return The size of a record in bytes
*/
gint
sed_hydrotrend_record_size( FILE* fp , gint byte_order , Sed_hydrotrend_header* h )
{
   gint n = 0;

   if ( fp )
   {
      if ( h )
         n = ( h->n_grains + 4 )*sizeof(float);
      else
      {
         gint where = ftell( fp );

         rewind( fp );

         h = sed_hydrotrend_read_header_from_byte_order( fp , byte_order );

         eh_require( h );

         n = ( h->n_grains + 4 )*sizeof(float);

         fseek( fp , where , SEEK_SET );

         eh_free( h->comment );
         eh_free( h );
      }
   }

   return n;
}

/** The number of suspended grains of a HydroTrend record

\param fp          A pointer to a HydroTrend file
\param byte_order  The byte order of the HydroTrend file
\param h           The header information if known, NULL otherwise

\return The number of suspended grains
*/
gint
sed_hydrotrend_n_grains( FILE* fp , gint byte_order , Sed_hydrotrend_header* h )
{
   gint n = 0;

   if ( fp )
   {
      if ( h )
         n = h->n_grains;
      else
      {
         gint where = ftell( fp );

         rewind( fp );

         h = sed_hydrotrend_read_header_from_byte_order( fp , byte_order );

         eh_require( h );

         n = h->n_grains;

         fseek( fp , where , SEEK_SET );

         eh_free( h->comment );
         eh_free( h );
      }
   }

   return n;
}

/** The file position that marks the start of the data

\param fp          A HydroTrend file
\param byte_order  The byte order of the HydroTrend file
\param h           The header info for the file (or NULL)

\return The offset (in bytes) from the start of the file to the
        start of the data
*/
gint
sed_hydrotrend_data_start( FILE* fp , gint byte_order , Sed_hydrotrend_header* h )
{
   gint n = 0;

   if ( fp )
   {
      if ( h )
         n = sizeof(gint) + strlen(h->comment) + 3*sizeof(gint);
      else
      {
         gint where = ftell( fp );

         rewind( fp );

         h = sed_hydrotrend_read_header_from_byte_order( fp , byte_order );

         eh_require( h );

         n = ftell( fp );

         fseek( fp , where , SEEK_SET );

         eh_free( h->comment );
         eh_free( h );
      }
   }

   return n;
}

