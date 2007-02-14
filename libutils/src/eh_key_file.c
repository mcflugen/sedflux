#include "eh_key_file.h"
#include "utils.h"

CLASS( Eh_key_file )
{
   GHashTable* t; //< A hash table that holds all of the group lists
   GList*      l; //< A list of the groups in the order they appear in the file
};

GQuark
eh_key_file_error_quark( void )
{
   return g_quark_from_static_string( "eh-key-file-error-quark" );
}

/** \brief Destroy each symbol table in a list.

A helper function that destroys each Eh_symbol_table in a GList, as well as
the GList itself.  In this case, it is used to destroy a group of an Eh_key_file.

\param key         The key parameter of a GHashTable.  In this case, it is the name of group.
\param value       The value parameter of a GHashTable.  In this case, it is a pointer to a GList*
                   of Eh_symbol_table's.
\param user_data   Unused.

*/
void destroy_hash_table_list( gpointer key , gpointer value , gpointer user_data )
{
   GList* l;
   for ( l=value ; l ; l = l->next )
      eh_symbol_table_destroy( (Eh_symbol_table)(l->data) );
   g_list_free( value );
}

/** \brief Duplicate a key of a GHashTable and add it to a list.

A helper function to duplicate each key of a GHashTable and add it to
a list of all the keys.  The keys in the list are delimited by '\n'.

\param key         A pointer to GHashTable key
\param value       A pointer to GHashTable value
\param user_data   Location of the string containing the list of keys

*/
void dup_key( gpointer key , gpointer value , gpointer user_data )
{
   gchar** s = (gchar**)(user_data);
   gchar* new_str;

   if ( (*s)[0] == '\0' )
      new_str = g_strdup( key );
   else
      new_str = g_strjoin( "\n" , (*s) , key , NULL );

   eh_free( *s );
   (*s) = new_str;

}

/** \brief Construct an array of value-strings from a list of symbol tables

From a GList of Eh_symbol_tables, get the corresponding value for the
given key-string and put it in an array of strings.

\param l     The GList of Eh_symbol_tables's
\param key   The key-string specifying the value to get

\return An array of strings.  The array should be freed with g_strfreev.
*/
gchar** eh_key_file_list_to_array( GList* l , gpointer key )
{
   gchar** ans;
   guint len = g_list_length( l );
   guint i;

   ans = g_new( gchar* , len+1 );
   for ( i=0 ; l ; i++, l=l->next )
      ans[i] = g_strdup( eh_symbol_table_lookup(l->data,key) );
   ans[len] = NULL;

   return ans;
}

/** Add a group to a Eh_key_file

Add the group \p group_name to an Eh_key_file.  If \p replace is TRUE, and the \p group_name
is already present in the key-file, the symbol table of the present group is returned.
Otherwise, a new symbol table is created.

\param f           An Eh_key_file
\param group_name  The name of the group to add (or append)
\param replace     If TRUE, and the given group already exists, it is replaced.  Otherwise,
                   a new instance of the group is created.

\return The symbol table of the group
*/
Eh_symbol_table eh_key_file_add_group( Eh_key_file f ,
                                       const gchar* group_name ,
                                       gboolean replace )
{
   /* Find the group in the GHashTable */
   GList* group_list = g_hash_table_lookup( f->t , group_name );
   Eh_symbol_table group;

   /* If the group doesn't exist or it does but replace is off */
   if ( !group_list || (group_list&&!replace) )
   {
      Eh_symbol_table new_tab = eh_symbol_table_new();

      /* prepend a new symbol table to the list. */
      group_list = g_list_prepend(group_list,new_tab);

      /* add the group to the GHashTable of the key-file */
      g_hash_table_insert( f->t                   ,
                           g_strdup( group_name ) ,
                           group_list );

      /* Add the group name to the end of the list */
      f->l = g_list_append( f->l , g_strdup( group_name ));

      group = new_tab;
   }
   else
      group = g_list_first(group_list)->data;

   eh_require( group );

   /* The symbol table of the specified group. */
   return group;
}

/** 
\brief Add a key-value pair to a key-file group

A helper function to add a key-value pair to a Eh_key_file

\param key        The key-string to add
\param value      The value-string to add
\param user_data  Location of a the key-file and group name.
*/
void add_record_value( gpointer key , gpointer value , gpointer user_data )
{
   Eh_key_file f          = ((gpointer*)user_data)[0];
   gchar*      group_name = ((gpointer*)user_data)[1];
   eh_key_file_set_value( f , group_name , key , value );
}

/** Create a new Eh_key_file

\return The new Eh_key_file.  Use eh_key_file_destroy to free.

*/
Eh_key_file eh_key_file_new( )
{
   Eh_key_file f;

   NEW_OBJECT( Eh_key_file , f );

   f->t = g_hash_table_new_full( &g_str_hash                  ,
                                 &g_str_equal                 ,
                                 (GDestroyNotify)&eh_free_mem ,
                                 NULL );
   f->l = NULL;

   return f;
}

/** Destroy an Eh_key_file

Free all of the resources used by an Eh_key_file.

\return Returns NULL.
*/
Eh_key_file eh_key_file_destroy( Eh_key_file f )
{
   if ( f )
   {
      GList* link;

      g_hash_table_foreach( f->t , (GHFunc)&destroy_hash_table_list , NULL );
      g_hash_table_destroy( f->t );

      for ( link=f->l ; link ; link=link->next )
         eh_free( link->data );
      g_list_free( f->l );

      eh_free( f );
   }
   return NULL;
}

/** Query if a key-file has the given group.

\param f          An Eh_key_file
\param group_name The name of the group in question

\return Returns TRUE if the key-file contains the group.
*/
gboolean eh_key_file_has_group( Eh_key_file f , const gchar* group_name )
{
   if ( f && g_hash_table_lookup( f->t , group_name ) )
      return TRUE;
   else
      return FALSE;
}

/** Query if a key-file group has the given key.

\param f          An Eh_key_file
\param group_name The name of the group in question
\param key        The name of the key

\return Returns TRUE if the key-file group contains the key.
*/
gboolean eh_key_file_has_key( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   if ( f )
   {
      GList* group = g_hash_table_lookup( f->t , group_name );
      if ( group && eh_symbol_table_lookup( group->data , key ) )
         return TRUE;
      else
         return FALSE;
   }
   else
      return FALSE;
}

/** Get the names of all the groups in an Eh_key_file

\param f   An Eh_key_file

\return A (NULL-terminated) array of all the group names.  Use g_strfreev to free it.
*/
gchar** eh_key_file_get_groups( Eh_key_file f )
{
   gchar** groups = NULL;

   if ( f )
   {
      gchar* group_name_list = g_strdup( "\0");

      g_hash_table_foreach( f->t , &dup_key , &group_name_list );

      groups = g_strsplit( group_name_list , "\n" , 0 );

      eh_free( group_name_list );
   }

   return groups;
}

/** The number of instances of a group in an Eh_key_file

\param f   An Eh_key_file
\param group_name The name of a key-file group

\return The number of occurances of the group in an Eh_key_file
*/
gint eh_key_file_group_size( Eh_key_file f ,
                             const gchar* group_name )
{
   GList* group = g_hash_table_lookup( f->t , group_name );
   return g_list_length( group );
}

/** The number of groups in an Eh_key_file

The total number of groups including repeated occurances of
a group.

\param f   An Eh_key_file

\return The total number of groups.
*/
gint eh_key_file_size( Eh_key_file f )
{
   gint len = 0;
   if ( f )
   {
      len = g_list_length( f->l );
   }
   return len;
}

/** Get all of the keys present in a given group

Note that copies of the key-strings are made, so it is ok to free them
when they are no longer needed.

\param f            An Eh_key_file
\param group_name   The name of a key-file group

\return A NULL-terminated array of key-strings.  Use g_strfreev to free it.
*/
gchar** eh_key_file_get_keys( Eh_key_file f , const gchar* group_name )
{
   gchar** keys = NULL;

   if ( f )
   {
      GList* group = g_hash_table_lookup( f->t , group_name );

      if ( group )
      {
         Eh_symbol_table first_group = group->data;
         gchar* key_list = "";

         eh_symbol_table_foreach( first_group , &dup_key , &key_list );

         keys = g_strsplit( key_list , "\n" , -1 );

         eh_free( key_list );
      }
   }

   return keys;
}

/** Get all of the values associated with a key from a key-file group

Note that copies of the value-strings are made, so it is ok to free them
when they are no longer needed.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return A NULL-terminated array of value-strings.  Use g_strfreev to free it.
*/
gchar** eh_key_file_get_all_values( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   gchar** value = NULL;

   if ( f )
   {
      GList* group = g_hash_table_lookup( f->t , group_name );
      if ( group )
         value = eh_key_file_list_to_array( group , (gchar*)key );
   }

   return value;
}

/** Get the value associated with the first occurance of key in a key-file group

Note that a copy of the value-string is made, so it is ok to free it
when it is no longer needed.  If there is more than one occurance of the group in the
key-file, the value is obtained from the first occurance of the group.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return A newly-allocated string of the value.  Use eh_free to free it.
*/
gchar*
eh_key_file_get_value( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   gchar* value = NULL;

   if ( f )
   {
      GList* group = g_hash_table_lookup( f->t , group_name );
      if ( group )
         value = g_strdup( eh_symbol_table_lookup( group->data , (gchar*)key ) );
   }

   return value;
}

/** Get a string value from a key-file

Note that a copy of the value-string is made, so it is ok to free it
when it is no longer needed.  If there is more than one occurance of the group
in the key-file, the value is obtained from the first occurance of the group.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return A newly-allocated string of the value.  Use eh_free to free it.

*/
gchar*
eh_key_file_get_str_value( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   return eh_key_file_get_value( f , group_name , key );
}

/** Get an array of strings from a key-file

Note that copies of the value-strings are made, so it is ok to free them
when they are no longer needed.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return A NULL-terminated array of value-strings.  Use g_strfreev to free it.
*/
gchar**
eh_key_file_get_str_values( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   return eh_key_file_get_all_values(f,group_name,key);
}

/** Find a key in a key-file and convert its value to a gboolean

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return The value converted to a gboolean.
*/
gboolean eh_key_file_get_bool_value( Eh_key_file f           ,
                                     const gchar* group_name ,
                                     const gchar* key )
{
   gchar*   str = eh_key_file_get_value(f,group_name,key);
   gboolean ans = strtobool( str );

   eh_free( str );

   return ans;
}

/** Find all values of a key in a key-file and convert them to gboolean

\note Use eh_key_file_group_size to get the length of the array.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return Array of values converted to a gboolean.  Use eh_free to free.
*/
gboolean* eh_key_file_get_bool_values( Eh_key_file f           ,
                                       const gchar* group_name ,
                                       const gchar* key )
{
   guint     len = eh_key_file_group_size(f,group_name);
   gchar**   str = eh_key_file_get_all_values(f,group_name,key);
   gboolean* ans = eh_new( gboolean , len );
   guint i;

   for ( i=0 ; i<len ; i++ )
      ans[i] = strtobool( str[i] );

   g_strfreev( str );

   return ans;
}

/** Find a key in a key-file and convert its value to a double

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return The value converted to a double.
*/
double eh_key_file_get_dbl_value( Eh_key_file f ,
                                  const gchar* group_name ,
                                  const gchar* key )
{
   double ans;
   gchar* str = eh_key_file_get_value(f,group_name,key);

   if ( str )
      ans = g_strtod( str , NULL );
   else
      ans = eh_nan();

   eh_free( str );

   return ans;
}

/** Find a key in a key-file and convert its value to a double array

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group
\param len           Location to store the array length

\return The value converted to a double array.
*/
double*
eh_key_file_get_dbl_array( Eh_key_file f           ,
                           const gchar* group_name ,
                           const gchar* key        ,
                           gssize* len )
{
   double* d_array = NULL;

   eh_require( key        );
   eh_require( group_name );
   eh_require( len        );

   if ( f )
   {
      gchar*  str       = eh_key_file_get_value( f , group_name , key );
      gchar** str_array = g_strsplit_set( str , ",;" , -1 );

      *len = g_strv_length( str_array );

      if ( *len > 0 )
      {
         gint i;
         d_array = eh_new( double , *len );
         for ( i=0 ; i<*len ; i++ )
            d_array[i] = g_strtod( str_array[i] , NULL );
      }

      eh_free( str );
      g_strfreev( str_array );
   }

   return d_array;
}

/** Find all values of a key in a key-file and convert them to double

\note Use eh_key_file_group_size to get the length of the array.

\param f             An Eh_key_file
\param group_name    The name of a key-file group
\param key           The name of a key within the group

\return Array of values converted to a double.  Use eh_free to free.
*/
double* eh_key_file_get_dbl_values( Eh_key_file f           ,
                                    const gchar* group_name ,
                                    const gchar* key )
{
   guint   len = eh_key_file_group_size(f,group_name);
   gchar** str = eh_key_file_get_all_values(f,group_name,key);
   double* ans = eh_new( double , len );
   guint i;

   for ( i=0 ; i<len ; i++ )
      ans[i] = g_strtod( str[i] , NULL );

   g_strfreev( str );

   return ans;
}

/** Set a value in a key-file.

Set the value of a key within a group of a key-file.  If the group, \p group_name
does not already exist, then a new group is created and the key-value pair added.
If the specified key is already present within the group, a new occurance of the
group is created and the key-value pair added to it.

\note Both the \p key and \p value strings are duplicated, so it is fine to free
them after use.

\param f          An Eh_key_file
\param group_name The name of a key-file group
\param key        The name of the key whose value to set
\param value      A string containing the new value
*/
void eh_key_file_set_value( Eh_key_file f           ,
                            const gchar* group_name ,
                            const gchar* key        ,
                            const gchar* value )
{
   eh_require( f );

   {
      Eh_symbol_table group;

      /* If the group doesn't exist, or the key is alreay present, add a new group */
      if ( !eh_key_file_has_group( f , group_name )
           || eh_key_file_has_key( f , group_name , key ) )
         group = eh_key_file_add_group( f , group_name , FALSE );
      else
         group = g_list_first(g_hash_table_lookup( f->t , group_name ))->data;
      /* We are now sure that the key is not present in this table. */

      eh_require( group );

      /* We duplicate the strings and add them to the table */
      eh_symbol_table_insert( group , g_strdup( key ) , g_strdup( value ) );
   }
}

/** Reset a value in a key-file

This is similar to eh_key_file_set_value, only if the key is already present
in the given group, it's value is set to the new value.  If the group is not
present, then a new group is created.

\note Both the \p key and \p value strings are duplicated, so it is fine to free
them after use.

\param f          An Eh_key_file
\param group_name The name of a key-file group
\param key        The name of the key whose value to set
\param value      A string containing the new value

*/
void eh_key_file_reset_value( Eh_key_file f           ,
                              const gchar* group_name ,
                              const gchar* key        ,
                              const gchar* value )
{
   Eh_symbol_table group;

   eh_require( f );

   group = eh_key_file_add_group( f , group_name , TRUE );

   eh_symbol_table_insert( group , g_strdup( key ) , g_strdup( value ) );
}


/** Construct a Eh_symbol_table of key-value pairs for a given group

A new symbol table is created that contains all of the key-value pairs of
the given group.  If the key-file contains more than one occurance of the
group, only the first occurance is used.

\param f          An Eh_key_file
\param group_name The name of a key-file group

\return A new Eh_symbol_table of key-value pairs.  Use eh_symbol_table_destroy to free.
*/
Eh_symbol_table eh_key_file_get_symbol_table( Eh_key_file f ,
                                              const gchar* group_name )
{
   GList* group = g_hash_table_lookup( f->t , group_name );
   return eh_symbol_table_dup( group->data );
}

/** Construct Eh_symbol_table's of key-value pairs for all occurances of a group

A new symbol table of key-value pairs is created for each occurance of the group and
placed into a NULL-terminated array.  Each symbol table can be safely destroyed 
using eh_symbol_table_destroy.

\param f          An Eh_key_file
\param group_name The name of a key-file group

\return A NULL-terminated array of newly-allocated Eh_symbol_table's of key-value pairs.
        Use eh_symbol_table_destroy to free each element of the array.
*/
Eh_symbol_table* eh_key_file_get_symbol_tables( Eh_key_file f , 
                                                const gchar* group_name )
{
   Eh_symbol_table* value = NULL;

   if ( f )
   {
      GList* group = g_hash_table_lookup( f->t , group_name );
      if ( group )
      {
         GList* l;
         guint i;
         guint len = eh_key_file_group_size( f , group_name );

         value = g_new( Eh_symbol_table , len+1 );
         for ( i=0,l=group ; l ; i++, l=l->next )
            value[i] = eh_symbol_table_dup( l->data );
         value[len] = NULL;
      }
   }
   return value;
}

/** \brief Print the name of a key

\param key        A key-string to print
\param value      Not used
\param user_data  Not used
*/
void print_keys( gpointer key , gpointer value , gpointer user_data )
{
   eh_message( "KEY = %s" , (gchar*)key );
}

/** Scan a key-file

Scan an entire key-file and construct a new Eh_key_file containing
the file information.

\param file   The name of the file to scan

\return A new Eh_key_file.  Use eh_key_file_destroy to free.
*/
Eh_key_file
eh_key_file_scan( const char* file , GError** error )
{
   Eh_key_file f = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   f = eh_key_file_new();

   if ( f )
   {
      GError*   tmp_err = NULL;
      GScanner* s;

      s = eh_open_scanner( file , &tmp_err );

      if ( s )
      {
         gboolean        done = FALSE;
         gchar*          group_name;
         Eh_symbol_table symbol_table;
         gpointer        user_data[2];

         while( !done && !g_scanner_eof(s) )
         {
            symbol_table = eh_symbol_table_new();
            group_name = eh_scan_next_record( s , symbol_table );

            if ( group_name )
            {
               user_data[0] = f;
               user_data[1] = group_name;

               eh_symbol_table_foreach( symbol_table , &add_record_value , user_data );
            }
            else
               done = TRUE;

            eh_symbol_table_destroy( symbol_table );
            eh_free( group_name );
         }

         eh_close_scanner( s );
      }
      else
      {
         eh_key_file_destroy( f );
         g_propagate_error( error , tmp_err );
      }
   }

   return f;
}

gint
eh_key_file_scan_from_template( const gchar* file       ,
                                const gchar* group_name ,
                                Eh_key_file_entry t[]   ,
                                GError** error )
{
   gint n_entries = 0;

   eh_return_val_if_fail( error==NULL || *error==NULL , 0 );

   if ( file )
   {
      GError*     tmp_error       = NULL;
      Eh_key_file f               = NULL;
      gchar**     missing_entries = NULL;
      gint*       len;
      gint        i;

      for ( n_entries=0 ; t[n_entries].label ; n_entries++ );

      for ( i=0 ; i<n_entries ; i++ )
         if ( t[i].arg == EH_ARG_DARRAY )
         {
            eh_require( t[i].arg_data_len );
            eh_return_val_if_fail( t[i].arg_data_len , 0 );

            *(t[i].arg_data_len) = 0;
         }

      len = eh_new( gint , n_entries );

      f = eh_key_file_scan( file , &tmp_error );

      for ( i=0 ; i<n_entries && !tmp_error ; i++ )
      {
         if ( eh_key_file_has_key( f , group_name , t[i].label ) )
         {
            switch ( t[i].arg )
            {
               case EH_ARG_DBL:
                  *(double* )(t[i].arg_data) = eh_key_file_get_dbl_value( f , group_name , t[i].label );
                  break;
               case EH_ARG_DARRAY:
                  *(double**)(t[i].arg_data) = eh_key_file_get_dbl_array( f , group_name , t[i].label , &(len[i]) );

                  if ( *(t[i].arg_data_len)==0 || *(t[i].arg_data_len)==len[i] )
                     *(t[i].arg_data_len) = len[i];
                  else
                     g_set_error( &tmp_error ,
                                  EH_KEY_FILE_ERROR ,
                                  EH_KEY_FILE_ERROR_ARRAY_LEN_MISMATCH ,
                                  "%s: Array length mismatch (%d!=%d): %s\n" ,
                                  file , len[i] , t[i].arg_data_len , t[i].label );

                  break;
               case EH_ARG_FILENAME:
                  *(gchar**)(t[i].arg_data)  = eh_key_file_get_value( f , group_name , t[i].label );
                  break;
            }
         }
         else
            eh_strv_append( &missing_entries , g_strconcat(file , ": Missing required entry: " , t[i].label , NULL ) );

      }

      if ( !tmp_error && missing_entries )
      {
         gchar* missing_list = g_strjoinv( "\n" , missing_entries );

         g_set_error( &tmp_error ,
                      EH_KEY_FILE_ERROR ,
                      EH_KEY_FILE_ERROR_MISSING_ENTRY ,
                      "%s\n" , missing_list );

         eh_free( missing_list );
      }

      if ( tmp_error )
         g_propagate_error( error , tmp_error );

      g_strfreev( missing_entries );
      eh_free( len );

      eh_key_file_destroy( f );
   }

   return n_entries;
}

gssize
eh_key_file_fprint_template( FILE* fp                ,
                             const gchar* group_name ,
                             Eh_key_file_entry entry[] )
{
   gint n = 0;

   if ( fp )
   {
      gint        n_entries;
      gint        max_len = 0;
      gchar*      pad;
      gint        i;

      for ( n_entries=0 ; entry[n_entries].label ; n_entries++ );

      for ( i=0 ; i<n_entries ; i++ )
         if ( strlen( entry[i].label ) > max_len )
            max_len = strlen( entry[i].label );

      pad = g_strnfill( max_len , ' ' );

      fprintf( fp , "[ %s ]" , group_name );

      for ( i=0 ; i<n_entries ; i++ )
      {
         n += fprintf( fp , "%s :%s" , entry[i].label , pad + strlen( entry[i].label ) );

         switch ( entry[i].arg )
         {
            case EH_ARG_DBL:
               n += fprintf( fp , "%s\n" , "<double-scalar>" );
               break;
            case EH_ARG_DARRAY:
               n += fprintf( fp , "%s\n" , "<double-array>" );
               break;
            case EH_ARG_FILENAME:
               n += fprintf( fp , "%s\n" , "<filename>" );
               break;
         }
      }

      eh_free( pad );
   }

   return n;
}

/** Scan a file for a particular key-file group

Scan a key-file for a particular group and place the key-value pairs found in
this group into a Eh_symbol_table.  If the input symbol table, \p tab is
NULL, a new symbol table is constructed.  Only the first occurance of the
group is scanned.

\param file     The name of the file to scan
\param name     The name of the group to scan
\param tab      A symbol table to place the key-value pairs into.  If NULL, a
                new table will be created.

\return         An Eh_symbol_table containing the key-value pairs of the group, or NULL if the
                file does not contain the specified group.
*/
Eh_symbol_table
eh_key_file_scan_for( const gchar* file ,
                      const gchar* name ,
                      Eh_symbol_table tab ,
                      GError** error )
{
   Eh_symbol_table new_tab = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   //---
   // Open the key-file and scan in all of the entries.
   //
   // Add each key-value pair (of the specified group) to the symbol table.
   //---
   {
      GError*     tmp_err  = NULL;
      Eh_key_file key_file = eh_key_file_scan( file , &tmp_err );
   
      if ( key_file )
      {
         if ( eh_key_file_has_group( key_file , name ) )
            new_tab = eh_key_file_get_symbol_table( key_file , name );
         else
            new_tab = NULL;
      }
      else
         g_propagate_error( error , tmp_err );

      eh_key_file_destroy( key_file );
   }

   if ( tab && new_tab )
   {
      eh_symbol_table_copy   ( tab , new_tab );
      eh_symbol_table_destroy( new_tab );

      new_tab = tab;
   }

   return new_tab;
}

/** Pop the next group of a key-file

Construct a symbol table of key-value pairs from the next group in a
key-file.  The group is removed from the Eh_key_file.  The groups are
ordered in the same way as they are in the original file that was scanned.

\param f   An Eh_key_file

\return An Eh_symbol_table of key-value pairs from the next group in a key-file
*/
Eh_symbol_table eh_key_file_pop_group( Eh_key_file f )
{
   Eh_symbol_table new_table = NULL;

   if ( f && f->l )
   {
      gchar* group_name = g_list_first(f->l)->data;
      GList* group_list = g_hash_table_lookup( f->t , group_name );

      eh_require( group_list );

      /* Pop the next group in the list */
      new_table = g_list_first( group_list )->data;

      /* Delete the link to this group */
      group_list = g_list_delete_link( group_list , group_list );

      if ( group_list )
         /* If the list isn't empty, insert the new start of the list into the hash table */
         g_hash_table_replace( f->t , g_strdup(group_name) , group_list );
      else
         /* Otherwise, remove the group from the hash table */
         g_hash_table_remove( f->t , group_name );

      /* Delete the first link in the list and free the data */
      eh_free( group_name );
      f->l = g_list_delete_link( f->l , f->l );
   }

   return new_table;
}

