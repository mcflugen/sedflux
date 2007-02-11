#include "eh_dlm_file.h"
#include "utils.h"

#include <string.h>

/* Local function declarations */
gchar*     eh_dlm_prepare                ( const gchar* file ,
                                           GError** err );
double**   eh_dlm_read_data              ( gchar* str        ,
                                           gchar* delims     ,
                                           gint* n_rows      ,
                                           gint* n_cols      ,
                                           GError** err );

gint       eh_dlm_find_n_cols            ( gchar* content , gchar* delims );
gint       eh_dlm_find_n_rows            ( gchar* content , gint delim );

gchar**    eh_dlm_split_records          ( gchar* str             ,
                                           const gchar* start_str ,
                                           const gchar* end_str   ,
                                           gchar*** rec_data );

gchar*
eh_dlm_prepare( const gchar* file , GError** err )
{
   gchar* str = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( file )
   {
      GError* tmp_error = NULL;

      g_file_get_contents( file , &str , NULL , &tmp_error );

      if ( !tmp_error )
      {
         /* Remove comments and empty lines */
         eh_str_remove_c_style_comments( str );
         eh_str_remove_to_eol_comments ( str , "#"  );
         eh_str_remove_to_eol_comments ( str , "//" );
         eh_str_replace                ( str , '\r' , '\n' );
         eh_dlm_remove_empty_lines     ( str );
      }
      else
         g_propagate_error( err , tmp_error );
   }

   return str;
}

/** Read a delimited file

Read data from the ASCII file \p file.  Data are written row-by-row with
each column delimited by any of the characters contained in \p delims.  The
number of columns is determined by the line with the largest number of 
elements.  Rows with fewer columns are padded with zeros.

\param file   The name of the data file
\param delims A string of possible delimeters
\param n_rows Location to put the number of rows read
\param n_cols Location to put the number of columns read
\param err    Retrun location for a GError, or NULL

\return A 2d array of the data scanned from the file.  Use eh_free_2 to free.

\sa eh_dlm_read_swap
*/
double**
eh_dlm_read( const gchar* file , 
             gchar* delims     ,
             gint* n_rows      ,
             gint* n_cols      ,
             GError** err )
{
   double** data = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( file )
   {
      double*** data_3;
      gint* n_x = NULL;
      gint* n_y = NULL;
      GError* tmp_err = NULL;

      data_3 = eh_dlm_read_full( file , delims , &n_x , &n_y , NULL , 1 , &tmp_err );

      if ( data_3 )
      {
         data    = data_3[0];
         *n_rows = n_x[0];
         *n_cols = n_y[0];

         eh_free( data_3 );
         eh_free( n_x    );
         eh_free( n_y    );
      }
      else
         g_propagate_error( err , tmp_err );
   }

   return data;
}

/** Read a delimited file and swap rows and columns

Same as eh_dlm_read except that the rows and columns are swapped.  That is,
what were columns in the file are now rows in the returned matrix.

\param file   The name of the data file
\param delims A string of possible delimeters
\param n_rows Location to put the number of rows read
\param n_cols Location to put the number of columns read
\param err    Retrun location for a GError, or NULL

\return A 2d array of the data scanned from the file.  Use eh_free_2 to free.

\sa eh_dlm_read

*/
double** eh_dlm_read_swap( const gchar* file ,
                           gchar* delims     ,
                           gint* n_rows      ,
                           gint* n_cols      ,
                           GError** err )
{
   double** data = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( file )
   {
      gint tmp_rows, tmp_cols;
      double** tmp;
      GError* tmp_err = NULL;

      tmp  = eh_dlm_read( file , delims , &tmp_rows , &tmp_cols , &tmp_err );

      if ( tmp )
      {
         gint i, j;

         data = eh_new_2( double , tmp_cols , tmp_rows );

         for ( i=0 ; i<tmp_rows ; i++ )
            for ( j=0 ; j<tmp_cols ; j++ )
               data[j][i] = tmp[i][j];

         *n_rows = tmp_cols;
         *n_cols = tmp_rows;

      }
      else
         g_propagate_error( err , tmp_err );

      eh_free_2( tmp );
   }

   return data;
}

/** Read multiple records from a text delimited file

Similar to eh_dlm_read except that multiple records can be present
within the text file.  Records begin with a head that is enclosed
by square brackets.  The data are read with the same format as that used
by eh_dlm_read.

\param file        The name of the data file
\param delims      A string of possible delimeters
\param n_rows      Location to put the array of number of rows read
\param n_cols      Location to put the array of number of columns read
\param rec_data    Location to hold a NULL-terminated array header strings
\param max_records If greater than 0, the maximum number of records to scan.
                   If less than 0, all of the records will be scanned.
\param err         Retrun location for a GError, or NULL

\return A 2d array of the data scanned from the file.  Use eh_free_2 to free.

\sa eh_dlm_read, eh_dlm_read_full_swap
*/
double***
eh_dlm_read_full( const gchar* file ,
                  gchar* delims     ,
                  gint** n_rows     ,
                  gint** n_cols     ,
                  gchar*** rec_data ,
                  gint max_records  ,
                  GError** err )
{
   double*** data = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( file )
   {
      gchar* str;
      GError* tmp_error = NULL;

      str = eh_dlm_prepare( file , &tmp_error );

      if ( !tmp_error )
      {
         gchar** rec;
         gint i, n_recs;
         rec = eh_dlm_split_records( str , "[" , "]" , rec_data );
         n_recs  = g_strv_length( rec );

         if ( max_records>0 )
            n_recs = MAX( max_records , n_recs );

         data    = eh_new( double** , n_recs+1 );
         *n_rows = eh_new( gint     , n_recs );
         *n_cols = eh_new( gint     , n_recs );

         for ( i=0 ; i<n_recs ; i++ )
         {
            data[i] = eh_dlm_read_data( rec[i]    ,
                                        delims    ,
                                        *n_rows+i ,
                                        *n_cols+i ,
                                        NULL );
         }
         data[i] = NULL;

         g_strfreev( rec );
      }
      else
         g_propagate_error( err , tmp_error );

      eh_free( str );
   }

   return data;
}

/** Read multiple records from a text delimited file and swap rows and columns

Same as eh_dlm_read_full except that the rows and columns of the output
matrices are swapped.

\param file        The name of the data file
\param delims      A string of possible delimeters
\param n_rows      Location to put the array of number of rows read
\param n_cols      Location to put the array of number of columns read
\param rec_data    Location to hold a NULL-terminated array header strings
\param max_records If greater than 0, the maximum number of records to scan.
                   If less than 0, all of the records will be scanned.
\param err         Retrun location for a GError, or NULL

\return A 2d array of the data scanned from the file.  Use eh_free_2 to free.

\sa eh_dlm_read_swap, eh_dlm_read_full
*/
double***
eh_dlm_read_full_swap( const gchar* file ,
                       gchar* delims     ,
                       gint** n_rows     ,
                       gint** n_cols     ,
                       gchar*** rec_data ,
                       gint max_records  ,
                       GError** err )
{
   double*** data = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( file )
   {
      gint i, j;
      gint n, n_recs;
      double*** tmp;
      GError* tmp_err = NULL;

      tmp  = eh_dlm_read_full( file , delims , n_rows , n_cols , rec_data , max_records , &tmp_err );

      n_recs = g_strv_length( (gchar**)tmp );

      data = eh_new( double** , n_recs+1 );

      for ( n=0 ; n<n_recs ; n++ )
      {
         data[n] = eh_new_2( double , (*n_cols)[n] , (*n_rows)[n] );

         for ( i=0 ; i<(*n_rows)[n] ; i++ )
            for ( j=0 ; j<(*n_cols)[n] ; j++ )
               data[n][j][i] = tmp[n][i][j];

         eh_free_2( tmp[n] );
      }
      data[n_recs] = 0;

      EH_SWAP_PTR( *n_rows , *n_cols );

      eh_free( tmp );
   }

   return data;
}

double**
eh_dlm_read_data( gchar* str        ,
                  gchar* delims     ,
                  gint* n_rows      ,
                  gint* n_cols      ,
                  GError** err )
{
   double** data;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   if ( !delims )
      delims = ";,";

   if ( str )
   {
      *n_rows = eh_dlm_find_n_rows( str , '\n' );
      *n_cols = eh_dlm_find_n_cols( str , delims );

      /* Parse the data */
      {
         gint i, j;
         gchar* pos_0;
         gchar* pos_1;
         gchar** values;
         gchar** line;
         gint n_vals;
         gchar* str_end = str+strlen(str);

         data = eh_new_2( double , *n_rows , *n_cols );

         pos_0 = str;
         for ( i=0 ; pos_0<str_end ; i++ )
         {
            pos_1 = strchr( pos_0 , '\n' );
            if ( !pos_1 )
               pos_1 = str_end;
            pos_1[0] = '\0';

            values = g_strsplit_set( pos_0 , delims , -1 );

            n_vals = g_strv_length( values );

            for ( j=0 ; j<n_vals ; j++ )
               data[i][j] = g_ascii_strtod( values[j] , NULL );
            for (     ; j<*n_cols ; j++ )
               data[i][j] = 0.;

            pos_0 = pos_1+1;

            g_strfreev( values );
         }
      }
   }

   return data;
}

gint
eh_dlm_find_n_rows( gchar* str , gint delim )
{
   gint n = 0;

   if ( str )
   {
      n = eh_str_count_chr( str , str+strlen(str) , delim )+1;
   }

   return n;
}

/** Count the number of occurances of a character in a string

Count the number of times \p delim occurs from \p str up to
and including \p end.

\param str   The start of the string
\param end   The end of the string
\param delim The character to look for

\return The number of occurances of the character in the string
*/
gint
eh_str_count_chr( gchar* str , gchar* end , gint delim )
{
   gint n = 0;

   if ( str )
   {   
      gchar* pos;
      for ( pos=strchr(str  , delim ),n=0 ;
            pos && pos<=end ;
            pos=strchr(pos+1, delim ),n++ );
   }

   return n;
}

gint
eh_dlm_find_n_cols( gchar* str , gchar* delims )
{
   gint n_cols = 0;

   if ( str )
   {
      gint i, n;
      gint n_delims  = strlen(delims);
      gchar* str_end = str + strlen(str);
      gchar* pos_0;
      gchar* pos_1;

      pos_0 = str;
      while ( pos_0<str_end )
      {
         pos_1 = strchr( pos_0 , '\n' );
         if ( !pos_1 )
            pos_1 = str_end;

         for ( i=0,n=0 ; i<n_delims ; i++ )
            n += eh_str_count_chr( pos_0 , pos_1 , delims[i] );

         if ( n>n_cols )
            n_cols = n;

         pos_0 = pos_1+1;
      }
   }

   return n_cols+1;
}

/** Remove empty lines from a string

This function does not allocate or reallocate any memory.  It modifies
\p str in place.  The pointer to \p str is returned to allow
the nesting of functions.

\param str A string to remove empty lines from.

\return \p str
*/
gchar*
eh_dlm_remove_empty_lines( gchar* str )
{
   if ( str && str[0]!='\0' )
   {
      gchar*  pos;
      gchar*  pos_0;
      gchar*  last_line;
      gint    len       = strlen(str);
      gchar*  str_end   = str+len;
      gchar** block     = eh_new0( gchar* , len );
      gchar** block_end = eh_new0( gchar* , len );
      gint    i=0;

      pos_0 = str;
      while ( pos_0>=str && pos_0<str_end )
      {
         last_line = NULL;

         for ( pos=pos_0 ; pos<str_end && g_ascii_isspace(*pos) ; pos++ )
         {
            if ( *pos == '\n' )
               last_line = pos;
         }

         if ( last_line )
         {
            block[i]     = pos_0;
            block_end[i] = last_line+1;
            i += 1;
         }

         pos_0 = strchr( pos , '\n' )+1;
      }
      eh_str_remove_blocks( str , block , block_end );

      eh_free( block_end );
      eh_free( block     );

      if ( str[strlen(str)-1]=='\n' )
         str[strlen(str)-1]='\0';
   }
   return str;
}

/** Remove comments that run to the end of a line

Remove comments that begin with \p com_start and continue until
the end of the line.  This function does not allocate or
reallocate any memory.  It modifies \p str in place.
The pointer to \p str is returned to allow the nesting of functions.

\param str       A string to remove comments from.
\param com_start A string that marks the start of a comment.

\return \p str
*/
gchar*
eh_str_remove_to_eol_comments( gchar* str , gchar* com_start )
{
   return eh_str_remove_comments( str , com_start , NULL , NULL );
}

/** Remove c-style comments from a string

Remove comments that begin with '\/*' and end with '*\/'
This function does not allocate or reallocate any memory.
It modifies \p str in place.  The pointer to \p str is
returned to allow the nesting of functions.

\param str       A string to remove comments from.

\return \p str
*/
gchar*
eh_str_remove_c_style_comments( gchar* str )
{
   return eh_str_remove_comments( str , "/*" , "*/" , NULL );
}

/** Remove comments from a string

Remove comments that begin with \p start_str and end with \p end_str.
If \p end_str is NULL, then each comment is assumed to end at the end
of the line.  This function does not allocate or reallocate any memory.
It modifies \p str in place.  The pointer to \p str is returned to allow
the nesting of functions.

If non-NULL, the parameter \p comments will contain a (newly-allocated)
string array of the comments that were removed.  Use g_strfreev to 
free when no longer in use.

\param str       A string to remove comments from.
\param start_str A string that indicates the start of a comment.
\param end_str   A string that indicates the end of a comment (or NULL
                 if the comment terminates at the end of its line).
\param comments  Location to put a string array that contains the comments
                 that were removed (or NULL).

\return \p str
*/
gchar*
eh_str_remove_comments( gchar* str             ,
                        const gchar* start_str ,
                        const gchar* end_str   ,
                        gchar*** comments )
{
   gchar* str_0 = str;
   gint end_len;

   eh_require( start_str );

   if ( !end_str )
   {
      /* This is a special case where the comment ends at the end of the
         line but we don't want to remove the EOL character */
      end_str = "\n";
      end_len = 0;
   }
   else
      end_len = strlen(end_str);

   if ( comments )
      *comments = NULL;

   if ( str )
   {
      gchar* pos_0;
      gchar* pos_1;
      gchar* str_end = str+strlen(str);
      gint   start_len = strlen(start_str);
      gint   len = 1;

      pos_0 = strstr( str , start_str );
      while ( pos_0 )
      {
         pos_1 = strstr( pos_0 , end_str );

         if ( !pos_1 )
            pos_1 = str_end-end_len;

         if ( comments )
         {
            len             += 1;
            *comments        = eh_renew( gchar* , *comments , len );
            *comments[len-2] = g_strndup( pos_0 + start_len ,
                                          pos_1 - (pos_0+start_len) );
            *comments[len-1] = NULL;
         }

         g_memmove( pos_0 ,
                    pos_1+end_len ,
                    str_end - (pos_1+end_len)+1 );
         str_end -= pos_1+end_len - pos_0;

         pos_0 = strstr( pos_0 , start_str );
      }
   }

   return str;
}

gchar**
eh_dlm_split_records( gchar* str             ,
                      const gchar* start_str ,
                      const gchar* end_str   ,
                      gchar*** rec_data )
{
   gchar** split_str = NULL;

   eh_require( start_str );
   eh_require( end_str   );

   if ( rec_data )
      *rec_data = NULL;

   if ( str )
   {
      for ( ; g_ascii_isspace(*str) ; str++ );

      if ( strstr( str , start_str ) != str )
      {
         eh_strv_append( &split_str , g_strdup(str) );
      }
      else
      {
         gchar* pos_0;
         gchar* pos_1;
         gchar* str_end = str+strlen(str);
         gint start_len = strlen(start_str);
         gint end_len   = strlen(end_str  );
         gint len;
         gchar* new_line;

         pos_0 = str+start_len;
         for ( len=2 ; pos_0<str_end ; len++ )
         {
            /* Find the close of the header (or the end of the string) */
            pos_1 = strstr( pos_0 , end_str );
            if ( !pos_1 )
               pos_1 = str_end-end_len;

            /* Save the header info */
            if ( rec_data )
            {
               new_line = g_strndup( pos_0 , pos_1-pos_0 );
               eh_strv_append( rec_data , new_line );
               g_strstrip( new_line );
            }

            /* Go to the start of the data */
            pos_0 = pos_1 + end_len;
            for ( ; g_ascii_isspace(*pos_0) ; pos_0++ );

            /* Find the end of the data (the start of the next header) */
            pos_1 = strstr( pos_0 , start_str );
            if ( !pos_1 )
               pos_1 = str_end;

            new_line = g_strndup( pos_0 , pos_1-pos_0 );
            g_strstrip( new_line );
            
            eh_strv_append( &split_str , new_line );

            pos_0 = pos_1+start_len;
            for ( ; pos_0<str_end && g_ascii_isspace(*pos_0) ; pos_0++ );
         }
      }
   }

   return split_str;
}

/** Parse a string into key-value pairs

Key-value pairs are separated by the delimiter, \p delim_2.  The keys and
values are separated by \p delim_1.  A Eh_symbol_table is created that 
holds all of the key-value pairs.

\param str       A string to parse.
\param delim_1   The delimiter that separates keys from values.
\param delim_2   The delimiter that separates key-value pairs from one another.

\return A Eh_symbol_table of key-value pairs.  Use eh_symbol_table_destroy to free.
*/
Eh_symbol_table
eh_str_parse_key_value( gchar* str , gchar* delim_1 , gchar* delim_2 )
{
   Eh_symbol_table tab = NULL;

   if ( str )
   {
      gchar** key_value;
      gchar** pairs;
      gint i, n_pairs;

      tab = eh_symbol_table_new( );

      pairs = g_strsplit( str , delim_2 , -1 );
      n_pairs = g_strv_length( pairs );
      for ( i=0 ; i<n_pairs ; i++ )
      {
         key_value = g_strsplit( pairs[i] , delim_1 , 2 );

         eh_require( g_strv_length(key_value)==2 )
         {
            eh_str_remove_comments( key_value[0] , "(" , ")" , NULL );
            g_strstrip( key_value[0] );
            g_strstrip( key_value[1] );

            eh_symbol_table_insert( tab , key_value[0] , key_value[1] );
         }

         g_strfreev( key_value );
      }
      g_strfreev( pairs );

      if ( eh_symbol_table_size(tab)==0 )
         tab = eh_symbol_table_destroy(tab);
   }

   return tab;
}

/** Append a string to the end of an array of strings.

If str_l points to NULL, a new string array will be allocated.
The location of \p new_str is appended to \p str_l rather than
a copy.  Thus, \p new_str should not be freed before \p str_l.

\param str_l   The location of a string array, or NULL.
\param new_str The string to append.

\return The input string_array.
*/
gchar** eh_strv_append( gchar*** str_l , gchar* new_str )
{
   if ( str_l && new_str )
   {
      if ( *str_l==NULL )
      {
         *str_l = eh_new( gchar* , 2 );

         (*str_l)[0] = new_str;
         (*str_l)[1] = NULL;
      }
      else
      {
         gint len = g_strv_length(*str_l)+1;

         *str_l = eh_renew( gchar* , *str_l , len+1 );

         (*str_l)[len-1] = new_str;
         (*str_l)[len]   = NULL;
      }
   }

   return *str_l;
}

gchar* eh_str_replace( gchar* str , gchar old_c , gchar new_c )
{
   if ( str )
   {
      gchar* pos;
      for ( pos=strchr(str  , old_c ) ;
            pos ;
            pos=strchr(pos+1, old_c ) )
         pos[0] = new_c;
   }
   return str;
}

gchar* eh_str_remove( gchar* str , gchar* start , gint n )
{
   if ( str )
   {
      gchar* tail = start+n;
      g_memmove( start , tail , strlen(tail)+1 );
   }
   return str;
}

gchar* eh_str_remove_blocks( gchar* str , gchar** block_start , gchar** block_end )
{
   if ( str && block_start && block_start[0] )
   {
      gchar* tail;
      gint i, n;
      gint len = g_strv_length( block_start );

      block_start[len] = block_end[len-1] + strlen(block_end[len-1])+1;

      tail = block_start[0];
      for ( i=0 ; i<len ; i++ )
      {
         n = block_start[i+1] - block_end[i];
         g_memmove( tail , block_end[i] , n );
         tail += n;
      }

      block_start[len] = NULL;
   }
   return str;
}

gint
eh_dlm_print_dbl_grid( const gchar* file , const gchar* delim , Eh_dbl_grid g , GError** error ) 
{
   gint n = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );
   eh_require( g     );

   if ( g )
   {
      FILE* fp;
      GError* tmp_err = NULL;

      if ( file )
         fp = eh_fopen_error( file , "w" , &tmp_err );
      else
         fp = stdout;

      if ( fp )
      {
         gint i, j;
         gint n_rows   = eh_grid_n_x( g );
         gint top_col  = eh_grid_n_y( g )-1;
         double** data = eh_dbl_grid_data( g );

         if ( !delim )
            delim = " ";

         for ( i=0 ; i<n_rows ; i++ )
         {
            for ( j=0 ; j<top_col ; j++ )
               n += fprintf( fp , "%f%s" , data[i][j] , delim );
            n += fprintf( fp , "%f" , data[i][j] , delim );
            n += fprintf( fp , "\n" );
         }
      }
      else
         g_propagate_error( error , tmp_err );
   }

   return n;
}

