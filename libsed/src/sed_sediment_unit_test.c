#include "utils.h"
#include <check.h>

#include "sed_sediment.h"

START_TEST ( test_sed_sediment_new )
{
   {
      Sed_sediment s = sed_sediment_new( );

      fail_if    ( s==NULL                    , "NULL is not a valid sediment" );
      fail_unless( sed_sediment_n_types(s)==0 , "New sediment should have 0 sediment types" );

      sed_sediment_destroy(s);
   }
}
END_TEST

START_TEST ( test_sed_type_new )
{
   {
      Sed_type t = sed_type_new();

      fail_if    ( t==NULL                             , "NULL is not a valid sediment type" );
      fail_unless( fabs(sed_type_grain_size(t))<=1e-12 , "New type should have 0 grain size" );
      fail_unless( fabs(sed_type_density_0(t)) <=1e-12 , "New type should have 0 density" );

      sed_type_destroy( t );
   }
}
END_TEST

START_TEST ( test_sed_type_copy )
{
   {
      Sed_type t_1 = sed_type_new();
      Sed_type t_2 = sed_type_new();
      Sed_type temp;

      sed_type_set_grain_size( t_1 , 1945 );
      sed_type_set_grain_size( t_2 , 1973 );
      temp = sed_type_copy( t_1 , t_2 );

      fail_unless( sed_type_is_same(t_1,t_2) , "Type not copied correctly" );
      fail_unless( temp==t_1                 , "Destination should be returned" );

      sed_type_destroy( t_1 );
      sed_type_destroy( t_2 );
   }

   {
      Sed_type t_1 = sed_type_new();
      Sed_type t_2;

      sed_type_set_grain_size( t_1 , 1945 );
      t_2 = sed_type_copy( NULL , t_2 );

      fail_unless( sed_type_is_same(t_1,t_2) , "NULL destination should duplicate" );
      fail_if    ( t_2==t_1                  , "Should make a copy" );

      sed_type_destroy( t_1 );
      sed_type_destroy( t_2 );
   }
}
END_TEST

START_TEST ( test_sed_sediment_add )
{
   {
      Sed_sediment s = sed_sediment_new();
      Sed_sediment s_0;
      Sed_type     t = sed_type_new();
      Sed_type     t_0;

      sed_type_set_grain_size( t , 142 );

      s_0 = sed_sediment_add_type( s , t );

      fail_unless( sed_sediment_n_types(s)==1 , "Sediment size not increased" );
      fail_unless( s_0==s                     , "Original sediment should be returned" );

      t_0 = sed_sediment_type(s,0);

      fail_if    ( t_0==t                  , "A copy of the type should be added" );
      fail_unless( sed_type_is_same(t,t_0) , "Added type not copied correctly" );

      sed_sediment_destroy( s );
      sed_type_destroy( t );
   }
}
END_TEST

START_TEST ( test_sed_sediment_scan )
{
   {
      Sed_sediment s = sed_sediment_scan( SED_SEDIMENT_TEST_FILE );

      fail_if    ( s==NULL                    , "Invalid sediment scanned" );
      fail_unless( sed_sediment_n_types(s)==5 , "Sediment not scanned properly from file" );

//      sed_sediment_fprint( stdout , s );

      sed_sediment_destroy( s );
   }
}
END_TEST

Suite *sed_sediment_suite( void )
{
   Suite *s = suite_create( "Sed_sediment" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_sed_sediment_new  );
   tcase_add_test( test_case_core , test_sed_sediment_add  );
   tcase_add_test( test_case_core , test_sed_sediment_scan  );

   tcase_add_test( test_case_core , test_sed_type_new  );
   tcase_add_test( test_case_core , test_sed_type_copy );

   return s;
}

int test_sed_sediment( void )
{
   int n;

   {
      Suite *s = sed_sediment_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


