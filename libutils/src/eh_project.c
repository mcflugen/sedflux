#include "eh_project.h"
#include "utils.h"

CLASS( Eh_project )
{
   char *name;
   char *working_dir_name;
   GDir *working_dir;
   GTree *open_files;
   GKeyFile *info_file;
};

char* construct_project_group_name( Eh_project p );
int   eh_sort_ptr( gconstpointer a , gconstpointer b , gpointer user_data );

Eh_project eh_create_project( const char *proj_name )
{
   Eh_project new_proj;

   NEW_OBJECT( Eh_project , new_proj );

   new_proj->name = g_strdup( proj_name );

   new_proj->working_dir = NULL;

   new_proj->open_files = g_tree_new_full( &eh_sort_ptr ,
                                           NULL         ,
                                           (GDestroyNotify)&fclose ,
                                           &eh_free_c_style );

   new_proj->working_dir_name = NULL;

   // Open the project info file.
   new_proj->info_file = g_key_file_new();

   eh_fill_project_info( new_proj );

   return new_proj;
}

char* eh_project_name( Eh_project p )
{
   return p->name;
}

char* eh_project_dir_name( Eh_project p )
{
   return p->working_dir_name;
}

GDir* eh_project_dir( Eh_project p )
{
   return p->working_dir;
}

GKeyFile* eh_project_info_file( Eh_project p )
{
   return p->info_file;
}

char* eh_project_info_file_full_name( Eh_project p )
{
   char* base_name = g_strconcat( eh_project_name(p) , ".info" , NULL );
   char* name      = g_build_filename( eh_project_dir_name(p) ,
                                       base_name              ,
                                       NULL );

   eh_free( base_name );

   return name;
}

gchar* eh_project_get_info_val( Eh_project p , const gchar* key )
{
   gchar* group_name = construct_project_group_name( p );
   gchar* value;

   // If the key doesn't exist, return NULL.
   value = g_key_file_get_value( p->info_file , group_name , key , NULL );

   eh_free( group_name );

   return value;
}

Eh_project eh_project_set_info_val( Eh_project p     ,
                                    const gchar* key ,
                                    const gchar* value )
{
   char* group_name = construct_project_group_name( p );

   // If the key doesn't exist, a new one is created.
   g_key_file_set_value( p->info_file , group_name , key , value );

   eh_free( group_name );

   return p;
}

Eh_project eh_project_add_info_val( Eh_project p , char* key , char* val )
{
   char* group_name = construct_project_group_name( p );

   //---
   // If the key already exists, add the new value to the list.
   //
   // If the key doesn't exits create it and add the value.
   //---
   if ( g_key_file_has_key( p->info_file , group_name , key , NULL ) )
   {
      gchar** cur_val;
      gsize n_vals;

      cur_val = g_key_file_get_string_list( p->info_file ,
                                            group_name   ,
                                            key          ,
                                            &n_vals      ,
                                            NULL );

      n_vals            = n_vals+1;
      cur_val           = eh_renew( char* , cur_val , n_vals+1 );
      cur_val[n_vals-1] = g_strdup( val );

      g_key_file_set_string_list( p->info_file ,
                                  group_name   ,
                                  key          ,
                                  cur_val      ,
                                  n_vals );

      g_strfreev( cur_val );
   }
   else
      g_key_file_set_value( p->info_file , group_name , key , val );

   eh_free( group_name );

   return p;
}

#define DONT_ASK (TRUE)

Eh_project eh_set_project_dir( Eh_project proj , const char* dir_name )
{
   GError *error = NULL;

   eh_require( dir_name );

   proj->working_dir_name = g_strdup( dir_name );
   proj->working_dir = g_dir_open( dir_name , 0 , &error );
   if ( error )
   {
      if (    ( DONT_ASK || eh_input_boolean( "Ok to create directory?" , TRUE ) )
           && g_mkdir( dir_name , S_ISUID|S_ISGID|S_IRWXU|S_IRWXG ) == 0 
           && (proj->working_dir = g_dir_open( dir_name , 0 , &error )) )
      {
         g_error_free( error );
      }
      else
      {
         eh_error( "Unable to open project: %s\n" , error->message );
      }
   }

   return proj;
}

#include <time.h>

char* construct_project_group_name( Eh_project p )
{
   return g_strconcat( p->name , " info entry" , NULL );
}

#include <time.h>

Eh_project eh_set_project_current_time( Eh_project p )
{
   GKeyFile *file = p->info_file;
   char *group_name = construct_project_group_name( p );

   //---
   // Define end date and time.
   //---
   eh_require( file )
   {
      time_t clock;
      char date_str[S_LINEMAX];
      char time_str[S_LINEMAX];
      struct tm now;
      GDate *today = g_date_new();

      g_date_set_time( today , time(&clock) );
      localtime_r( &clock , &now );
      g_date_strftime( date_str , S_LINEMAX , "%d/%m/%Y" , today );
      sprintf( time_str , "%d:%d:%d" , now.tm_hour , now.tm_min , now.tm_sec );

      g_key_file_set_value( file , group_name , "CURRENT DATE"  , date_str );
      g_key_file_set_value( file , group_name , "CURRENT TIME"  , time_str );

      g_date_free( today );
   }

   return p;
}

Eh_project eh_fill_project_info( Eh_project p )
{
   GKeyFile *file = p->info_file;
   char *group_name = construct_project_group_name( p );

   //---
   // Place comment at the top of the info file.
   //---
   g_key_file_set_comment( file ,
                           NULL ,
                           NULL ,
                           "Automatically generated file. Please do not edit." ,
                           NULL );

   //---
   // Define start date and time.
   //---
   eh_require( file )
   {
      time_t clock;
      char date_str[S_LINEMAX];
      char time_str[S_LINEMAX];
      struct tm now;
      GDate *today = g_date_new();

      g_date_set_time( today , time(&clock) );
      localtime_r( &clock , &now );
      g_date_strftime( date_str , S_LINEMAX , "%d/%m/%Y" , today );
      sprintf( time_str , "%d:%d:%d" , now.tm_hour , now.tm_min , now.tm_sec );

      g_key_file_set_value( file , group_name , "RUN START DATE"  , date_str );
      g_key_file_set_value( file , group_name , "RUN START TIME"  , time_str );

      g_date_free( today );
   }

   //---
   // Define the user name, host, and current directory.
   //---
   g_key_file_set_value( file , group_name , "USER" ,  g_get_user_name() );
   g_key_file_set_value( file , group_name , "HOST" , g_get_host_name() );
   g_key_file_set_value( file , group_name , "RUN DIRECTORY" , g_get_current_dir() );

   eh_free( group_name );

   return p;
}

void eh_write_project_info_file( Eh_project proj )
{
   char* info_file = eh_project_info_file_full_name( proj );

   eh_require( info_file )
   {
      char *file_str;
      gsize file_len;
      FILE *fp = eh_fopen( info_file , "w" );

      file_str = g_key_file_to_data( proj->info_file , &file_len , NULL );
      fprintf( fp , "%s" , file_str );

      fclose( fp );
      eh_free( file_str );
   }

   eh_free( info_file );

   return;
}

gboolean eh_read_project_info_file( Eh_project proj )
{
   char *file_name = g_strconcat( proj->name , ".info" , NULL );
   char *info_file = g_build_filename( proj->working_dir_name ,
                                       file_name              ,
                                       NULL );
   gboolean read_ok;

   read_ok = g_key_file_load_from_file( proj->info_file ,
                                        info_file       ,
                                        G_KEY_FILE_NONE ,
                                        NULL );

   eh_free( info_file );
   eh_free( file_name );

   return read_ok;
}

Eh_project eh_load_project( gchar* info_file )
{
   Eh_project p;

   if ( g_file_test( info_file , G_FILE_TEST_EXISTS ) )
   {
      gchar* project_name = g_path_get_basename( info_file );
      gboolean read_ok;

      eh_require( project_name )
      {
         gchar* ext = g_strrstr( project_name , "." );
         if ( ext )
            ext[0] = '\0';
      }

      p = eh_create_project( project_name );

      read_ok = g_key_file_load_from_file( p->info_file    ,
                                           info_file       ,
                                           G_KEY_FILE_NONE ,
                                           NULL );

      if ( !read_ok )
         p = eh_destroy_project( p );

      eh_free( project_name );
   }
   else
      p = NULL;

   return p;
}

Eh_project eh_destroy_project( Eh_project proj )
{
   if ( proj->working_dir )
      g_dir_close( proj->working_dir );
   eh_free( proj->working_dir_name );
   g_key_file_free( proj->info_file );
   eh_free( proj );

   return NULL;
}

int eh_sort_ptr( gconstpointer a , gconstpointer b , gpointer user_data )
{
   return (a<b)?-1:((a==b)?0:1);
}

FILE *eh_open_project_file( Eh_project proj  ,
                            const char *file ,
                            const char *mode )
{
   char *full_name;
   FILE *fp;

   eh_require( proj );
   eh_require( proj->working_dir_name );
   eh_require( file );
   eh_require( mode );

   full_name = g_build_filename( proj->working_dir_name , file , NULL );

   fp = fopen( full_name , mode );
   if ( !fp )
      eh_error( "Unable to open file: %s\n" , file );

   g_tree_insert( proj->open_files , fp , g_strdup( file ) );

   return fp;
}

void eh_close_project_file( Eh_project proj , FILE *fp )
{
   g_tree_remove( proj->open_files , fp );
}

void eh_close_project_file_all( Eh_project proj )
{
   g_tree_destroy( proj->open_files );
}

