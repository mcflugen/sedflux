#include <string.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include <check.h>
#include <time.h>

void subside_grid_load( Eh_dbl_grid w , Eh_dbl_grid v_0 , double eet , double y );

Eh_dbl_grid
subside_test( const gint n_x, const gint n_y )
{
   Eh_dbl_grid w   = eh_grid_new( double, n_x, n_y );
   Eh_dbl_grid v_0 = eh_grid_new( double, n_x, n_y );
   double eet      = 5000.;
   double y        = 7e10;

   eh_dbl_grid_set( w  , 0.    );
   eh_dbl_grid_set( v_0, 5000. );

   subside_grid_load( w, v_0, eet, y );

   eh_grid_destroy( v_0, TRUE );

   return w;
}

START_TEST ( test_subside_0 )
{
   Eh_dbl_grid w;
   double total;
   gint i;
   gint n[] = { 16, 32, 64, 128, 256, 512, 1024 };
   gint len = sizeof(n)/sizeof(gint);
   clock_t start, end;
   time_t t_start, t_end;
   double elapsed;
   gchar* key[] = {"8.16283e-06", "0.000130605", "0.00208968", "0.0334349", "0.534959", "8.55935", "136.95"};
   gchar* ans;

   for ( i=0; i<len; i++ )
   {
      eh_message( "Grid size: %d, %d", n[i], n[i] );

      start = clock();
      t_start = time(NULL);
      w = subside_test( n[i], n[i] );
      end  = clock();
      t_end  = time(NULL);

      elapsed = ( (double)(end-start) ) / CLOCKS_PER_SEC;
      eh_message( "Elapsed cpu time: %f", elapsed );
      eh_message( "Elapsed wall clock time: %ld", (long)(t_end-t_start) );

      total = eh_dbl_grid_sum( w );

      ans = g_strdup_printf( "%g", total );
      fail_unless( strcmp(ans,key[i])==0,
                   "Deflection does not match benchmark." );
      eh_free( ans );

      eh_grid_destroy( w, TRUE );
   }
}
END_TEST

Suite*
sed_subside_suite( void )
{
   Suite *s = suite_create( "Subsidence" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_set_timeout( test_case_core, 0 );
   tcase_add_test( test_case_core , test_subside_0  );

   return s;
}

int main( void )
{
   int n;

   eh_init_glib();

   {
      Suite *s = sed_subside_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}

