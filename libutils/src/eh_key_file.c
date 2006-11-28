#include "eh_key_file.h"
#include "utils.h"

CLASS( Eh_key_file )
{
   GHashTable* t;
};

void destroy_hash_table_list( gpointer key , gpointer value , gpointer user_data )
{
   GList* l;
   for ( l=value ; l ; l = l->next )
      eh_symbol_table_destroy( (Eh_symbol_table)(l->data) );
   g_list_free( value );
}

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

Eh_symbol_table eh_key_file_add_group( Eh_key_file f ,
                                       const gchar* group_name ,
                                       gboolean replace )
{
   GList* group_list = g_hash_table_lookup( f->t , group_name );
   Eh_symbol_table group;

   if ( !group_list || (group_list&&!replace) )
   {
      Eh_symbol_table new_tab = eh_symbol_table_new();

      group_list = g_list_prepend(group_list,new_tab);
      g_hash_table_insert( f->t                   ,
                           g_strdup( group_name ) ,
                           group_list );
      group = new_tab;
   }
   else
      group = g_list_first(group_list)->data;

   eh_require( group );

   return group;
}

void add_record_value( gpointer key , gpointer value , gpointer user_data )
{
   Eh_key_file f          = ((gpointer*)user_data)[0];
   gchar*      group_name = ((gpointer*)user_data)[1];
   eh_key_file_set_value( f , group_name , key , value );
}

Eh_key_file eh_key_file_new( )
{
   Eh_key_file f;

   NEW_OBJECT( Eh_key_file , f );

   f->t = g_hash_table_new_full( &g_str_hash                  ,
                                 &g_str_equal                 ,
                                 (GDestroyNotify)&eh_free_mem ,
                                 NULL );
   return f;
}

Eh_key_file eh_key_file_destroy( Eh_key_file f )
{
   g_hash_table_foreach( f->t , (GHFunc)&destroy_hash_table_list , NULL );
   g_hash_table_destroy( f->t );
   eh_free( f );
   return NULL;
}

gboolean eh_key_file_has_group( Eh_key_file f , const gchar* group_name )
{
   if ( f && g_hash_table_lookup( f->t , group_name ) )
      return TRUE;
   else
      return FALSE;
}

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

guint eh_key_file_group_size( Eh_key_file f ,
                              const gchar* group_name )
{
   GList* group = g_hash_table_lookup( f->t , group_name );
   return g_list_length( group );
}

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

gchar* eh_key_file_get_value( Eh_key_file f , const gchar* group_name , const gchar* key )
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

gchar*
eh_key_file_get_str_value( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   return eh_key_file_get_value( f , group_name , key );
}

gchar**
eh_key_file_get_str_values( Eh_key_file f , const gchar* group_name , const gchar* key )
{
   return eh_key_file_get_all_values(f,group_name,key);
}

gboolean eh_key_file_get_bool_value( Eh_key_file f           ,
                                     const gchar* group_name ,
                                     const gchar* key )
{
   gchar*   str = eh_key_file_get_value(f,group_name,key);
   gboolean ans = strtobool( str );

   eh_free( str );

   return ans;
}

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

double eh_key_file_get_dbl_value( Eh_key_file f ,
                                  const gchar* group_name ,
                                  const gchar* key )
{
   gchar* str = eh_key_file_get_value(f,group_name,key);
   double ans = g_strtod( str , NULL );

   eh_free( str );

   return ans;
}

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

void eh_key_file_set_value( Eh_key_file f           ,
                            const gchar* group_name ,
                            const gchar* key        ,
                            const gchar* value )
{
   eh_require( f );

   {
      Eh_symbol_table group;
      if ( !eh_key_file_has_group( f , group_name )
           || eh_key_file_has_key( f , group_name , key ) )
         group = eh_key_file_add_group( f , group_name , FALSE );
      else
         group = g_list_first(g_hash_table_lookup( f->t , group_name ))->data;

      eh_require( group );

      eh_symbol_table_insert( group , g_strdup( key ) , g_strdup( value ) );
   }
}

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

Eh_symbol_table eh_key_file_get_symbol_table( Eh_key_file f ,
                                              const gchar* group_name )
{
   GList* group = g_hash_table_lookup( f->t , group_name );
   return eh_symbol_table_dup( group->data );
}

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

void print_keys( gpointer key , gpointer value , gpointer user_data )
{
   eh_message( "KEY = %s" , (gchar*)key );
}

Eh_key_file eh_key_file_scan( const char* file )
{
   Eh_key_file f = eh_key_file_new();

   if ( f )
   {
      GScanner* s;
      gchar* group_name;
      gboolean done = FALSE;
      Eh_symbol_table symbol_table;
      gpointer user_data[2];

      s = eh_open_scanner( file );

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
   return f;
}

Eh_symbol_table eh_key_file_scan_for( const gchar* file ,
                                      const gchar* name ,
                                      Eh_symbol_table tab )
{
   Eh_symbol_table new_tab;

   //---
   // Open the key-file and scan in all of the entries.
   //
   // Add each key-value pair (of the specified group) to the symbol table.
   //---
   {
      Eh_key_file key_file = eh_key_file_scan( file );
   
      if ( eh_key_file_has_group( key_file , name ) )
         new_tab = eh_key_file_get_symbol_table( key_file , name );
      else
         new_tab = NULL;

      eh_key_file_destroy( key_file );
   }

   if ( tab && new_tab )
   {
      eh_symbol_table_copy( tab , new_tab );
      eh_symbol_table_destroy( new_tab );
   }

   return new_tab;
}

