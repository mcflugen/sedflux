#include <eh_utils.h>
#include <strings.h>
#include <errno.h>

GQuark
eh_str_error_quark( void )
{
   return g_quark_from_static_string( "eh-str-error-quark" );
}

double**
eh_str_to_time_range_piecewise( const gchar* s , GError** error )
{
   double** range = NULL;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( s )
   {
      gchar** pieces = g_strsplit( s , ";" , 0 );
      GError* tmp_e  = NULL;
      gchar** p;
      double* this_range;
      double  last_top = -G_MAXDOUBLE;

      for ( p=pieces ; *p && !tmp_e ; p++ )
      {
         this_range = eh_str_to_time_range( *p , &tmp_e );

         if ( !tmp_e )
         {
            eh_strv_append( (gchar***)&range , (gchar*)this_range );

            if ( this_range[1] < last_top )
            {
               g_set_error( &tmp_e ,
                            EH_STR_ERROR ,
                            EH_STR_ERROR_RANGE_OVERLAP ,
                            "Overlapping ranges" );
            }

            last_top = this_range[1];
         }
      }

      g_strfreev( pieces );

      if ( tmp_e )
      {
         g_propagate_error( error , tmp_e );
         g_strfreev( (gchar**)range );
         range = NULL;
      }
   }

   return range;
}

double**
eh_str_to_dbl_range_piecewise( const gchar* s , GError** error )
{
   double** range = NULL;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( s )
   {
      gchar** pieces = g_strsplit( s , ";" , 0 );
      GError* tmp_e  = NULL;
      gchar** p;
      double* this_range;
      double  last_top = -G_MAXDOUBLE;

      for ( p=pieces ; *p && !tmp_e ; p++ )
      {
         this_range = eh_str_to_dbl_range( *p , &tmp_e );

         if ( !tmp_e )
         {
            eh_strv_append( (gchar***)&range , (gchar*)this_range );

            if ( this_range[1] < last_top )
            {
               g_set_error( &tmp_e ,
                            EH_STR_ERROR ,
                            EH_STR_ERROR_RANGE_OVERLAP ,
                            "Overlapping ranges" );
            }

            last_top = this_range[1];
         }
      }

      g_strfreev( pieces );

      if ( tmp_e )
      {
         g_propagate_error( error , tmp_e );
         g_strfreev( (gchar**)range );
         range = NULL;
      }
   }

   return range;
}

double*
eh_str_to_dbl_range( const gchar* s , GError** error )
{
   double* range = NULL;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( s )
   {
      gchar** end_s = g_strsplit( s , "->" , 0 );
      gint    len   = g_strv_length( end_s );
      GError* tmp_e = NULL;

      if ( len>=2 )
      {
         range    = eh_new( double , 2 );

         range[0] = eh_str_to_dbl( end_s[0    ] , &tmp_e );
         range[1] = eh_str_to_dbl( end_s[len-1] , &tmp_e );

         if ( !tmp_e && range[0] > range[1] )
         {
            g_set_error( &tmp_e ,
                         EH_STR_ERROR ,
                         EH_STR_ERROR_BAD_RANGE ,
                         "Upper bound not greater than lower bound" );
         }

         if ( tmp_e )
            eh_free( range );
      }
      else
      {
         g_set_error( &tmp_e ,
                      EH_STR_ERROR ,
                      EH_STR_ERROR_NO_RANGE ,
                      "Range specifier ('->') not found" );
      }
      
      g_strfreev( end_s );

      if ( tmp_e )
      {
         g_propagate_error( error , tmp_e );
         range = NULL;
      }
   }

   return range;
}

double*
eh_str_to_time_range( const gchar* s , GError** error )
{
   double* range = NULL;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( s )
   {
      gchar** end_s = g_strsplit( s , "->" , 0 );
      gint    len   = g_strv_length( end_s );
      GError* tmp_e = NULL;

      if ( len>=2 )
      {
         range    = eh_new( double , 2 );

         if ( !tmp_e ) range[0] = eh_str_to_time_in_years( end_s[0    ] , &tmp_e );
         if ( !tmp_e ) range[1] = eh_str_to_time_in_years( end_s[len-1] , &tmp_e );
         if ( !tmp_e && range[0] > range[1] )
         {
            g_set_error( &tmp_e ,
                         EH_STR_ERROR ,
                         EH_STR_ERROR_BAD_RANGE ,
                         "Upper bound not greater than lower bound" );
         }

         if ( tmp_e )
            eh_free( range );
      }
      else
      {
         g_set_error( &tmp_e ,
                      EH_STR_ERROR ,
                      EH_STR_ERROR_NO_RANGE ,
                      "Range specifier ('->') not found" );
      }
      
      g_strfreev( end_s );

      if ( tmp_e )
      {
         g_propagate_error( error , tmp_e );
         range = NULL;
      }
   }

   return range;
}

double
eh_str_to_dbl( const gchar* s , GError** error )
{
   double dbl_val;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , eh_nan() );

   if ( s )
   {
      GError* tmp_e = NULL;
      gchar*  p;

      dbl_val = g_strtod( s , &p );

      if ( p==s )
      {
         g_set_error( &tmp_e ,
                      EH_STR_ERROR ,
                      EH_STR_ERROR_BAD_STRING ,
                      "Failed to convert string to double: %s" , g_strerror( errno ) );
         g_propagate_error( error , tmp_e );
         dbl_val = eh_nan();
      }
   }

   return dbl_val;
}

gint64
eh_str_to_int( const gchar* s , GError** error )
{
   gint64 int_val;

   eh_require (s);
   eh_return_val_if_fail (error==NULL || *error==NULL, G_MININT64);

   if ( s )
   {
      GError* tmp_e = NULL;
      gchar*  p;

      int_val = g_ascii_strtoull( s , &p , 10 );

      if (      p==s 
           || ( errno==ERANGE && ( int_val==G_MAXINT64 || int_val==G_MININT64 ) )
           || ( errno==EINVAL &&   int_val==0 ) )
      {
         g_set_error( &tmp_e ,
                      EH_STR_ERROR ,
                      EH_STR_ERROR_BAD_STRING ,
                      "Failed to convert string to int: %s" , g_strerror( errno ) );
         g_propagate_error( error , tmp_e );
         int_val = G_MININT;
      }
   }

   return int_val;
}

double
eh_str_to_time_in_years( const gchar* s , GError** error )
{
   double t;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , eh_nan() );

   if ( s )
   {
      GError*   tmp_e  = NULL;
      char      unit_c = 0;
      GString*  str    = g_string_new(s);
      Eh_date_t time   = {0,0,0};
      int       n;
      double    val;

      eh_string_remove_white_space( str );

      while (    !tmp_e
              && ( n=sscanf(eh_string_c_str(str),"%lf%c",&val,&unit_c) ) > 0 )
      {
         /* It's an error if both values are not scanned. */
         if ( n == 2 )
         {
            switch ( unit_c )
            {
               case 'd': time.day   = val; break;
               case 'm': time.month = val; break;
               case 'y': time.year  = val; break;

               default:
                  g_set_error( &tmp_e ,
                               EH_STR_ERROR ,
                               EH_STR_ERROR_BAD_UNIT ,
                               "Invalid unit: %c" , unit_c );
            }
         }
         else
         {
            g_set_error( &tmp_e ,
                         EH_STR_ERROR ,
                         EH_STR_ERROR_NO_UNIT ,
                         "Missing time unit [dmy]" );
         }

         g_string_erase(str,0,eh_string_find_first_of(str,unit_c)+1);
      }

      if ( !tmp_e )
         t = eh_date_to_years( &time );
      else
      {
         t = eh_nan();
         g_propagate_error( error , tmp_e );
      }

      g_string_free( str , TRUE );
   }

   return t;
}

gboolean
eh_str_is_boolean( const gchar* s )
{
   gboolean is_boolean = FALSE;

   eh_require( s );

   if ( s )
   {
      GError* error = NULL;

      eh_str_to_boolean( s , &error );

      if ( error )
      {
         is_boolean = FALSE;
         g_error_free( error );
      }
      else
         is_boolean = TRUE;
   }

   return is_boolean;
}

gboolean
eh_str_to_boolean( const gchar* s , GError** error )
{
   gboolean val = FALSE;

   eh_require( s );
   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   if ( s )
   {
      GError* tmp_e = NULL;
      gchar*  str   = g_strstrip( g_strdup( s ) );

      if      ( g_ascii_strcasecmp(str,"YES"  ) == 0 ||
                g_ascii_strcasecmp(str,"ON"   ) == 0 ||
                g_ascii_strcasecmp(str,"TRUE" ) == 0 ||
                g_ascii_strcasecmp(str,"OUI"  ) == 0 )
         val = TRUE;
      else if ( g_ascii_strcasecmp(str,"NO"   ) == 0 ||
                g_ascii_strcasecmp(str,"OFF"  ) == 0 ||
                g_ascii_strcasecmp(str,"FALSE") == 0 ||
                g_ascii_strcasecmp(str,"NON"  ) == 0 )
         val = FALSE;
      else
      {
         g_set_error( &tmp_e ,
                      EH_STR_ERROR ,
                      EH_STR_ERROR_BAD_LOGICAL_VAL ,
                      "Invalid logical value: %s" , str );
         g_propagate_error( error , tmp_e );
         val = FALSE;
      }

      eh_free( str );
   }

   return val;
}

/** Append a string to the end of an array of strings.

If str_l points to NULL, a new string array will be allocated.
The location of \p new_str is appended to \p str_l rather than
a copy.  Thus, \p new_str should not be freed before \p str_l.

\param str_l   The location of a string array, or NULL.
\param new_str The string to append.

\return The input string_array.
*/
gchar**
eh_strv_append( gchar*** str_l , gchar* new_str )
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

gchar**
eh_strv_concat( gchar*** str_l , gchar** new_l )
{
   if ( str_l && new_l )
   {
      gchar** s;

      for ( s=new_l ; *s ; s++ )
         *str_l = eh_strv_append( str_l , *s );
   }

   return *str_l;
}

gint
eh_strv_find( const gchar** str_l , const gchar* needle )
{
   gint i = -1;

   eh_require( str_l );

   if ( str_l && needle )
   {
      gchar** s;
      gint    n;
      for ( s=(gchar**)str_l,n=0 ; *s && i<0 ; s++,n++ )
         if ( g_ascii_strcasecmp( *s , needle )==0 )
            i = n;
   }

   return i;
}

gchar*
eh_str_replace( gchar* str , gchar old_c , gchar new_c )
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

gchar*
eh_str_remove( gchar* str , gchar* start , gint n )
{
   if ( str )
   {
      gchar* tail = start+n;
      g_memmove( start , tail , strlen(tail)+1 );
   }
   return str;
}

gchar*
eh_str_remove_blocks( gchar* str , gchar** block_start , gchar** block_end )
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
eh_str_parse_key_value (gchar* str, const gchar* delim_1, const gchar* delim_2)
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
eh_str_remove_to_eol_comments( gchar* str , const gchar* com_start )
{
   return eh_str_remove_comments( str , com_start , NULL , NULL );
}

/** Remove c-style comments from a string

Remove comments that begin with '\/\*' and end with '\*\/'
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

#include <string.h>

#define WHITE_SPACE " \t\n"

gchar*
eh_str_trim_left( gchar *str )
{
   char *ptr;

//   ptr = (char*)malloc1D(sizeof(char)*(strlen(str)+1));
   ptr = eh_new( char , strlen(str)+1 );
   strcpy(ptr,str);
   while (strchr(WHITE_SPACE,ptr[0]) != 0 && *ptr != '\0')
      ptr++;
   strcpy(str,ptr);
   return str;
}

gchar*
eh_str_trim_right( gchar *str )
{
   char *ptr;
   int len;

//   ptr = (char*)malloc1D(sizeof(char)*(strlen(str)+1));
   ptr = eh_new( char , strlen(str)+1 );
   strcpy(ptr,str);
   len = strlen(ptr);
   while ( strchr(WHITE_SPACE,ptr[len-1])!=0 && len>0 )
   {
      ptr[len-1]='\0';
      len--;
   }
   strcpy(str,ptr);
   return str;
}

gchar*
eh_str_remove_white_space( gchar *str )
{
   int i, j;
   char *str_temp;
// Add one for the terminating null character.
   str_temp = eh_new( char , strlen(str)+1 );
   strcpy(str_temp,str);
   for (i=0,j=0;i<strlen(str_temp);i++)
      if ( strchr(WHITE_SPACE,str_temp[i]) == NULL )
      {
         str[j] = str_temp[i];
         j++;
      }
   str[j] = '\0';
   eh_free(str_temp);
   return str;
}

