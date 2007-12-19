#include <eh_utils.h>
#include <check.h>

START_TEST (test_create_grid)
{
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );

   fail_if( g==NULL , "Grid set to NULL" );
   fail_unless( eh_grid_n_x(g) == 50  , "Number of x-nodes not set correctly" );
   fail_unless( eh_grid_n_y(g) == 250 , "Number of y-nodes not set correctly" );

   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST (test_destroy_grid)
{
   Eh_dbl_grid g = eh_grid_new( double , 1 , 1 );

   g = eh_grid_destroy( g , TRUE );

   fail_unless( g==NULL , "Destroy function should return NULL" );
}
END_TEST

START_TEST (test_cmp_grid)
{
   gboolean ok;
   Eh_dbl_grid g_1 = eh_grid_new( double , 50 , 250 );
   Eh_dbl_grid g_2;

   eh_dbl_grid_randomize( g_1 );
   g_2 = eh_grid_dup( g_1 );

   ok = eh_grid_cmp_x_data( g_1 , g_2 );
   fail_unless( ok , "Comparison of grid x-data failed" );

   ok = eh_grid_cmp_y_data( g_1 , g_2 );
   fail_unless( ok , "Comparison of grid y-data failed" );

   ok = eh_grid_cmp_data( g_1 , g_2 );
   fail_unless( ok , "Comparison of grid data failed" );

   eh_dbl_grid_set_val( g_2 , 0 , 0 , eh_dbl_grid_val(g_2,0,0)+1e-12 );
   ok = eh_dbl_grid_cmp( g_1 , g_2 , 1e-10 );
   fail_unless( ok , "Comparison of almost equal grids failed" );

   ok = eh_dbl_grid_cmp( g_1 , g_2 , -1 );
   fail_unless( !ok , "Comparison of unequal grids failed" );

   g_1 = eh_grid_destroy( g_1 , TRUE );
   g_2 = eh_grid_destroy( g_2 , TRUE );
}
END_TEST

START_TEST (test_cmp_unequal_grid)
{
   gboolean ok;
   Eh_dbl_grid g_1 = eh_grid_new( double , 50 , 250 );
   Eh_dbl_grid g_2 = eh_grid_new( double , 50 , 50  );

   ok = eh_dbl_grid_cmp( g_1 , g_2 , -1 );

   fail_if( ok , "Comparison of unequal sized grids failed" );

   g_1 = eh_grid_destroy( g_1 , TRUE );
   g_2 = eh_grid_destroy( g_2 , TRUE );
}
END_TEST

START_TEST ( test_zero_create_grid )
{
   Eh_dbl_grid g;

   g = eh_grid_new( double , 0 , 0 );
   fail_if( g==NULL , "Poorly created empty grid" );
   fail_unless( eh_grid_n_x(g)==0     , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_n_y(g)==0     , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_data(g)==NULL , "Memory should not be allocated for empty grid" );
   fail_unless( eh_grid_x(g)==NULL    , "Memory should not be allocated for empty grid" );
   fail_unless( eh_grid_x(g)==NULL    , "Memory should not be allocated for empty grid" );
   eh_grid_destroy( g , TRUE );

   g = eh_grid_new( double , 0 , 250 );
   fail_unless( eh_grid_n_x(g)==0     , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_n_y(g)==250   , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_data(g)==NULL , "Memory should not be allocated for empty grid" );
   fail_unless( eh_grid_x(g)==NULL    , "Memory should not be allocated for empty grid" );
   fail_unless( eh_grid_y(g)!=NULL    , "Memory should be allocated for y-coordinates" );
   eh_grid_destroy( g , TRUE );

   g = eh_grid_new( double , 50 , 0 );
   fail_unless( eh_grid_n_x(g)==50    , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_n_y(g)==0     , "Incorrect dimensions of empty grid" );
   fail_unless( eh_grid_data(g)==NULL , "Memory should not be allocated for empty grid" );
   fail_unless( eh_grid_x(g)!=NULL    , "Memory should be allocated for x-coordinates" );
   fail_unless( eh_grid_y(g)==NULL    , "Memory should not be allocated for empty grid" );
   eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST ( test_negative_create_grid )
{
   Eh_dbl_grid g;

   g = eh_grid_new( double , -1 , 250 );
   fail_unless( g==NULL , "NULL not returned with negative dimension" );

   g = eh_grid_new( double , 50 , -1 );
   fail_unless( g==NULL , "NULL not returned with negative dimension" );

}
END_TEST

START_TEST ( test_set_grid )
{
   double total, n_elem = 50*250;
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );

   eh_dbl_grid_set( g , 1 );

   total = eh_dbl_grid_sum( g );

   fail_unless( fabs(total-n_elem)<1e-16 , "Grid set incorrectly" );

   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST (test_dup_grid)
{
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );
   Eh_dbl_grid h;
   
   eh_dbl_grid_randomize( g );

   h = eh_grid_dup( g );

   fail_unless( eh_dbl_grid_cmp( g , h , 1e-6 ) ,
                "Duplicated grid differs from original" );

   eh_grid_destroy( g , TRUE );
   eh_grid_destroy( h , TRUE );

}
END_TEST

START_TEST ( test_copy_grid )
{
   Eh_dbl_grid src  = eh_grid_new( double , 50 , 250 );
   Eh_dbl_grid dest = eh_grid_new( double , 50 , 250 );

   eh_dbl_grid_randomize( src );

   eh_grid_copy( dest , src );

   fail_unless( eh_grid_cmp_data  ( dest , src ) , "Grid copy failed"   );
   fail_unless( eh_grid_cmp_x_data( dest , src ) , "X-data copy failed" );
   fail_unless( eh_grid_cmp_y_data( dest , src ) , "Y-data copy failed" );

   src  = eh_grid_destroy( src  , TRUE );
   dest = eh_grid_destroy( dest , TRUE );
}
END_TEST

START_TEST (test_sum_grid)
{
   double total;
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );
   
   eh_dbl_grid_set( g , 1. );

   total = eh_dbl_grid_sum( g );

   fail_unless( fabs( total-eh_grid_n_el(g) ) < 1e-6 , "Incorrect sum" );

   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST (test_bad_sum_grid)
{
   int i, j;
   int n_bad_vals = 0;
   double total;
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );
   
   for ( i=0 ; i<50 ; i++ )
      for ( j=0 ; j<250 ; j++ )
      {
         if ( g_random_boolean( ) )
            eh_dbl_grid_set_val( g , i , j , 1. );
         else
         {
            eh_dbl_grid_set_val( g , i , j , -1.+1e-16 );
            n_bad_vals++;
         }
      }

   total = eh_dbl_grid_sum_bad_val( g , -1 );

   fail_unless( fabs( total-(eh_grid_n_el(g)-n_bad_vals) ) < 1e-6 ,
                "Incorrect sum using -1" );

   n_bad_vals = 0;
   for ( i=0 ; i<50 ; i++ )
      for ( j=0 ; j<250 ; j++ )
      {
         if ( g_random_boolean( ) )
            eh_dbl_grid_set_val(g,i,j,1.);
         else
         {
            eh_dbl_grid_set_val(g,i,j,eh_nan());
            n_bad_vals++;
         }
      }

   total = eh_dbl_grid_sum( g );

   fail_unless( fabs( total-(eh_grid_n_el(g)-n_bad_vals) ) < 1e-6 ,
                "Incorrect sum using NaN" );
   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST (test_scalar_mult_grid)
{
   double total_before, total_after;
   double multiplier;
   Eh_dbl_grid g = eh_grid_new( double , 50 , 250 );
   
   multiplier = g_random_double();

   eh_dbl_grid_randomize( g );
   total_before = eh_dbl_grid_sum( g );

   eh_dbl_grid_scalar_mult( g , multiplier );
   total_after = eh_dbl_grid_sum( g );

   fail_unless( fabs( total_before-total_after/multiplier ) < 1e-6 ,
                "Incorrect multiplication" );

   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST ( test_reindex_grid )
{
   Eh_int_grid g = eh_grid_new( int , 50 , 250 );

   eh_int_grid_set_val( g , 0 , 0 , 1 );
   eh_int_grid_set_val( g , eh_grid_n_x(g)-1 , eh_grid_n_y(g)-1  , eh_grid_n_el(g) );

   eh_grid_x(g)[0]                = 1;
   eh_grid_x(g)[eh_grid_n_x(g)-1] = eh_grid_n_x(g);

   eh_grid_y(g)[0]                = 1;
   eh_grid_y(g)[eh_grid_n_y(g)-1] = eh_grid_n_y(g);

   eh_grid_reindex( g , -5 , -10 );

   fail_unless( eh_int_grid_val(g,-5,-10) == 1 , "Lower limit of data not reindexed" );
   fail_unless( eh_grid_x(g)[-5]          == 1 , "Lower limit of x-data not reindexed" );
   fail_unless( eh_grid_y(g)[-10]         == 1 , "Lower limit of y-data not reindexed" );

   {
      gssize i = -5  + eh_grid_n_x(g) - 1;
      gssize j = -10 + eh_grid_n_y(g) - 1;

      fail_unless( eh_int_grid_val(g,i,j) == eh_grid_n_el(g) ,
                   "Upper limit of data not reindexed" );
      fail_unless( eh_grid_x(g)[i]  == eh_grid_n_x(g) ,
                   "Upper limit of x-data not reindexed" );
      fail_unless( eh_grid_y(g)[j] == eh_grid_n_y(g) ,
                   "Upper limit of y-data not reindexed" );
   }

   g = eh_grid_destroy( g , TRUE );
}
END_TEST

START_TEST ( test_reduce_grid )
{
   double initial_sum, final_sum;
   Eh_dbl_grid g = eh_grid_new( double , 1 , 50 );
   Eh_dbl_grid small_g;

   g = eh_dbl_grid_randomize( g );
   g = eh_dbl_grid_set( g , 1. );
   initial_sum = eh_dbl_grid_sum( g );

   small_g   = eh_dbl_grid_reduce( g , 1 , 10 );
   final_sum = eh_dbl_grid_sum( small_g );

   fail_unless( small_g!=NULL            , "NULL grid was returned"                );
   fail_unless( eh_grid_n_x(small_g)==1  , "x-direction was not reduced correctly" );
   fail_unless( eh_grid_n_y(small_g)==10 , "y-direction was not reduced correctly" );
   fail_if    ( final_sum==0             , "NaNs found in reduced grid"            );
//eh_dbl_grid_fprintf( stderr , "%f " , small_g );
   fail_unless( fabs(initial_sum-5*final_sum)<1e-3 , "Mass lost in grid reduction" );

   g       = eh_grid_destroy( g       , TRUE );
   small_g = eh_grid_destroy( small_g , TRUE );
}
END_TEST

START_TEST ( test_rebin_grid )
{
   Eh_dbl_grid x, x_new;
   double sum, sum_new;
   gssize i, j;

   x     = eh_grid_new( double , 93 , 109  );
   x_new = eh_grid_new( double ,  2 , 1000 );

   for ( i=0 ; i<eh_grid_n_x(x) ; i++ )
      eh_grid_x(x)[i] = 33*i;

   for ( j=0 ; j<eh_grid_n_y(x) ; j++ )
      eh_grid_y(x)[j] = 300*j;

   eh_dbl_grid_set( x , 1 );

   for ( i=0 ; i<eh_grid_n_x(x_new) ; i++ )
      eh_grid_x(x_new)[i] = 10000*i;

   for ( j=0 ; j<eh_grid_n_y(x_new) ; j++ )
      eh_grid_y(x_new)[j] = 500*j;

   eh_dbl_grid_rebin_bad_val( x , x_new , 0 );

   sum     = eh_dbl_grid_sum( x )*33*300;
   sum_new = eh_dbl_grid_sum( x_new )*10000*500;
/*
   eh_dbl_grid_fprintf( stderr , "%f " , x_new );
   eh_watch_dbl( sum );
   eh_watch_dbl( sum_new );
*/

   fail_unless( fabs(sum_new-sum)/sum < 1e-2 , "Grid not rebinned correctly" );

   eh_grid_destroy( x     , TRUE );
   eh_grid_destroy( x_new , TRUE );

}
END_TEST

START_TEST ( test_line_path )
{
   Eh_grid g = eh_grid_new( double , 10 , 10  );
   gssize* path;

   {
      path = eh_dbl_grid_line_ids( g , 2 , 2 , 2 , 2 );
      fail_unless( path==NULL , "NULL if start and end are the same point" );
   }

   {
      gssize ans[7] = { 0 , 1 , 2 , 3 , 4 , 5 , -1 };
      path = eh_dbl_grid_line_ids( g , 0 , 0 , 0 , 5 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   {
      gssize ans[7] = { 0 , 11 , 22 , 33 , 44 , 55 , -1 };
      path = eh_dbl_grid_line_ids( g , 0 , 0 , 5 , 5 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   {
      gssize ans[7] = { 0 , 10 , 20 , 30 , 40 , 50 , -1 };
      path = eh_dbl_grid_line_ids( g , 0 , 0 , 5 , 0 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   {
      gssize ans[7] = { 5 , 4 , 3 , 2 , 1 , 0 , -1 };
      path = eh_dbl_grid_line_ids( g , 0 , 5 , 0 , 0 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   {
      gssize ans[7] = { 50 , 40 , 30 , 20 , 10 , 0 , -1 };
      path = eh_dbl_grid_line_ids( g , 5 , 0 , 0 , 0 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   {
      gssize ans[10] = { 0 , 10 , 21 , 31 , 42 , 53 , -1 };
      path = eh_dbl_grid_line_ids( g , 0 , 0 , 5 , 3 );

      fail_if( path==NULL , "Path should be non-NULL" );
      fail_unless( eh_grid_path_is_same(path,ans) , "Incorrect path" );

      eh_free( path );
   }

   eh_grid_destroy( g , TRUE );
}
END_TEST

Suite *grid_suite( void )
{
   Suite *s = suite_create( "Grid" );
   TCase *test_case_core = tcase_create( "Core" );
   TCase *test_case_set  = tcase_create( "Set" );

   suite_add_tcase( s , test_case_core );
   suite_add_tcase( s , test_case_set  );

   tcase_add_test( test_case_core , test_create_grid          );
   tcase_add_test( test_case_core , test_destroy_grid         );
   tcase_add_test( test_case_core , test_set_grid             );
   tcase_add_test( test_case_core , test_zero_create_grid     );
   tcase_add_test( test_case_core , test_negative_create_grid );

   tcase_add_test( test_case_set  , test_dup_grid             );
   tcase_add_test( test_case_set  , test_copy_grid            );
   tcase_add_test( test_case_set  , test_sum_grid             );
   tcase_add_test( test_case_set  , test_bad_sum_grid         );
   tcase_add_test( test_case_set  , test_scalar_mult_grid     );
   tcase_add_test( test_case_set  , test_cmp_grid             );
   tcase_add_test( test_case_set  , test_cmp_unequal_grid     );
   tcase_add_test( test_case_set  , test_reindex_grid         );
   tcase_add_test( test_case_set  , test_reduce_grid          );
   tcase_add_test( test_case_set  , test_rebin_grid           );
//   tcase_add_test( test_case_set  , test_line_path            );

   return s;
}

int test_grid( void )
{
   int n;

   {
      Suite *s = grid_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}

