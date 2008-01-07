#include <eh_utils.h>
#include <check.h>

START_TEST ( test_create_symbol_table )
{
   Eh_symbol_table t;

   t = eh_symbol_table_new( );

   fail_unless( t>0 , "Bad symbol table handle" );
   fail_unless( eh_symbol_table_size(t)==0 , "Empty symbol table should be size 0" );

   t = eh_symbol_table_destroy( t );

   fail_unless( t==NULL , "Destroyed symbol table should be set to NULL" );
}
END_TEST

START_TEST ( test_add_value )
{
   Eh_symbol_table t;
   gchar* label = g_strdup( "test label" );
   gchar* value = g_strdup( "test value" );
   gchar* stored_value;

   t = eh_symbol_table_new( );

   eh_symbol_table_insert( t , label , value );

   fail_unless( eh_symbol_table_size(t)==1 , "Symbol table should be size 1" );
   fail_unless( eh_symbol_table_has_label(t,label) , "Added label not found" );

   stored_value = eh_symbol_table_value( t , label );

   fail_unless( stored_value!=value , "A copy of the value should be returned" );
   fail_unless( g_ascii_strcasecmp(stored_value,value)==0 , "Added value not returned" );

   eh_free( stored_value );

   stored_value = eh_symbol_table_lookup( t , label );
   fail_unless( stored_value!=value , "A copy of the value should be stored" );

   t = eh_symbol_table_destroy( t );
}
END_TEST

Suite *eh_symbol_table_suite( void )
{
   Suite *s = suite_create( "Eh_symbol_table" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_create_symbol_table );
   tcase_add_test( test_case_core , test_add_value );

   return s;
}

int test_symbol_table( void )
{
   int n;

   {
      Suite *s = eh_symbol_table_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}

