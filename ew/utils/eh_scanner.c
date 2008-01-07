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

#include <eh_utils.h>

#if defined( OLD_SYMBOL_TABLE )

guint eh_str_case_hash(gconstpointer key);
gboolean eh_str_case_equal(gconstpointer a, gconstpointer b);

void eh_symbol_table_free_label( gpointer label )
{
   eh_free( label );
}

Symbol_table *eh_create_symbol_table(void)
{
   return (Symbol_table*)g_hash_table_new_full( &eh_str_case_hash           ,
                                                &eh_str_case_equal          ,
                                                &eh_symbol_table_free_label ,
                                                &eh_symbol_table_free_label );
}

guint eh_str_case_hash(gconstpointer key)
{
   char *new_key = g_strdup((char*)key);
   guint ans;

   g_strup(new_key);
   ans = g_str_hash(new_key);
   eh_free(new_key);

   return ans;
}

void eh_destroy_key( gpointer key , gpointer value , gpointer user_data)
{
   eh_free(key);
}

//#include <varargs.h>
#include <stdarg.h>

void __eh_symbol_table_insert( char *key, char *value, Symbol_table *t )
{
   eh_symbol_table_insert( t , g_strdup(key) , g_strdup(value) );
}

Symbol_table* eh_symbol_table_dup( Symbol_table* t )
{
   return eh_symbol_table_copy( NULL , t );
}

Symbol_table* eh_symbol_table_copy( Symbol_table* dest , Symbol_table* src )
{
   if ( !dest )
      dest = eh_create_symbol_table();
   
   g_hash_table_foreach( src , (GHFunc)&__eh_symbol_table_insert , dest );

   return dest;
}

Symbol_table *eh_merge_symbol_table( Symbol_table* table1 , ... )
{
   va_list args;
   Symbol_table *rtn, *table;

   rtn = eh_create_symbol_table();
   g_hash_table_foreach( table1 , (GHFunc)&__eh_symbol_table_insert , rtn );

   va_start( args , table1 );
   table = va_arg( args , Symbol_table* );
   while ( table )
   {
      g_hash_table_foreach( table , (GHFunc)&__eh_symbol_table_insert , rtn );
      table = va_arg( args , Symbol_table* );
   }
   va_end( args );
   return rtn;
}

gboolean eh_str_case_equal(gconstpointer a, gconstpointer b)
{
   return (g_strcasecmp((char*)a,(char*)b)==0)?TRUE:FALSE;
}

void eh_symbol_table_insert(Symbol_table *t, char *key, char *value)
{
   g_hash_table_insert((GHashTable*)t,g_strdup(key),g_strdup(value));
}

void eh_symbol_table_replace( Symbol_table* t , char* key , char* value )
{
   g_hash_table_replace((GHashTable*)t,g_strdup(key),g_strdup(value));
}

char*
eh_symbol_table_lookup(Symbol_table *t, char *key)
{
   return (char*)g_hash_table_lookup((GHashTable*)t,key);
/*
   char *value = (char*)g_hash_table_lookup((GHashTable*)t,key);
   if ( value )
      return value;
   else
   {
      eh_info( "eh_symbol_table_lookup : unable to find key : %s\n" , key );
      eh_exit(-1);
   }
   eh_require_not_reached();
   return NULL;
*/
}

typedef struct
{
   FILE *fp;
   int max_key_len;
}
aligned_st;
void eh_print_symbol_aligned( char *key, char *value, aligned_st *st );
void eh_get_max_key_len( char *key , char *value , int *max_len );

void eh_print_symbol(char *key, char *value, FILE *fp)
{
   fprintf(fp,"%s : %s\n",key,value);
}

void eh_print_symbol_aligned( char *key, char *value, aligned_st *st )
{
   int len = st->max_key_len-strlen(key)+1;
   char *fill = g_strnfill( len , ' ' );
   fprintf(st->fp,"%s%s: %s\n",key,fill,value);
   eh_free(fill);
}

void eh_get_max_key_len( char *key , char *value , int *max_len )
{
   int len = strlen(key);
   if ( len > *max_len )
      *max_len = len;
}

void eh_print_symbol_table( Symbol_table *t , FILE *fp )
{
   g_hash_table_foreach( t , (GHFunc)&eh_print_symbol , fp );
}

void eh_print_symbol_table_aligned( Symbol_table *t , FILE *fp )
{
   int max_len=0;
   aligned_st st;
   g_hash_table_foreach( t , (GHFunc)&eh_get_max_key_len , &max_len );
   st.fp = fp;
   st.max_key_len = max_len;
   g_hash_table_foreach( t , (GHFunc)&eh_print_symbol_aligned , &st );
}

gssize eh_symbol_table_size( Symbol_table *t )
{
   return g_hash_table_size( t );
}

Symbol_table* eh_destroy_symbol_table(Symbol_table *t)
{
   if ( t )
      g_hash_table_destroy( (GHashTable*)t );
   return NULL;
}

void eh_free_symbol_table_labels( Symbol_table *t )
{
   if ( t )
   {
      g_hash_table_foreach( (GHashTable*)t , &eh_destroy_key , NULL );
   }
}

#endif

#if defined( RECORD_FILE )

GHashTable *eh_scan_record_file(const char *filename)
{
   GScanner *s;
   char *record_name;
   Eh_symbol_table symbol_table;
   GHashTable *record_table;
   GSList *list;

   s = eh_open_scanner(filename);

   record_table = g_hash_table_new( &g_str_hash , &g_str_equal );

   while ( !g_scanner_eof(s) )
   {
      symbol_table = eh_symbol_table_new();
      record_name = eh_scan_next_record(s,symbol_table);
      if ( !record_name )
         break;
      list = (GSList*)g_hash_table_lookup(record_table,record_name);
      list = g_slist_append(list,symbol_table);
      g_hash_table_insert(record_table,record_name,list);
   }

   // Destroy the last symbol_table since it is empty
   eh_symbol_table_destroy( symbol_table );

   eh_close_scanner( s );

   return record_table;
}

void eh_destroy_record_file( GHashTable *table )
{
   g_hash_table_destroy( table );
}

Symbol_table *eh_scan_record_file_for( const char *filename , const char *rec_name )
{
   GHashTable *records;
   GSList *list;
   Symbol_table *a;

// for files files of normal size, scanning all of the records is fast; we'll just 
// get them all, not just the one we're looking for.
   records = eh_scan_record_file( filename );

// remove the record we're looking for.
   list = g_hash_table_lookup( records , rec_name );
   a = g_slist_nth_data( list , 0 );

   g_slist_remove( list , a );
   g_slist_free( list );
   g_hash_table_remove( records , rec_name );

// destroy the rest of the records.
   eh_destroy_record_file( records );

   return a;
}

Symbol_table* eh_scan_key_file_for( const gchar* file ,
                                    const gchar* name ,
                                    Symbol_table *tab )
{
   Symbol_table* new_tab;

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
      eh_symbol_table_copy( tab , new_tab );

   return new_tab;
}

#endif 

#include <errno.h>

GScanner *eh_open_scanner(const char *filename , GError** error )
{
   GScanner* s = NULL;

   eh_require( filename );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   {
      int fd = g_open( filename , O_RDONLY );

      if ( fd != -1 )
      {
         GScannerConfig config;

         config.cset_skip_characters = g_strdup( " \t\n" );

         config.cset_identifier_first = g_strconcat( G_CSET_a_2_z        , 
                                                     "^%.+-,_0123456789" ,
                                                     G_CSET_A_2_Z        , 
                                                     NULL );
         config.cset_identifier_nth   = g_strconcat( G_CSET_a_2_z        ,
                                                     G_CSET_A_2_Z        ,
                                                     "^%.+-,_0123456789" ,
                                                     G_CSET_LATINS       ,
                                                     G_CSET_LATINC       ,
                                                     NULL );
         config.cpair_comment_single = g_strdup( "#\n" );

         config.skip_comment_multi    = TRUE;      /* C like comment */
         config.skip_comment_single   = TRUE;      /* single line comment */
         config.scan_comment_multi    = TRUE;      /* scan multi line comments? */
         config.scan_identifier       = 1;
         config.scan_identifier_1char = 1;
         config.scan_identifier_NULL  = 1;
         config.scan_symbols          = 1;
         config.scan_binary           = FALSE;
         config.scan_octal            = FALSE;
         config.scan_float            = FALSE;
         config.scan_hex              = FALSE;     /* `0x0ff0' */
         config.scan_hex_dollar       = FALSE;     /* `$0ff0' */
         config.scan_string_sq        = TRUE;      /* string: 'anything' */
         config.scan_string_dq        = TRUE;      /* string: "\\-escapes!\n" */
         config.numbers_2_int         = TRUE;      /* bin, octal, hex => int */
         config.int_2_float           = TRUE;      /* int => G_TOKEN_FLOAT? */
         config.identifier_2_string   = TRUE;
         config.char_2_token          = TRUE;      /* return G_TOKEN_CHAR? */
         config.symbol_2_token        = FALSE;
         config.scope_0_fallback      = TRUE;      /* try scope 0 on lookups? */

         s = g_scanner_new(&config);
         s->input_name = filename;

         g_scanner_input_file( s , fd );
      }
      else
         eh_set_file_error_from_errno( error , filename , errno );
   }

   return s;
}

#include <unistd.h>

void eh_close_scanner( GScanner *s )
{
   close( s->input_fd );

   eh_free( s->config->cset_skip_characters  );
   eh_free( s->config->cset_identifier_first );
   eh_free( s->config->cset_identifier_nth   );
   eh_free( s->config->cpair_comment_single  );

   g_scanner_destroy( s );
}

#ifdef OLD

GPtrArray *eh_scan_data_file( const char *filename  ,
                              const char *delimeter ,
                              gboolean row_major    ,
                              gboolean with_header )
{
   GScanner *s;
   GPtrArray *data=g_ptr_array_new();
   Eh_data_record *entry;

   s = eh_open_scanner(filename);

   do
   {
      entry = eh_create_data_record( );

      if ( with_header )
         eh_scan_next_record(s,entry->symbol_table);

      eh_scan_data_record( s , delimeter , row_major , entry->data );

      g_ptr_array_add( data , entry );

   }
   while ( entry->data->len > 0 );

// the last entry was empty so remove and destroy it.
   g_ptr_array_remove( data , entry );
   eh_free_data_record( entry );
   eh_close_scanner( s );

   return data;
}

GPtrArray *eh_scan_data_record(GScanner *s, const char *delimeter, gboolean row_major, GPtrArray *data)
{
   double d_val;
   int i, n_rows;
   char *data_line=NULL;
   char **data_vals;
   GArray *vals;

   if ( !g_scanner_scope_lookup_symbol(s,0,"---") )
      g_scanner_scope_add_symbol(s,0,"---",g_strdup("---") );
/*
   if ( !g_scanner_scope_lookup_symbol(s,0,"[") )
      g_scanner_scope_add_symbol(s,0,"[",g_strdup("[") );

   if ( !g_scanner_scope_lookup_symbol(s,0,"]") )
      g_scanner_scope_add_symbol(s,0,"]",g_strdup("]") );
*/
   eh_debug( "Find the next record" );
   {
      char *record_name = eh_seek_record_start(s);
      if ( !record_name )
         return NULL;
      eh_free( record_name );
   }

   data_line = eh_scan_ascii_data_line(s);
   if ( data_line )
      data_vals = g_strsplit(data_line,delimeter,-1);
   else
      eh_error( "Trouble reading first line of data." );
   eh_free( data_line );

   for ( n_rows=0 ; data_vals[n_rows] ; n_rows++ );

   if ( row_major )
   {
      do
      {
         vals = g_array_new(FALSE,FALSE,sizeof(double));
         for ( i=0 ; i<n_rows && data_vals[i] ; i++ )
         {
            d_val = g_strtod(data_vals[i],NULL);
            g_array_append_val(vals,d_val);
         }
         g_ptr_array_add(data,vals);
         g_strfreev(data_vals);
         data_line = eh_scan_ascii_data_line(s);
         if ( data_line )
            data_vals = g_strsplit(data_line,delimeter,0);
         else
            data_vals = NULL;
         eh_free( data_line );
      }
      while ( data_vals );
   }
   else
   {
      for ( i=0 ; i<n_rows ; i++ )
         g_ptr_array_add(data,g_array_new(FALSE,FALSE,sizeof(double)));
      do
      {
         for ( i=0 ; i<n_rows && data_vals[i] ; i++ )
         {
            d_val = g_strtod(data_vals[i],NULL);
            g_array_append_val((GArray*)g_ptr_array_index(data,i),d_val);
         }
         g_strfreev(data_vals);
         data_line = eh_scan_ascii_data_line(s);
         if ( data_line )
            data_vals = g_strsplit(data_line,delimeter,0);
         else
            data_vals = NULL;
         eh_free( data_line );
      }
      while ( data_vals );
   }

   return data;
}

GPtrArray *eh_print_data_table(GPtrArray *data, const char *delimeter, gboolean row_major, FILE *fp )
{
   int i, j, n;
   GArray *row;

   if ( row_major )
   {
      for ( i=0 ; i<data->len ; i++ )
      {
         row = g_ptr_array_index( data , i );
         for ( j=0 ; j<row->len-1 ; j++ )
            fprintf(fp,"%f%c ",g_array_index(row,double,j),*delimeter);
         fprintf(fp,"%f\n",g_array_index(row,double,j));
      }
   }
   else
   {
      row = g_ptr_array_index( data , 0 );
      n = row->len;
      for ( i=0 ; i<n ; i++ )
      {
         for ( j=0 ; j<data->len-1 ; j++ )
         {
            row = g_ptr_array_index( data , j );
            fprintf(fp,"%f%c ",g_array_index(row,double,i),*delimeter);
         }
         row = g_ptr_array_index( data , j );
         fprintf(fp,"%f\n",g_array_index(row,double,i));
      }
   }
   return data;
}

#endif
gboolean eh_scanner_eol( GScanner *s );

double* eh_scan_ascii_data_line_dbl( GScanner* s , const char* delim ,  gssize *len )
{
   double* row = NULL;
   gssize n_cols = 0;

   if ( s )
   {
      gchar* str = eh_scan_ascii_data_line( s );

      if ( str )
      {
         gssize i;
         gchar** data_vals = g_strsplit( str , delim , -1 );

         n_cols = g_strv_length( data_vals );
         row  = eh_new( double , n_cols );

         for ( i=0 ; i<n_cols ; i++ )
            row[i] = g_ascii_strtod( data_vals[i] , NULL );
      
         g_strfreev( data_vals );

         eh_free( str );
      }
      else
         eh_error( "Trouble reading data at line %d (%s)" ,
                   g_scanner_cur_line(s) ,
                   (s->input_name)?(s->input_name):("unknown") );
   }

   *len = n_cols;

   return row;
}

char *eh_scan_ascii_data_line(GScanner *s)
{
   char *skip  = g_strdup(" \t"); // Don't skip over '\n'
   char *old_skip;
   GString *entry_string;
   char *entry;
   char c[2];
   c[1] = '\0';

   old_skip  = s->config->cset_skip_characters;
   s->config->cset_skip_characters  = skip;

   if (    g_scanner_eof(s) 
        || eh_scanner_eor(s) )
   {
      eh_free( skip );
      s->config->cset_skip_characters  = old_skip;
      return NULL;
   }
   else
      entry_string = g_string_new("");

   while ( eh_scanner_eol(s) && !g_scanner_eof(s) );

   if ( g_scanner_eof(s) )
   {
      eh_free( skip );
      s->config->cset_skip_characters  = old_skip;
      g_string_free( entry_string , TRUE );
      return NULL;
   }

   while (     g_scanner_get_next_token(s)!='\n'
           &&  g_scanner_cur_token(s)!='\r'
           && !g_scanner_eof(s)
           && !eh_scanner_eor(s) )
   {
      if ( g_scanner_cur_token(s) == G_TOKEN_STRING )
      {
         g_string_append(entry_string,g_scanner_cur_value(s).v_string);
      }
      else if ( g_scanner_cur_token(s) < G_TOKEN_NONE )
      {
         c[0] = g_scanner_cur_token(s);
         g_string_append(entry_string,c);
      }
      else
      {
         g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"eh_scan_ascii_data_line",TRUE);

         g_string_free(entry_string,TRUE);
         eh_free( skip );
         s->config->cset_skip_characters  = old_skip;
         return NULL;
      }
   }

   entry = entry_string->str;
   g_string_free( entry_string , FALSE );

   eh_free( skip );
   s->config->cset_skip_characters  = old_skip;

   return entry;
}

char *eh_scan_next_record( GScanner *s , Eh_symbol_table symbol_table )
{
   char *record_name;

   if ( !g_scanner_scope_lookup_symbol(s,0,"---") )
      g_scanner_scope_add_symbol(s,0,"---",g_strdup("---") );

/*
   if ( !g_scanner_scope_lookup_symbol(s,0,"[") )
      g_scanner_scope_add_symbol(s,0,"[",g_strdup("[") );

   if ( !g_scanner_scope_lookup_symbol(s,0,"]") )
      g_scanner_scope_add_symbol(s,0,"]",g_strdup("]") );
*/

   record_name = eh_seek_record_start(s);
   if ( !record_name )
      return NULL;

   // We found the start of a record.  Now read the symbol/value pairs until
   // we get to the end of the record or the end of the file.
   {
      char *label, *entry;

      while ( !g_scanner_eof(s) )
      {
         if ( g_scanner_peek_next_token(s) == G_TOKEN_STRING )
         {
            label = eh_scan_label(s);
            entry = eh_scan_entry(s);

            eh_symbol_table_insert(symbol_table,label,entry);

            eh_free( label );
            eh_free( entry );
         }
         else if ( eh_scanner_eor(s) )
            break;
         else if ( g_scanner_peek_next_token(s) == G_TOKEN_EOF )
            break;
         else if ( !eh_scanner_eol(s) )
         {
            g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"eh_scan_next_record",TRUE);
            eh_exit(-1);
         }
      }
   }

   return record_name;
}

gboolean
eh_scanner_eor(GScanner *s)
{
   if ( g_scanner_peek_next_token(s) == G_TOKEN_SYMBOL )
   {
      /* We've reached the start of the next record */
      if (    strncmp((char*)(s->next_value.v_symbol),"---",3)==0 )
         return TRUE;
      else
      {
         g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"eh_scanner_eor",TRUE);
         eh_exit(-1);
      }
   }
   else if ( g_scanner_peek_next_token(s) == G_TOKEN_LEFT_BRACE )
      return TRUE;

   return FALSE;
}

gboolean eh_scanner_eol( GScanner *s )
{
   if (    g_scanner_peek_next_token(s) != '\n'
        && g_scanner_peek_next_token(s) != '\r' )
      return FALSE;
   else
   {
      g_scanner_get_next_token(s);
      eh_scanner_eol(s);
      return TRUE;
   }
}

/*
void eh_print_record_list(const char *key, GSList *list, FILE *fp)
{
   int i;
   fprintf(fp,"--- %s ---\n",key);
   for ( i=0 ; i<g_slist_length(list); i++ )
      g_hash_table_foreach((GHashTable*)g_slist_nth_data(list,i),(GHFunc)&eh_print_symbol,stdout);
}

void eh_print_record( const char *rec_name , Symbol_table *t , FILE *fp )
{
   fprintf(fp,"--- %s ---\n",rec_name);
   g_hash_table_foreach( t ,(GHFunc)&eh_print_symbol , fp );
}
*/
char*
eh_scan_label(GScanner *s)
{
   char *first = g_strconcat(G_CSET_a_2_z," _0123456789",G_CSET_A_2_Z,NULL);
   char *nth   = g_strconcat(G_CSET_a_2_z,G_CSET_A_2_Z," _0123456789",G_CSET_LATINS,G_CSET_LATINC,NULL);
   char* comment = g_strdup( "()" );
   GString *label = g_string_new("");
   char *old_first, *old_nth, *old_comment;
   char* ans;
   char c[2];
   c[1] = '\0';

   old_first   = s->config->cset_identifier_first;
   old_nth     = s->config->cset_identifier_nth;
   old_comment = s->config->cpair_comment_single;

   s->config->cset_identifier_first = first;
   s->config->cset_identifier_nth   = nth;
   s->config->cpair_comment_single  = comment;

   while ( g_scanner_peek_next_token(s) != ':' && g_scanner_peek_next_token(s) != G_TOKEN_EOF )
   {
      if ( g_scanner_get_next_token(s) == G_TOKEN_STRING )
         g_string_append(label,g_scanner_cur_value(s).v_string);
      else if ( g_scanner_cur_token(s) < G_TOKEN_NONE )
      {
         c[0] = g_scanner_cur_token(s);
         g_string_append(label,c);
      }
      else
      {
         g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"get_label",TRUE);
         eh_exit(-1);
      }
   }
   if ( g_scanner_peek_next_token(s) == G_TOKEN_EOF )
      g_scanner_unexp_token(s,(GTokenType)':',NULL,NULL,NULL,"get_label",TRUE);

   g_scanner_get_next_token(s);

   s->config->cset_identifier_first = old_first;
   s->config->cset_identifier_nth   = old_nth;
   s->config->cpair_comment_single  = old_comment;

   eh_free( comment );
   eh_free( nth     );
   eh_free( first   );

   ans = label->str;

   g_string_free( label , FALSE );

   return g_strstrip(ans);
}

char *eh_scan_entry(GScanner *s)
{
   char *skip  = g_strdup(" \t"); // Don't skip over '\n'
   char *first = g_strconcat(G_CSET_a_2_z,"_0123456789",G_CSET_A_2_Z,NULL);
   char *nth   = g_strconcat(G_CSET_a_2_z,G_CSET_A_2_Z,"_0123456789",G_CSET_LATINS,G_CSET_LATINC,NULL);
   char *old_skip, *old_first, *old_nth;
   GString *entry = g_string_new("");
   char* ans;
   char c[2];
   c[1] = '\0';

   old_first = s->config->cset_identifier_first;
   old_nth   = s->config->cset_identifier_nth;
   old_skip  = s->config->cset_skip_characters;

   s->config->cset_skip_characters  = skip;
   s->config->cset_identifier_first = first;
   s->config->cset_identifier_nth   = nth;
   
   while ( g_scanner_peek_next_token(s) != '\n' && g_scanner_peek_next_token(s) != G_TOKEN_EOF )
   {
      if ( g_scanner_get_next_token(s) == G_TOKEN_STRING )
         g_string_append(entry,g_scanner_cur_value(s).v_string);
      else if ( g_scanner_cur_token(s) < G_TOKEN_NONE )
      {
         c[0] = g_scanner_cur_token(s);
         g_string_append(entry,c);
      }
      else
      {
         g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"get_entry",TRUE);
         eh_exit(-1);
      }
   }

   g_scanner_get_next_token(s);

   s->config->cset_skip_characters  = old_skip;
   s->config->cset_identifier_first = old_first;
   s->config->cset_identifier_nth   = old_nth;

   eh_free( skip  );
   eh_free( nth   );
   eh_free( first );

   ans = entry->str;

   g_string_free( entry , FALSE );

   return g_strstrip(ans);
}

#include <string.h>

char *eh_seek_record_start(GScanner *s)
{
   gchar* ans;
   char c[2];
   GString *record_name = g_string_new("");
   c[1] = '\0';

   while ( !g_scanner_eof(s) )
   {
      if ( g_scanner_get_next_token(s) == G_TOKEN_SYMBOL )
      {
         if ( strncmp((char*)g_scanner_cur_value(s).v_symbol,"---",3)==0  )
            break;
      }
      else if ( g_scanner_cur_token(s)==G_TOKEN_LEFT_BRACE )
         break;
   }

   if ( g_scanner_eof(s) )
   {
      g_string_free( record_name , TRUE );
      return NULL;
   }

   while ( !g_scanner_eof(s) )
   {
      if ( g_scanner_get_next_token(s) == G_TOKEN_STRING )
      {
         g_string_append(record_name,g_scanner_cur_value(s).v_string);
      }
      else if ( g_scanner_cur_token(s) == G_TOKEN_RIGHT_BRACE )
         break;
      else if ( g_scanner_cur_token(s) == G_TOKEN_SYMBOL )
      {
         if ( strncmp((char*)g_scanner_cur_value(s).v_symbol,"---",3)==0 )
            break;
      }
      else if ( g_scanner_cur_token(s) < G_TOKEN_NONE )
      {
         c[0] = g_scanner_cur_token(s);
         g_string_append(record_name,c);
      }
      else
      {
         g_scanner_unexp_token(s,G_TOKEN_STRING,NULL,NULL,NULL,"get_next_record",TRUE);
         eh_exit(-1);
      }
   }

   while ( eh_scanner_eol(s) && !g_scanner_eof(s) );

   ans = record_name->str;

   g_string_free( record_name , FALSE );

   return ans;
}

#ifdef OLD

Eh_data_record *eh_create_data_record()
{
   Eh_data_record *p=eh_new( Eh_data_record , 1 );
   p->symbol_table = eh_create_symbol_table();
   p->data = g_ptr_array_new();
   return p;
}

void eh_free_data_record(Eh_data_record *p)
{
   eh_destroy_symbol_table( p->symbol_table );
   g_ptr_array_free( p->data , FALSE );
   eh_free( p );
}

void eh_print_data_record( Eh_data_record *p , char *rec_name , char *delimeter , gboolean row_major , gboolean with_header , FILE *fp )
{
   if ( with_header )
   {
      fprintf( fp , "[ %s ]\n" , rec_name );
      eh_print_symbol_table_aligned( p->symbol_table , fp );
      fprintf(fp,"[ data ]\n");
   }
   eh_print_data_table( p->data , delimeter , row_major , fp );
}

int
eh_get_data_record_size( Eh_data_record *p , int dim )
{
   if ( dim==0 )
      return p->data->len;
   else if ( dim==1 )
      return ((GArray*)g_ptr_array_index(p->data,0))->len;
   else
   {
      eh_info( "dimension argument must be 0 or 1 : eh_get_data_record_size" );
      eh_exit(-1);
   }
   eh_require_not_reached();
   return -1;
}

Symbol_table *eh_get_data_record_sym_table(Eh_data_record *p)
{
   return p->symbol_table;
}

GArray *eh_get_data_record_row(Eh_data_record *p,int row)
{
   if ( row < eh_get_data_record_size(p,0) )
      return (GArray*)g_ptr_array_index(p->data,row);
   else
      eh_error( "eh_get_data_record_row: Row is out of bounds: %d >= %d" ,
                row , eh_get_data_record_size(p,0) );
   eh_require_not_reached();
   return NULL;
}

void eh_set_data_record_row( Eh_data_record *p , int row , GArray *a )
{
   g_ptr_array_index( p->data , row ) = a;
}

void eh_add_data_record_row( Eh_data_record *p , GArray *a )
{
   g_ptr_array_add( p->data , a );
}

void eh_add_data_record_label( Eh_data_record *p , char *label , char *value )
{
   eh_symbol_table_insert( p->symbol_table , label , value );
}

void eh_interpolate_data_record_rows( Eh_data_record *p , int row , GArray *x )
{
   GArray *x1, *y1, *y;
   int i, len1, len = eh_get_data_record_size( p , 0 );

   x1 = eh_get_data_record_row( p , row );
   len1 = x1->len;
//   len = x->len;

   eh_set_data_record_row( p , row , x );

   for ( i=0 ; i<len ; i++ )
   {
      if ( i!=row )
      {
         // alocate memory for the new array.
         y  = g_array_new( FALSE , FALSE , sizeof(double) );
         g_array_set_size( y , x->len );

         // get the next row.
         y1 = eh_get_data_record_row( p , i );

         // interpolate the row.
         interpolate( (double*)(x1->data) , (double*)(y1->data) , len1 ,
                      (double*)(x->data)  , (double*)(y->data)  , x->len );

         // free the memory for old array and substitute it with the new array.
         g_array_free( y1 , FALSE );
         eh_set_data_record_row( p , i , y );
      }
   }

   // free the memory for the old row and substitute it with the new one.
//   g_array_free( eh_get_data_record_row( p , row ) , FALSE );
   g_array_free( x1 , FALSE );
}

Eh_record_file *eh_open_record_file( const char *filename )
{
   Eh_record_file *rec_file = eh_new0( Eh_record_file , 1 );

   rec_file->filename = g_strdup( filename );
   rec_file->records = eh_scan_record_file( filename );

   return rec_file;
}

void eh_close_record_file( Eh_record_file *rec_file )
{
   if ( rec_file )
   {
      eh_free( rec_file->filename );
      eh_free( rec_file );
   }
}

Eh_symbol_table *eh_get_record_from_record_file( Eh_record_file *rec_file , const char *name )
{
   GSList *rec_list = g_hash_table_lookup( rec_file->records , name );
   return g_slist_nth_data( rec_list , 0 );
}

char *eh_get_value_from_record_file( Eh_record_file *rec_file , const char *rec_name , const char *label )
{
   Eh_symbol_table *record = eh_get_record_from_record_file( rec_file , rec_name );
   return eh_symbol_table_lookup( record , (char*)label );
}

#endif

Eh_data_file*
eh_open_data_file( const char *filename , Eh_data_file_attr *attr , GError** error )
{
   Eh_data_file* data_file = NULL;
   GError*       tmp_err   = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   data_file = eh_new0( Eh_data_file , 1 );

   if ( attr==NULL )
   {
      data_file->delimeter   = ",";
      data_file->fast_dim    = EH_FAST_DIM_COL;
      data_file->row_major   = FALSE;
      data_file->with_header = TRUE;
   }
   else
   {
      data_file->delimeter   = attr->delimeter;
      data_file->fast_dim    = EH_FAST_DIM_COL;
      data_file->row_major   = attr->row_major;
      data_file->with_header = attr->with_header;
   }

   data_file->filename    = g_strdup( filename );
   data_file->records     = eh_data_record_scan_file( filename             ,
                                                      data_file->delimeter ,
                                                      data_file->fast_dim  ,
                                                      data_file->with_header ,
                                                      &tmp_err );

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );

      eh_free( data_file->filename );
      eh_free( data_file           );

      data_file = NULL;
   }

   return data_file;
}

void eh_close_data_file( Eh_data_file *data_file )
{
   if ( data_file )
   {
      gssize i;
      eh_free( data_file->filename );
      for ( i=0 ; data_file->records[i] ; i++ )
         eh_data_record_destroy( data_file->records[i] );
      eh_free( data_file->records );
   }
}

int eh_get_data_file_size( Eh_data_file *data_file )
{
   gssize len=0;
   eh_require( data_file )
   {
      for ( len=0 ; data_file->records[len] ; len++ );
   }
   return len;
}

#ifdef OLD
Eh_data_record *eh_get_data_from_file( Eh_data_file *data_file , int i )
{
   return (Eh_data_record*)g_ptr_array_index( data_file->records , i );
}
#endif

