#include <eh_utils.h>
#include <check.h>

Eh_project create_test_project( )
{
   char* proj_dir_name  = "/tmp/project_test";
   char* proj_full_name = tempnam( proj_dir_name , "TEST_" );
   char* proj_name      = g_path_get_basename( proj_full_name );
   Eh_project p         = eh_create_project( proj_name );

   eh_set_project_dir( p , proj_dir_name );

//   eh_free( proj_dir_name  );
//   eh_free( proj_full_name );
//   eh_free( proj_name      );

   return p;
}

START_TEST ( test_create_project )
{
   Eh_project p;

   p = eh_create_project( "TEST_PROJECT" );

   fail_unless( strcmp(eh_project_name(p),"TEST_PROJECT")==0 ,
                "Wrong project name" );

   fail_unless( eh_project_dir(p)==NULL                      ,
                "Directory should be set to NULL" );

   fail_unless( eh_project_dir_name(p)==NULL                 ,
                "Directory name should be set to NULL" );

   p = eh_destroy_project( p );
}
END_TEST

START_TEST ( test_set_project_dir )
{
   Eh_project p;
   FILE *fp;
   int n;

   p = eh_create_project( "TEST_PROJECT" );

   eh_set_project_dir( p , "/tmp/project_test" );

   fp = fopen( "/tmp/project_test/test.txt" , "w" );
   fail_if( fp==NULL , "Error opening file in working directory" );

   n = fprintf( fp , "This is a test" );
   fail_unless( n==14 , "Error writing to project file" );

   fclose( fp );

   p = eh_destroy_project( p );
}
END_TEST

START_TEST ( test_project_add_info_entry )
{
   gchar* info_file_name;
   gchar* proj_name;

   {
      Eh_project p = create_test_project( proj_name );

      eh_project_add_info_val( p , "TEST KEY" , "TEST VALUE" );

      eh_project_add_info_val( p , "TEST KEY WITH EQUAL" , "x=1" );

      eh_project_add_info_val( p , "KEY LIST" , "1" );
      eh_project_add_info_val( p , "KEY LIST" , "2" );
      eh_project_add_info_val( p , "KEY LIST" , "3" );

      eh_write_project_info_file( p );

      info_file_name = eh_project_info_file_full_name( p );
      proj_name      = g_strdup( eh_project_name(p) );


      p = eh_destroy_project( p );
   }

   {
      GKeyFile* info_file = g_key_file_new();
      char* group_name    = g_strconcat( proj_name     ,
                                         " info entry" ,
                                         NULL );
      char* value;
      gint* list;
      gsize len;

      g_key_file_load_from_file( info_file            ,
                                 info_file_name  ,
                                 G_KEY_FILE_NONE      ,
                                 NULL );

      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "TEST KEY"       ,
                                       NULL ) ,
                   "Missing key 'TEST KEY'" );
      value = g_key_file_get_value( info_file  ,
                                    group_name ,
                                    "TEST KEY" ,
                                    NULL );
      fail_unless( strcmp( value , "TEST VALUE" )==0 ,
                   "Value written incorrectly" );

      eh_free( value );

      fail_unless( g_key_file_has_key( info_file             ,
                                       group_name            ,
                                       "TEST KEY WITH EQUAL" ,
                                       NULL ) ,
                   "Missing key 'TEST KEY WITH EQUAL'" );
      value = g_key_file_get_value( info_file  ,
                                    group_name ,
                                    "TEST KEY WITH EQUAL" ,
                                    NULL );
      fail_unless( strcmp( value , "x=1" )==0 ,
                   "Value written incorrectly" );

      eh_free( value );

      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "KEY LIST"       ,
                                       NULL ) ,
                   "Missing key 'KEY LIST'" );

      list = g_key_file_get_integer_list( info_file  ,
                                          group_name ,
                                          "KEY LIST" ,
                                          &len       ,
                                          NULL );

      fail_unless( len==3     , "List length is incorrect" );
      fail_unless( list[0]==1 , "Fist element set incorrectly" );
      fail_unless( list[1]==2 , "Second element set incorrectly" );
      fail_unless( list[2]==3 , "Third element set incorrectly" );

      eh_free( list                );
      eh_free( group_name          );

      g_key_file_free( info_file );
   }

   eh_free( info_file_name );
   eh_free( proj_name      );
}
END_TEST

START_TEST ( test_project_info_file )
{
   gchar* info_file_name;
   gchar* proj_name;

   {
      Eh_project p = create_test_project( );

      eh_write_project_info_file( p );

      info_file_name = eh_project_info_file_full_name( p );
      proj_name      = g_strdup( eh_project_name(p) );

      p = eh_destroy_project( p );
   }

   {
      GKeyFile* info_file = g_key_file_new();
      char* group_name    = g_strconcat( proj_name     ,
                                         " info entry" ,
                                         NULL );

      fail_unless( g_file_test( info_file_name , G_FILE_TEST_EXISTS ) ,
                   "Info file does not exist where it should" );

      fail_unless( g_key_file_load_from_file( info_file            ,
                                              info_file_name  ,
                                              G_KEY_FILE_NONE      ,
                                              NULL ) ,
                   "Error loading info file" );

      fail_unless( g_key_file_has_group( info_file , group_name ) ,
                  "Group (%s) is missing" , group_name );

      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "RUN START DATE" ,
                                       NULL ) ,
                   "Missing key 'RUN START DATE'" );
      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "RUN START TIME" ,
                                       NULL ) ,
                   "Missing key 'RUN START TIME'" );
      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "USER"           ,
                                       NULL ) ,
                   "Missing key 'USER'" );
      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "HOST"           ,
                                       NULL ) ,
                   "Missing key 'HOST'" );
      fail_unless( g_key_file_has_key( info_file        ,
                                       group_name       ,
                                       "RUN DIRECTORY"  ,
                                       NULL ) ,
                   "Missing key 'RUN DIRECTORY'" );

      eh_free( group_name          );

      g_key_file_free( info_file );
   }

   eh_free( info_file_name );
   eh_free( proj_name      );
}
END_TEST

START_TEST ( test_project_load_info_file )
{
   gchar* info_file;

   {
      Eh_project p = create_test_project( );

      eh_project_add_info_val( p , "TEST KEY" , "TEST VALUE" );

      eh_write_project_info_file( p );

      info_file = eh_project_info_file_full_name( p );

      p = eh_destroy_project( p );
   }

   {
      Eh_project p = eh_load_project( "WRONG NAME" );

      fail_unless( p==NULL , "eh_load_project should return NULL on error." );
   }

   {
      Eh_project p = eh_load_project( info_file );
      gchar* value;

      fail_if( p==NULL , "Info file did not load correctly" );

      value = eh_project_get_info_val( p , "TEST KEY" );

      fail_unless( strcmp( value , "TEST VALUE" )==0 ,
                   "Value loaded incorrectly" );

      p = eh_destroy_project( p );

      eh_free( value );
   }

   eh_free( info_file );

}
END_TEST

START_TEST ( test_project_set_info_val )
{
   Eh_project p;
   gchar* value;

   p = create_test_project( );

   eh_project_set_info_val( p , "TEST VAL" , "1" );

   value = eh_project_get_info_val( p , "TEST VAL" );

   fail_unless( strcmp( value , "1" )==0 ,
                "Info value set incorrectly" );

   eh_free( value );

   eh_project_set_info_val( p , "TEST VAL" , "2" );

   value = eh_project_get_info_val( p , "TEST VAL" );

   fail_unless( strcmp( value , "2" )==0 ,
                "Info value reset incorrectly" );

   eh_free( value );

   eh_destroy_project( p );

}
END_TEST

START_TEST ( test_project_get_info_val )
{
   Eh_project p;
   gchar* value;
   const gchar* name = g_get_user_name();

   p = create_test_project( );

   value = eh_project_get_info_val( p , "GARBAGE" );

   fail_unless( value==NULL , "Value should be NULL if key doesn't exist." );

   value = eh_project_get_info_val( p , "USER" );

   fail_unless( strcmp( value , name )==0 ,
                "Incorrect info value retrieved." );

   eh_free( value );

   eh_destroy_project( p );
}
END_TEST

Suite *project_suite( void )
{
   Suite *s = suite_create( "Project" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_create_project );
   tcase_add_test( test_case_core , test_set_project_dir );
   tcase_add_test( test_case_core , test_project_info_file );
   tcase_add_test( test_case_core , test_project_add_info_entry );
   tcase_add_test( test_case_core , test_project_load_info_file );
   tcase_add_test( test_case_core , test_project_set_info_val );
   tcase_add_test( test_case_core , test_project_get_info_val );

   return s;
}

int test_project( void )
{
   int n;

   {
      Suite *s_project = project_suite();
      SRunner *sr_project = srunner_create( s_project );

      srunner_run_all( sr_project , CK_NORMAL );
      n = srunner_ntests_failed( sr_project );
      srunner_free( sr_project );
   }

   return n;
}


