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
#include <stdarg.h>
#include <stdlib.h>

GHashTable* _log_files_;
long int _log_file_code_;

#include <time.h>

FILE*
eh_open_log( const char *log_name )
{
   FILE *fp = NULL;
   char *log_file_name;
   GPtrArray *new_vec, *log_fp_vec;
   time_t current_time;

   current_time = time( &current_time );

   if ( !_log_files_ )
   {
      _log_files_ = g_hash_table_new(&g_str_hash,&eh_str_case_equal);
      new_vec = g_ptr_array_new();
      g_ptr_array_add(new_vec,NULL);
      g_hash_table_insert(_log_files_,g_strdup("/dev/null"),new_vec);
      new_vec = g_ptr_array_new();
      g_ptr_array_add(new_vec,stdout);
      g_hash_table_insert(_log_files_,g_strdup("stdout"),new_vec);
      new_vec = g_ptr_array_new();
      g_ptr_array_add(new_vec,stderr);
      g_hash_table_insert(_log_files_,g_strdup("stderr"),new_vec);
      _log_file_code_ = random();
   }
//   if ( log_name && !g_hash_table_lookup(_log_files_,log_name) )
   if ( log_name )
   {

      log_fp_vec = g_hash_table_lookup( _log_files_ , log_name );
      if ( log_fp_vec )
         fp = (FILE*)g_ptr_array_index(log_fp_vec,log_fp_vec->len-1);
      else
      {

         log_file_name = g_strconcat( log_name , ".log" , NULL );
         g_strstrip(log_file_name);
         while ( strstr(log_file_name," ") != NULL )
            strstr(log_file_name," ")[0] = '_';

//        fp = eh_open_log_file( log_file_name );
         fp = fopen( log_file_name , "w" );
         fprintf( fp , "# Creation data : %s"    , ctime( &current_time ) );
         fprintf( fp , "# Log code      : %ld\n" , _log_file_code_ );
         fflush( fp );
         new_vec = g_ptr_array_new();
         g_ptr_array_add(new_vec,fp);
         g_hash_table_insert(_log_files_,g_strdup(log_name),new_vec);

         eh_free( log_file_name );
      }
   }

   return fp;
}

FILE *eh_open_log_file(const char *log_name)
{
   FILE *fp;
   char *good_name;

//   good_name = g_strdup(log_name);

   fp = eh_open_log( log_name );
/*
   good_name = g_strconcat(log_name,".log",NULL);
   g_strstrip(good_name);
   while ( strstr(good_name," ") != NULL )
      strstr(good_name," ")[0] = '_';
*/

//   fp = fopen(good_name,"w");

   if ( !fp )
   {
      fprintf(stderr,"Could not open log file, %s\n",good_name);
      eh_exit( EXIT_FAILURE );
   }

//   eh_free(good_name);

   return fp;
}

void eh_print_log(const char *log_name, const char *message,...)
{
   GPtrArray *log_fp_vec;
   FILE *fp;
   char *stripped_msg;
   va_list ap;
   if ( !_log_files_ )
      eh_open_log( log_name );
   if ( (log_fp_vec=(GPtrArray*)g_hash_table_lookup(_log_files_,log_name)) )
   {
      stripped_msg = g_strconcat(log_name," : ",message,NULL);
      g_strchomp(stripped_msg);
      va_start(ap,message);
      fp = (FILE*)g_ptr_array_index(log_fp_vec,log_fp_vec->len-1);
      if ( fp )
      {
         vfprintf(fp,g_strconcat(stripped_msg,"\n",NULL),ap);
         fflush(fp);
      }
      va_end(ap);
      eh_free(stripped_msg);
   }
}

void eh_close_log(const char *log_name)
{
   GPtrArray *log_fp_vec;
   if ( log_name && (log_fp_vec=(GPtrArray*)g_hash_table_lookup(_log_files_,log_name)) )
   {
      fclose((FILE*)g_ptr_array_index(log_fp_vec,log_fp_vec->len-1));
      g_ptr_array_free(log_fp_vec,TRUE);
      g_hash_table_remove(_log_files_,log_name);
   }
}

void eh_redirect_log(const char *log_file1,const char *log_file2)
{
   GPtrArray *log_fp_vec1;
   GPtrArray *log_fp_vec2;
   if ( !_log_files_ )
      eh_open_log( log_file1 );
   log_fp_vec1 = (GPtrArray*)g_hash_table_lookup(_log_files_,log_file1);
   log_fp_vec2 = (GPtrArray*)g_hash_table_lookup(_log_files_,log_file2);
   if ( log_fp_vec1 && log_fp_vec2 )
      g_ptr_array_add(log_fp_vec2,g_ptr_array_index(log_fp_vec1,log_fp_vec1->len-1));
}

void eh_reset_log(const char *log_file)
{
   GPtrArray *log_fp_vec;
   log_fp_vec = (GPtrArray*)g_hash_table_lookup(_log_files_,log_file);
   if ( log_fp_vec && log_fp_vec->len > 1 )
     g_ptr_array_remove_index(log_fp_vec,log_fp_vec->len-1);
}

static GLogLevelFlags eh_ignore_log_level = 0;

void
eh_set_ignore_log_level( GLogLevelFlags ignore )
{
   eh_ignore_log_level |= ignore;
}

GLogLevelFlags
eh_set_verbosity_level( gint verbosity )
{
   GLogLevelFlags ignore = 0;

   if ( verbosity<0 )
      verbosity = 0;

   if ( verbosity>6 )
      verbosity = 6;

   switch ( verbosity )
   {
      case 0:
         ignore |= G_LOG_LEVEL_ERROR;
      case 1:
         ignore |= G_LOG_LEVEL_CRITICAL;
      case 2:
         ignore |= G_LOG_LEVEL_WARNING;
      case 3:
         ignore |= G_LOG_LEVEL_MESSAGE;
      case 4:
         ignore |= G_LOG_LEVEL_INFO;
      case 5:
         ignore |= G_LOG_LEVEL_DEBUG;
   }

   eh_set_ignore_log_level( ignore );

   return ignore;
}

void eh_logger( const gchar*   log_domain ,
                GLogLevelFlags log_level  ,
                const gchar*   message    ,
                gpointer       user_data )
{
   FILE **fp_list;
   gchar *log_label = NULL;
   gboolean is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
   int i;

   if ( eh_ignore_log_level & log_level )
      return;

   if ( !message )
      message = "(NULL) message";

   if ( user_data && !(log_level & G_LOG_LEVEL_DEBUG) )
      fp_list = (FILE**)user_data;
   else
   {
      fp_list = eh_new( FILE* , 2 );
      fp_list[0] = stderr;
      fp_list[1] = NULL;
   }

   if ( log_level & G_LOG_LEVEL_WARNING )
      log_label = "Warning";
   else if ( log_level & G_LOG_LEVEL_ERROR )
      log_label = "Error";

   for ( i=0 ; fp_list[i]!=NULL ; i++ )
   {
      if ( log_label )
         fprintf( fp_list[i] , "%s: " , log_label );
      if ( log_domain && !(log_level&EH_LOG_LEVEL_DATA) )
         fprintf( fp_list[i] , "%s: " , log_domain );
      fprintf( fp_list[i] , "%s\n" , message    );

      fflush ( fp_list[i] );
   }

   if ( is_fatal )
      eh_exit( EXIT_FAILURE );

   if ( !(user_data && !(log_level & G_LOG_LEVEL_DEBUG)) )
      eh_free( fp_list );
}


