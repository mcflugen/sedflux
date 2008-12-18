#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <check.h>

void
subside_grid_load( Eh_dbl_grid w , Eh_dbl_grid v_0 , double eet , double y )
START_TEST ( test_compact_0 )
{
   Eh_dbl_grid w   = eh_grid_new( double, 100, 100 );
   Eh_dbl_grid v_0 = eh_grid_new( double, 100, 100 );
   double eet      = 5000.;
   double y        = 7e10;

   eh_dbl_grid_set( w  , 0.    );
   eh_dbl_grid_set( v_0, 5000. );

   subside_grid_load( w, v_0, eet, y );

   eh_grid_destroy( w,   TRUE );
   eh_grid_destroy( v_0, TRUE );
}
END_TEST


Suite*
sed_subside_suite( void )
{
   Suite *s = suite_create( "Subsidence" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_subside_0  );

   return s;
}

int main( void )
{
   int n;

   eh_init_glib();

   {
      Suite *s = sed_subsidence_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


