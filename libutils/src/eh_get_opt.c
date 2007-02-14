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

#include "eh_get_opt.h"
#include "utils.h"

Eh_args *eh_create_args( void )
{
   Eh_args *new_args = eh_new( Eh_args , 1 );
   new_args->args = eh_symbol_table_new();
   new_args->defaults = eh_symbol_table_new();
   return new_args;
}

void eh_destroy_args( Eh_args *args )
{
   if ( args )
   {
      if ( args->args )
         eh_symbol_table_destroy( args->args );
      if ( args->defaults )
         eh_symbol_table_destroy( args->defaults );
      eh_free( args );
   }
}

Eh_args *eh_opts_init( int argc, char *argv[] )
{
   Eh_args* args = eh_new( Eh_args , 1 );
   char *base_name = g_path_get_basename( argv[0]);
   int i, len, arg_no;
   char **opt;
   char *label, *value;

   args->args = eh_symbol_table_new();

   for ( i=1,arg_no=0 ; i<argc ; i++ )
   {
      opt = g_strsplit( argv[i] , "=" , -1 );
      for ( len=0 ; opt[len] ; len++ );
      if ( len==1 )
      {
         label = g_strdup_printf( "arg%d" , arg_no );
         value = opt[0];
         arg_no++;
      }
      else
      {
         label = g_strstrip(opt[0]);
         value = g_strstrip(opt[1]);
      }
      eh_free( opt );
      eh_symbol_table_insert( args->args , label , value );
   }

   args->defaults = eh_get_opt_defaults( base_name );

   eh_free( base_name );

   return args;
}

Eh_symbol_table eh_get_opt_defaults( const char *prog_name )
{
   char *ehrc = g_strdup( ".ehrc" );
   char *cur_dir = g_get_current_dir();
   const char *home_dir = g_get_home_dir();
   char *rc_file;
   Eh_symbol_table defaults;
   Eh_symbol_table home_defaults=NULL;
   Eh_symbol_table dir_defaults=NULL;

// try the .ehrc file in the user's home directory.
   rc_file  = g_strconcat( home_dir , "/" , ehrc , NULL );
   if ( eh_is_readable_file( rc_file ) )
      home_defaults = eh_key_file_scan_for( rc_file , prog_name , NULL , NULL );
   if ( !home_defaults )
      home_defaults = eh_symbol_table_new();
   eh_free(rc_file);
//   eh_free(home_dir);

// try the .ehrc file in the current directory.
   rc_file  = g_strconcat( cur_dir , "/" , ehrc , NULL );
   if ( eh_is_readable_file( rc_file ) )
      dir_defaults = eh_key_file_scan_for( rc_file , prog_name , NULL , NULL );
   if ( !dir_defaults )
      dir_defaults = eh_symbol_table_new();
   eh_free(rc_file);
   eh_free( cur_dir );

// merge the home and current directory defaults together.  the home defaults
// go first so that any defaults in dir_defaults will take precedence.
   defaults = eh_symbol_table_merge( home_defaults , dir_defaults , NULL );

// destroy the temp default symbol tables.
   eh_symbol_table_destroy( home_defaults );
   eh_symbol_table_destroy( dir_defaults );

   eh_free( ehrc );

   return defaults;
}

char *eh_args_lookup(Eh_args *t, char *key)
{
   return eh_symbol_table_lookup(t->args,key);
}

void eh_args_insert(Eh_args *t, char *key, char *value)
{
   eh_symbol_table_insert(t->args,key,value);
}

void eh_args_insert_default(Eh_args *t, char *key, char *value)
{
   eh_symbol_table_insert(t->defaults,key,value);
}

typedef struct
{
   char **possible;
   gboolean err_flag;
}
Label_is_valid_st;

void check_label_is_possible( char *key , char *value , Label_is_valid_st *user_data );
void check_label_is_valid( char *key , char *value , int *err_flag );

gboolean eh_check_opts( Eh_args* args , char **required , char **possible , char *help[] )
{
   gboolean print_help=FALSE;
   int i, err_flag=0;
   Label_is_valid_st possible_st;

   possible_st.possible = possible;
   possible_st.err_flag = err_flag;

   if ( required )
      for ( i=0 ; required[i] ; i++ )
      {
         if ( !eh_symbol_table_lookup( args->args , required[i] ) )
            fprintf(stderr,"error : required arg %s not given\n",required[i]), err_flag++;
      }

   if ( possible )
   {
      possible_st.err_flag = err_flag;
      eh_symbol_table_foreach( args->args , (GHFunc)&check_label_is_possible, &possible_st );
      err_flag = possible_st.err_flag;
   }
   
   eh_symbol_table_foreach( args->args , (GHFunc)&check_label_is_valid , &err_flag);

   print_help = eh_get_opt_bool( args , g_strdup("help") , FALSE );
   if ( (err_flag || print_help) && help )
      eh_print_message( stderr , help );

   return err_flag||print_help;
}

void check_label_is_possible( char *key , char *value , Label_is_valid_st *user_data )
{
   char **possible = user_data->possible;
   int i;
   gboolean found=FALSE;
   gboolean is_valid=FALSE;

   if ( g_strncasecmp( key , "arg"      , 3 )==0 )
     is_valid=TRUE;
   else
   {
      for ( i=0 ; possible[i] && !found ; i++ )
         if ( g_strcasecmp ( key , possible[i] )==0 )
            found = TRUE;
      if ( !found )
         fprintf(stderr,"error : arg %s is not a valid argument\n",key);
      else
         is_valid=TRUE;
   }
   user_data->err_flag = is_valid;
}

void check_label_is_valid( char *key , char *value , int *err_flag )
{
   gboolean is_valid=FALSE;

   if ( g_strncasecmp( key , "arg" , 3 )==0 )
      is_valid=TRUE;
   else if ( g_strcasecmp( value , "" )==0 )
      fprintf(stderr,"error : no value given for arg %s\n",key);
   else
      is_valid=TRUE;
   if ( !is_valid )
      (*err_flag)++;
}

void eh_print_opt( Eh_args *args , char *label )  
{
   fprintf( stderr , "%s = %s"  , label , eh_get_opt( args , label ) );
   fprintf( stderr , " [%s]\n"  , eh_get_opt_default( args , label ) );
}

char *eh_get_opt( Eh_args *args , char *label )
{
   char *value = eh_args_lookup( args , label );
   char *rtn;
   if ( value )
      rtn = value;
   else
      rtn = eh_get_opt_default( args , label );
   return rtn;
}

char *eh_get_opt_default( Eh_args *args , char *label )
{
   return eh_symbol_table_lookup( args->defaults , label );
}

char *eh_get_arg_n( Eh_args *args , int n )
{
   char *arg_label = g_strdup_printf( "arg%d" , n );
   char *rtn = eh_get_opt( args , arg_label );
   eh_free( arg_label );
   return rtn;
}

char *eh_get_opt_str( Eh_args *args , char *label , char *default_val )
{
   char *value = eh_get_opt( args , label );
   char *default_str = g_strdup( default_val );
   char *rtn;

   if ( !eh_get_opt_default( args , label ) )
      eh_args_insert_default( args , label , default_str );

   if ( value )
      rtn = g_strdup(value);
   else
      rtn = g_strdup(default_val);

   return rtn;
}

gboolean eh_get_opt_bool( Eh_args *args , char *label , gboolean default_val )
{
   char *value = eh_get_opt( args , label );
   char *default_str = g_strdup_printf( "%d" , default_val );
   gboolean rtn;

   if ( !eh_get_opt_default( args , label ) )
      eh_args_insert_default( args , label , default_str );

   if ( value )
      rtn = strtobool(value);
   else
    
  rtn = default_val;
   return rtn;
}

int eh_get_opt_key( Eh_args *args , char *label , int default_val , char *keys[] )
{
   char *value = eh_get_opt( args , label );
   char *default_str = g_strdup_printf( "%d" , default_val );
   int i, rtn;

   if ( !eh_get_opt_default( args , label ) )
      eh_args_insert_default( args , label , default_str );

   if ( value )
   {
      for ( i=0 ; keys[i] ; i++ )
         if ( g_strcasecmp( value , keys[i] )==0 )
         {
            rtn = i;
            break;
         }
      if ( !keys[i] )
      {
         fprintf( stderr , "error : unknown key (%s) for opt %s.\n" , value , label );
         fprintf( stderr , "error : possible keys are: ");
         for ( i=0 ; keys[i+1] ; i++ )
            fprintf( stderr , "%s, " , keys[i] );
         fprintf( stderr , "or %s\n" , keys[i] );
         eh_exit(-1);
      }
   }
   else
      rtn = default_val;

   return rtn;
}

int eh_get_opt_int( Eh_args *args , char *label , int default_val )
{
   char *value = eh_get_opt( args , label );
   char *default_str = g_strdup_printf( "%d" , default_val );
   int rtn;

   if ( !eh_get_opt_default( args , label ) )
      eh_args_insert_default( args , label , default_str );

   if ( value )
      sscanf(value, "%d" , &rtn );
   else
      rtn = default_val;

   return rtn;
}

double eh_get_opt_dbl( Eh_args *args , char *label , double default_val )
{
   char *value = eh_get_opt( args , label );
   char *default_str = g_strdup_printf( "%g" , default_val );
   double rtn;

   if ( !eh_get_opt_default( args , label ) )
      eh_args_insert_default( args , label , default_str );

   if ( value )
      sscanf(value, "%lf" , &rtn );
   else
      rtn = default_val;
   return rtn;
}

void eh_print_message( FILE *fp , char *msg[] )
{
   char **p;
   for ( p=msg ; *p ; p++ )
      fprintf(fp,"%s\n",*p);
}

typedef struct
{
   int max_key_len;
   int max_value_len;
   Eh_args *args;
}
Print_opt_pad_st;

void print_opt_pad( char *key , char *value , Print_opt_pad_st *user_data );
void get_max_label_length( char *key , char *value , Print_opt_pad_st *user_data );

void eh_print_all_opts( Eh_args *args , char *prog_name , FILE *fp )
{
   Print_opt_pad_st data;

   data.max_key_len   = 0;
   data.max_value_len = 0;
   data.args          = args;

   fprintf( fp , "--- %s ---\n" , prog_name );

   eh_symbol_table_foreach( args->defaults , (GHFunc)&get_max_label_length , &data );
   eh_symbol_table_foreach( args->defaults , (GHFunc)&print_opt_pad , &data );

   return;
}

void print_opt_pad( char *key , char *unused , Print_opt_pad_st *user_data )  
{
   char *str;
   char *value;

   str = g_strdup_printf( "%%-%ds : %%-%ds\n" ,
                          user_data->max_key_len ,
                          user_data->max_value_len );
   value = eh_get_opt( user_data->args , key );
   if ( strlen( value )>0 && g_strtod(value,NULL)!=E_NOVAL )
      fprintf( stderr , str  , key , value );
   else
      fprintf( stderr , str  , key , "<no value>" );
   eh_free( str );

   return;
}

void get_max_label_length( char *key , char *value , Print_opt_pad_st *user_data )
{
   int key_len   = strlen( key );
   int value_len = strlen( value );

   if ( key_len > user_data->max_key_len )
      user_data->max_key_len = key_len;
   if ( value_len > user_data->max_value_len )
      user_data->max_value_len = value_len;

   return;
}


