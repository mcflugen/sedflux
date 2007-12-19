#include <eh_utils.h>

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

gchar**
eh_dlm_split_records( gchar*       str       ,
                      const gchar* start_str ,
                      const gchar* end_str   ,
                      gchar***     rec_data )
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

gint
eh_dlm_print( const gchar* file , const gchar* delim , const double** data , const gint n_rows , const gint n_cols , GError** error ) 
{
   return eh_dlm_print_full( file , delim , data , n_rows , n_cols , FALSE , error );
}

gint
eh_dlm_print_swap( const gchar* file , const gchar* delim , const double** data , const gint n_rows , const gint n_cols , GError** error ) 
{
   return eh_dlm_print_full( file , delim , data , n_rows , n_cols , TRUE , error );
}

gint
eh_dlm_print_full( const gchar* file , const gchar* delim , const double** data , const gint n_rows , const gint n_cols , gboolean swap , GError** error ) 
{
   gint n = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );
   eh_require( data     );
   eh_require( n_rows>0 );
   eh_require( n_cols>0 );

   if ( data )
   {
      FILE* fp;
      GError* tmp_err = NULL;

      if ( file ) fp = eh_fopen_error( file , "w" , &tmp_err );
      else        fp = stdout;

      if ( fp )
      {
         gint i, j;

         if ( !delim ) delim = " ";

         if ( swap )
         {
            const gint top_row = n_rows-1;
            for ( j=0 ; j<n_cols ; j++ )
            {
               for ( i=0 ; i<top_row ; i++ )
                  n += fprintf( fp , "%f%s" , data[i][j] , delim );
               n += fprintf( fp , "%f" , data[i][j] );
               n += fprintf( fp , "\n" );
            }
         }
         else
         {
            const gint top_col = n_cols-1;
            for ( i=0 ; i<n_rows ; i++ )
            {
               for ( j=0 ; j<top_col ; j++ )
                  n += fprintf( fp , "%f%s" , data[i][j] , delim );
               n += fprintf( fp , "%f" , data[i][j] );
               n += fprintf( fp , "\n" );
            }
         }
      }
      else
         g_propagate_error( error , tmp_err );
   }

   return n;
}

gint
eh_dlm_print_dbl_grid( const gchar* file , const gchar* delim , Eh_dbl_grid g , GError** error ) 
{
   gint n = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );
   eh_require( g );

   if ( g )
   {
      GError* tmp_err = NULL;

      n = eh_dlm_print( file , delim , (double**)eh_dbl_grid_data(g) , eh_grid_n_x(g) , eh_grid_n_y(g) , &tmp_err );

      if ( tmp_err )
         g_propagate_error( error , tmp_err );
   }

   return n;
}

