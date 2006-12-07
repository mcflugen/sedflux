#include "sed_cell.h"
#include "utils.h"
#include <check.h>

START_TEST ( test_cell_new )
{
   {
      Sed_cell c = sed_cell_new( 5 );
      fail_if    ( c==NULL                         , "NULL returned instead of a new cell"  );
      fail_unless( sed_cell_n_types(c)==5          , "Incorrect number of grain types"      );
      fail_unless( sed_cell_is_empty(c)            , "Newly created cell should be empty"   );
      fail_unless( fabs( sed_cell_age(c) ) < 1e-12 , "Newly created cell should have 0 age" );
      fail_unless( sed_cell_is_valid(c)            , "Newly created cell should be valid"   );

      sed_cell_destroy( c );
   }

   {
      Sed_cell c = sed_cell_new( 0 );

      fail_unless( c==NULL , "A cell of 0 size is not allowed" );
      
      sed_cell_destroy( c );
   }

   {
      Sed_cell c = sed_cell_new( -1 );

      fail_unless( c==NULL , "A cell of negative size is not allowed" );
      
      sed_cell_destroy( c );
   }

}
END_TEST

START_TEST (test_cell_destroy)
{
   {
      Sed_cell c = sed_cell_new( 1 );

      c = sed_cell_destroy( c );

      fail_unless( c==NULL , "Destroy function should return NULL" );
   }
}
END_TEST

START_TEST (test_cell_cmp)
{
   gboolean same;

   {
      Sed_cell a = sed_cell_new( 3 );

      same = sed_cell_is_same( a , a );

      fail_unless( same , "Cell comparison failed" );

      sed_cell_destroy( a );
   }

   {
      Sed_cell a = sed_cell_new( 3 );
      Sed_cell b = sed_cell_new( 1 );

      same = sed_cell_is_same( a , b );

      fail_unless( !same , "Cells must have same size" );
      
      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      Sed_cell a = sed_cell_new( 3 );
      Sed_cell b = sed_cell_new( 3 );

      sed_cell_resize( a , 1. );
      sed_cell_resize( b , 2. );

      same = sed_cell_is_same( a , b );

      fail_unless( !same , "Cells must have same thickness" );
      
      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

}
END_TEST

START_TEST (test_cell_copy)
{

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b;

      sed_cell_resize( a , 100 );

      b = sed_cell_copy( NULL , a );

      fail_if    ( a == b            , "Duplicate cell should not be the original" );
      fail_unless( sed_cell_is_same(a,b) , "Failed to duplicate cell" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b = sed_cell_new( 5 );

      sed_cell_resize( a , 100 );

      sed_cell_copy( b , a );

      fail_unless( sed_cell_is_same(a,b) , "Failed to copy cell" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

}
END_TEST

START_TEST ( test_cell_clear )
{
   {
      Sed_cell c = sed_cell_new( 5 );

      fail_unless( sed_cell_clear(c)==c , "Should return the cleared cell" );

      sed_cell_destroy( c );
   }
   {
      Sed_cell c = sed_cell_new( 5 );
      double f[5] = { .2 , .2 , .2 , .2 , .2 };

      sed_cell_resize ( c , 10 );
      sed_cell_set_age( c , 33 );
      sed_cell_set_fraction( c , f );

      sed_cell_clear( c );

      fail_unless( sed_cell_is_empty(c)         , "Thickness not cleared" );
      fail_unless( fabs(sed_cell_age(c)) <1e-12 , "Age not cleared"       );
   }
}
END_TEST

START_TEST ( test_cell_set_and_get_functions )
{
   {
      gssize n;
      Sed_cell c = sed_cell_new( 5 );
      double f[5] = { .2 , .2 , .2 , .2 , .2 };

      sed_cell_resize( c , 2 );
      fail_unless( sed_cell_is_size(c,2)              , "Size not set correctly" );
      fail_unless( fabs(sed_cell_size_0(c)-2) < 1e-12 , "Original size not set correctly" );

      sed_cell_set_age( c , 33 );
      fail_unless( fabs(sed_cell_age(c)-33) < 1e-12 , "Age not set correctly" );

      sed_cell_set_facies( c , S_FACIES_PLUME );
      fail_unless( sed_cell_facies(c)==S_FACIES_PLUME , "Facies not set correctly" );

      sed_cell_set_facies( c , S_FACIES_WAVE );
      fail_unless( sed_cell_facies(c)==S_FACIES_WAVE , "Facies not reset correctly" );

      sed_cell_add_facies( c , S_FACIES_RIVER );
      fail_unless( sed_cell_facies(c)&S_FACIES_WAVE  , "Adding a facies should keep original" );
      fail_unless( sed_cell_facies(c)&S_FACIES_RIVER , "New facies not added correctly" );

      sed_cell_set_fraction( c , f );
      f[0] = 1;
      fail_if( fabs( sed_cell_fraction(c,0)-1. ) < 1e-12 , "Values should be copied" );
      for ( n=0 ; n<sed_cell_n_types(c) ; n++ )
         fail_unless( fabs( sed_cell_fraction(c,n)-.2 ) < 1e-12 , "Fraction not set correctly" );

      sed_cell_set_pressure( c , 1000 );
      fail_unless( fabs(sed_cell_pressure(c)-1000) < 1e-12 , "Pressure not set correctly" );

      sed_cell_destroy( c );
   }
}
END_TEST

START_TEST ( test_cell_add )
{

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b = sed_cell_new( 5 );

      sed_cell_set_equal_fraction( a );
      sed_cell_resize( a , 1 );

      sed_cell_add( b , a );

      fail_unless( sed_cell_is_same(a,b) , "Adding to an empty cell should produce a copy" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b = sed_cell_new( 5 );

      sed_cell_set_equal_fraction( a );
      sed_cell_resize( a , 1 );
      sed_cell_set_age( a , 1 );

      sed_cell_set_equal_fraction( b );
      sed_cell_resize( b , 2 );
      sed_cell_set_age( b , 4 );

      sed_cell_add( b , a );

      fail_unless(  sed_cell_is_size(b,3)            , "New size should be the sum of cell sizes" );
      fail_if    ( !sed_cell_is_size(a,1)            , "Size of the original cell shouldn't change" );
      fail_unless( fabs(sed_cell_age (b)-3.) < 1e-12 , "New age should be a weighted average" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }
}
END_TEST

START_TEST ( test_cell_resize )
{
   {
      gssize n;
      Sed_cell c = sed_cell_new( 5 );

      sed_cell_set_age     ( c , 10 );
      sed_cell_set_pressure( c , 10 );
      sed_cell_set_facies  ( c , S_FACIES_PLUME );

      sed_cell_set_equal_fraction( c );

      sed_cell_resize( c , 1 );
      fail_unless( fabs(sed_cell_size  (c)-1.) < 1e-12 , "Size not initialized correctly" );
      fail_unless( fabs(sed_cell_size_0(c)-1.) < 1e-12 , "Original size not initialized correctly" );

      sed_cell_resize( c , .5 );
      fail_unless( fabs(sed_cell_size  (c)-.5) < 1e-12 , "Size not reset correctly" );
      fail_unless( fabs(sed_cell_size_0(c)-.5) < 1e-12 , "Resize must maintain degree of compaction" );

      sed_cell_compact( c , .25 );
      fail_unless( fabs(sed_cell_size  (c)-.25) < 1e-12 , "Cell not compacted correctly" );
      fail_unless( fabs(sed_cell_size_0(c)-.5 ) < 1e-12 , "Compaction must not affect original size" );

      fail_unless( fabs( sed_cell_age(c)-10. ) < 1e-12      , "Resizing should not change age" );
      fail_unless( fabs( sed_cell_pressure(c)-10. ) < 1e-12 , "Resizing should not change pressure" );
      fail_unless( sed_cell_facies(c)==S_FACIES_PLUME       , "Resizing should not change facies" );

      for ( n=0 ; n<sed_cell_n_types(c) ; n++ )
         fail_unless( fabs(sed_cell_fraction(c,n)-.2)<1e-12 , "Resizing should not change fraction" );

      sed_cell_destroy( c );
   }
}
END_TEST

START_TEST ( test_cell_resize_neg )
{
   Sed_cell c = sed_cell_new( 5 );

   sed_cell_resize( c , -1 );

   fail_unless( sed_cell_is_clear(c) , "Negative resize should clear cell" );

   sed_cell_destroy( c );
}
END_TEST

START_TEST ( test_cell_resize_zero )
{
   Sed_cell c = sed_cell_new( 5 );

   sed_cell_resize( c , 0 );

   fail_unless( sed_cell_is_clear(c) , "Zero resize should clear cell" );

   sed_cell_destroy( c );
}
END_TEST

START_TEST ( test_cell_separate_cell )
{
   {
      double f_0[5] = { 1. , 0 , 0 , 0 , 0 };
      double   f[5] = { .2 , .2 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 5 , f   );
      Sed_cell b = sed_cell_new_sized( 5 , 1 , f_0 );
      Sed_cell b_copy;

      b_copy = sed_cell_dup( b );

      sed_cell_separate_cell( a , b );

      fail_unless( sed_cell_is_same(b_copy,b)                 , "Right-hand cell should not change" );
      fail_unless( sed_cell_is_size(a,4)                  , "Separated cell not resized correctly" );
      fail_unless( fabs( sed_cell_fraction(a,0) ) < 1e-12 , "Incorrect sediment type removed" );
      fail_unless( sed_cell_is_valid(a)                   , "Separation must produce a valid cell" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( b_copy );
   }
}
END_TEST

START_TEST ( test_cell_separate )
{
   {
      double f[5] = { .2 , .2 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 1 , f );
      Sed_cell b;

      b = sed_cell_separate( a , f , .25 , NULL );

      fail_unless( sed_cell_is_valid(b)    , "Separated cell must be a valid cell"   );
      fail_unless( sed_cell_is_size(a,.75) , "Original cell not separated correctly" );
      fail_unless( sed_cell_is_size(b,.25) , "Separated cell not sized correctly"    );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      double f[5] = { .2 , .2 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 1 , f );
      Sed_cell b = sed_cell_new_sized( 5 , 2 , f );
      Sed_cell b_copy = b;
      Sed_cell a_after;
      Sed_cell b_after;

      sed_cell_set_age   ( a , 1              );
      sed_cell_set_facies( a , S_FACIES_WAVE  );
      sed_cell_set_age   ( b , 10             );
      sed_cell_set_facies( b , S_FACIES_PLUME );

      a_after = sed_cell_dup( a );
      b_after = sed_cell_dup( a );

      sed_cell_resize( a_after , .75 );
      sed_cell_resize( b_after , .25 );

      b = sed_cell_separate( a , f , .25 , b );

      fail_if    ( b!=b_copy               , "Separated cell should be provided cell" );
      fail_unless( sed_cell_is_valid(b)    , "Separated cell must be a valid cell" );
      fail_unless( sed_cell_is_same(a,a_after) , "Original cell not separated correctly" );
      fail_unless( sed_cell_is_same(b,b_after) , "Separated cell not separated correctly" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( a_after );
      sed_cell_destroy( b_after );
   }
}
END_TEST

START_TEST ( test_cell_separate_thickness )
{
   double f[5] = { .2 , .2 , .2 , .2 , .2 };
   Sed_cell a = sed_cell_new_sized( 5 , 1 , f );

   {
      Sed_cell b;
      Sed_cell a_after;
      Sed_cell b_after;

      sed_cell_set_age   ( a , 1              );
      sed_cell_set_facies( a , S_FACIES_WAVE  );

      a_after = sed_cell_dup( a );
      b_after = sed_cell_dup( a );

      sed_cell_resize( a_after , .25 );
      sed_cell_resize( b_after , .75 );

      b = sed_cell_separate_thickness( a , .75 , NULL );

      fail_unless( sed_cell_is_valid(b)        , "Separated cell must be a valid cell" );
      fail_unless( sed_cell_is_same(a,a_after) , "Original cell not separated correctly" );
      fail_unless( sed_cell_is_same(b,b_after) , "Separated cell not separated correctly" );

      sed_cell_resize( a_after , .0  );
      sed_cell_resize( b_after , .25 );

      b = sed_cell_separate_thickness( a , .75 , NULL );

      fail_unless( sed_cell_is_valid(b)        , "Separated cell must be a valid cell" );
      fail_unless( sed_cell_is_same(a,a_after) , "Original cell not separated correctly" );
      fail_unless( sed_cell_is_same(b,b_after) , "Separated cell not separated correctly" );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( a_after );
      sed_cell_destroy( b_after );
   }
}
END_TEST

START_TEST ( test_cell_separate_fraction )
{
   {
      double f[5] = { 1. , 0  , .5 , .5  , 0  };
      double f_0[5] = { .4 , .0 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 1 , f_0 );
      Sed_cell b;

      b = sed_cell_separate_fraction( a , f , NULL );

      fail_unless( sed_cell_is_valid(a)    , "Separated cell must be a valid cell" );
      fail_unless( sed_cell_is_valid(b)    , "Separated cell must be a valid cell" );
      fail_unless( sed_cell_is_size(a,.4)  , "Original cell not sized correctly"   );
      fail_unless( sed_cell_is_size(b,.6)  , "Separated cell not sized correctly"  );

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }
}
END_TEST

START_TEST ( test_cell_is_valid )
{
   {
      Sed_cell a = sed_cell_new( 3 );
      double f[3] = { 0. , 0. , 0. };

      fail_unless( !sed_cell_is_valid(NULL) , "NULL is an invalid cell" );

      sed_cell_resize( a , 0 );
      sed_cell_set_fraction( a , f );
      fail_unless( sed_cell_is_valid(a) , "A cell with no size can be non-normalized" );

      sed_cell_resize( a , 1 );
      fail_unless( !sed_cell_is_valid(a) , "A cell with size must be normalized" );

      f[0] = 1, f[1] = 1, f[2] = 1;
      sed_cell_set_fraction( a , f );
      fail_unless( !sed_cell_is_valid(a) , "Fractions must sum to 1" );

      f[0] = -1;
      fail_unless( !sed_cell_is_valid(a) , "Fractions must be positive" );

      sed_cell_destroy( a );
   }

   {
      Sed_cell a = sed_cell_new( 3 );

      fail_unless( sed_cell_is_valid(a) , "A newly created empty cell is valid" );

      sed_cell_destroy( a );
   }

   {
      double f[3] = { .75 , .25 , 0 };
      Sed_cell a = sed_cell_new_sized( 3 , 2 , f );

      fail_unless( sed_cell_is_valid(a) , "A newly created filled cell is valid" );

      sed_cell_destroy( a );
   }
}
END_TEST

Suite *sed_cell_suite( void )
{
   Suite *s = suite_create( "Sed_cell" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_cell_new          );
   tcase_add_test( test_case_core , test_cell_destroy);
   tcase_add_test( test_case_core , test_cell_cmp);
   tcase_add_test( test_case_core , test_cell_copy);
   tcase_add_test( test_case_core , test_cell_clear );
   tcase_add_test( test_case_core , test_cell_set_and_get_functions );
   tcase_add_test( test_case_core , test_cell_add );
   tcase_add_test( test_case_core , test_cell_resize );
   tcase_add_test( test_case_core , test_cell_resize_neg );
   tcase_add_test( test_case_core , test_cell_resize_zero );
   tcase_add_test( test_case_core , test_cell_separate_cell );
   tcase_add_test( test_case_core , test_cell_separate );
   tcase_add_test( test_case_core , test_cell_separate_thickness );
   tcase_add_test( test_case_core , test_cell_separate_fraction );
   tcase_add_test( test_case_core , test_cell_is_valid );


   return s;
}

int test_sed_cell( void )
{
   int n;

   {
      Suite *s = sed_cell_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


