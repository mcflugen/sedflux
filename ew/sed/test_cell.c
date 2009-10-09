#include <utils/utils.h>
#include <sed_sedflux.h>
#include <sed_cell.h>
#include <glib.h>

#include "test_sed.h"

void
test_cell_new (void)
{
   {
      Sed_cell c = sed_cell_new( 5 );
      g_assert (c!=NULL);
      g_assert_cmpint (sed_cell_n_types (c), ==, 5);
      g_assert (sed_cell_is_empty (c));
      g_assert (eh_compare_dbl (sed_cell_age (c), 0., 1e-12));
      g_assert (sed_cell_is_valid (c));

      sed_cell_destroy( c );
   }

   {
      Sed_cell c = sed_cell_new( 0 );
      g_assert (c==NULL);
      sed_cell_destroy( c );
   }

   {
      Sed_cell c = sed_cell_new( -1 );
      g_assert (c==NULL);
      sed_cell_destroy( c );
   }
}

void
test_cell_new_classed (vodi)
{
   Sed_cell c = sed_cell_new_classed (
                  NULL, 27.2,
                  S_SED_TYPE_SAND|S_SED_TYPE_SILT|S_SED_TYPE_CLAY);

   g_assert (sed_cell_is_valid (c));
   g_assert (sed_cell_mass(c)>=0);
   g_assert (!sed_cell_is_empty(c));

   sed_cell_destroy(c);
}

void
test_cell_destroy (void)
{
   {
      Sed_cell c = sed_cell_new( 1 );

      c = sed_cell_destroy( c );

      g_assert (c==NULL);
   }
}

void
test_cell_cmp (void)
{
   gboolean same;

   {
      Sed_cell a = sed_cell_new( 3 );

      same = sed_cell_is_same( a , a );

      g_assert (same);

      sed_cell_destroy( a );
   }

   {
      Sed_cell a = sed_cell_new( 3 );
      Sed_cell b = sed_cell_new( 1 );

      same = sed_cell_is_same( a , b );

      g_assert (!same);
      
      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      Sed_cell a = sed_cell_new( 3 );
      Sed_cell b = sed_cell_new( 3 );

      sed_cell_resize( a , 1. );
      sed_cell_resize( b , 2. );

      same = sed_cell_is_same( a , b );

      g_assert (!same);
      
      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

}

void
test_cell_copy (void)
{

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b;

      sed_cell_resize( a , 100 );

      b = sed_cell_copy( NULL , a );

      g_assert (a!=b);
      g_assert (sed_cell_is_same (a, b));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b = sed_cell_new( 5 );

      sed_cell_resize( a , 100 );

      sed_cell_copy( b , a );

      g_assert (sed_cell_is_same (a, b));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }
}

void
test_cell_clear (void)
{
   {
      Sed_cell c = sed_cell_new( 5 );

      g_assert (sed_cell_clear(c)==c);

      sed_cell_destroy( c );
   }

   {
      Sed_cell c = sed_cell_new( 5 );
      double f[5] = { .2 , .2 , .2 , .2 , .2 };

      sed_cell_resize ( c , 10 );
      sed_cell_set_age( c , 33 );
      sed_cell_set_fraction( c , f );

      sed_cell_clear( c );

      g_assert (sed_cell_is_empty (c));
      g_assert (eh_compare_dbl (sed_cell_age(c), 0., 1e-12));
   }
}

void
test_cell_set_and_get_functions (void)
{
   {
      gssize n;
      Sed_cell c = sed_cell_new( 5 );
      double f[5] = { .2 , .2 , .2 , .2 , .2 };

      sed_cell_resize( c , 2 );
      g_assert (sed_cell_is_size(c,2));
      g_assert (eh_compare_dbl (sed_cell_size_0(c), 2, 1e-12));

      sed_cell_set_age( c , 33 );
      g_assert (eh_compare_dbl (sed_cell_age(c), 33, 1e-12));

      sed_cell_set_facies( c , S_FACIES_PLUME );
      g_assert (sed_cell_facies(c)==S_FACIES_PLUME);

      sed_cell_set_facies( c , S_FACIES_WAVE );
      g_assert (sed_cell_facies(c)==S_FACIES_WAVE);

      sed_cell_add_facies( c , S_FACIES_RIVER );
      g_assert (sed_cell_facies(c)&S_FACIES_WAVE);
      g_assert (sed_cell_facies(c)&S_FACIES_RIVER);

      sed_cell_set_fraction( c , f );
      f[0] = 1;
      g_assert (!eh_compare_dbl (sed_cell_fraction(c,0), 1, 1e-12));
      for ( n=0 ; n<sed_cell_n_types(c) ; n++ )
         g_assert (eh_compare_dbl (sed_cell_fraction(c,n), .2, 1e-12));

      sed_cell_set_pressure( c , 1000 );
      g_assert (eh_compare_dbl (sed_cell_pressure (c), 1000., 1e-12));

      sed_cell_destroy( c );
   }
}

void
test_cell_add (void)
{

   {
      Sed_cell a = sed_cell_new( 5 );
      Sed_cell b = sed_cell_new( 5 );

      sed_cell_set_equal_fraction( a );
      sed_cell_resize( a , 1 );

      sed_cell_add( b , a );

      g_assert (sed_cell_is_same (a, b));

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

      g_assert (sed_cell_is_size(b,3));
      g_assert (sed_cell_is_size(a,1));
      g_assert (eh_compare_dbl (sed_cell_age (b), 3., 1e-12));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }
}

void
test_cell_resize (void)
{
   {
      gssize n;
      Sed_cell c = sed_cell_new( 5 );

      sed_cell_set_age     ( c , 10 );
      sed_cell_set_pressure( c , 10 );
      sed_cell_set_facies  ( c , S_FACIES_PLUME );

      sed_cell_set_equal_fraction( c );

      sed_cell_resize( c , 1 );
      g_assert (eh_compare_dbl (sed_cell_size (c), 1., 1e-12));
      g_assert (eh_compare_dbl (sed_cell_size_0 (c), 1., 1e-12));

      sed_cell_resize( c , .5 );
      g_assert (eh_compare_dbl (sed_cell_size (c), .5, 1e-12));
      g_assert (eh_compare_dbl (sed_cell_size_0 (c), .5, 1e-12));

      sed_cell_compact( c , .25 );
      g_assert (eh_compare_dbl (sed_cell_size (c), .25, 1e-12));
      g_assert (eh_compare_dbl (sed_cell_size_0 (c), .5, 1e-12));

      g_assert (eh_compare_dbl (sed_cell_age (c), 10., 1e-12));
      g_assert (eh_compare_dbl (sed_cell_pressure (c), 10., 1e-12));
      g_assert (sed_cell_facies(c)==S_FACIES_PLUME);

      for ( n=0 ; n<sed_cell_n_types(c) ; n++ )
         g_assert (eh_compare_dbl (sed_cell_fraction (c, n), .2, 1e-12));

      sed_cell_destroy( c );
   }
}

void
test_cell_resize_neg (void)
{
   Sed_cell c = sed_cell_new( 5 );

   sed_cell_resize( c , -1 );

   g_assert (sed_cell_is_clear (c));

   sed_cell_destroy( c );
}

void
test_cell_resize_zero (void)
{
   Sed_cell c = sed_cell_new( 5 );

   sed_cell_resize( c , 0 );

   g_assert (sed_cell_is_clear(c));

   sed_cell_destroy( c );
}

void
test_cell_separate_cell (void)
{
   {
      double f_0[5] = { 1. , 0 , 0 , 0 , 0 };
      double   f[5] = { .2 , .2 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 5 , f   );
      Sed_cell b = sed_cell_new_sized( 5 , 1 , f_0 );
      Sed_cell b_copy;

      b_copy = sed_cell_dup( b );

      sed_cell_separate_cell( a , b );

      g_assert (sed_cell_is_same (b_copy, b));
      g_assert (sed_cell_is_size (a, 4));
      g_assert (eh_compare_dbl (sed_cell_fraction(a,0), 0., 1e-12));
      g_assert (sed_cell_is_valid(a));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( b_copy );
   }
}

void
test_cell_separate (void)
{
   {
      double f[5] = { .2 , .2 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 1 , f );
      Sed_cell b;

      b = sed_cell_separate( a , f , .25 , NULL );

      g_assert (sed_cell_is_valid (b));
      g_assert (sed_cell_is_size(a,.75));
      g_assert (sed_cell_is_size(b,.25));

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

      g_assert (b==b_copy);
      g_assert (sed_cell_is_valid (b));
      g_assert (sed_cell_is_same (a,a_after));
      g_assert (sed_cell_is_same (b,b_after));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( a_after );
      sed_cell_destroy( b_after );
   }
}

void
test_cell_separate_thickness (void)
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

      g_assert (sed_cell_is_valid (b));
      g_assert (sed_cell_is_same (a, a_after));
      g_assert (sed_cell_is_same (b, b_after));

      sed_cell_resize( a_after , .0  );
      sed_cell_resize( b_after , .25 );

      b = sed_cell_separate_thickness( a , .75 , NULL );

      g_assert (sed_cell_is_valid (b));
      g_assert (sed_cell_is_same (a, a_after));
      g_assert (sed_cell_is_same (b, b_after));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
      sed_cell_destroy( a_after );
      sed_cell_destroy( b_after );
   }
}

void
test_cell_separate_fraction (void)
{
   {
      double f[5] = { 1. , 0  , .5 , .5  , 0  };
      double f_0[5] = { .4 , .0 , .2 , .2 , .2 };
      Sed_cell a = sed_cell_new_sized( 5 , 1 , f_0 );
      Sed_cell b;

      b = sed_cell_separate_fraction( a , f , NULL );

      g_assert (sed_cell_is_valid(a));
      g_assert (sed_cell_is_valid(b));
      g_assert (sed_cell_is_size(a,.4));
      g_assert (sed_cell_is_size(b,.6));

      sed_cell_destroy( a );
      sed_cell_destroy( b );
   }
}

void
test_cell_is_valid (void)
{
   {
      Sed_cell a = sed_cell_new( 3 );
      double f[3] = { 0. , 0. , 0. };

      g_assert (!sed_cell_is_valid(NULL));

      sed_cell_resize( a , 0 );
      sed_cell_set_fraction( a , f );
      g_assert (sed_cell_is_valid(a));

      sed_cell_resize( a , 1 );
      g_assert (!sed_cell_is_valid(a));

      f[0] = 1, f[1] = 1, f[2] = 1;
      sed_cell_set_fraction( a , f );
      g_assert (!sed_cell_is_valid(a));

      f[0] = -1;
      g_assert (!sed_cell_is_valid(a));

      sed_cell_destroy( a );
   }

   {
      Sed_cell a = sed_cell_new( 3 );

      g_assert (sed_cell_is_valid(a));

      sed_cell_destroy( a );
   }

   {
      double f[3] = { .75 , .25 , 0 };
      Sed_cell a = sed_cell_new_sized( 3 , 2 , f );

      g_assert (sed_cell_is_valid(a));

      sed_cell_destroy( a );
   }
}

void
test_cell_array_delete_empty (void)
{
   Sed_cell* a      = NULL;
   double    f_0[5] = { .4 , .0 , .2 , .2 , .2 };
   Sed_cell  b      = NULL;
   gint      i;

   for ( i=0 ; i<10 ; i++ )
   {
      b = sed_cell_new_sized( 5 , i+1 , f_0 );
      eh_strv_append( (gchar***)&a , (gchar*)b );
   }

   g_assert_cmpint (g_strv_length ((gchar**)a), == ,10);

   sed_cell_clear( a[2] );
   a = sed_cell_array_delete_empty( a );
   g_assert_cmpint (g_strv_length ((gchar**)a), ==, 9);

   sed_cell_clear( a[0] );
   a = sed_cell_array_delete_empty( a );
   g_assert_cmpint (g_strv_length ((gchar**)a), ==, 8);

   sed_cell_clear( a[2] );
   sed_cell_clear( a[3] );
   a = sed_cell_array_delete_empty( a );
   g_assert_cmpint (g_strv_length ((gchar**)a), ==, 6);

   sed_cell_clear( a[5] );
   a = sed_cell_array_delete_empty( a );
   g_assert_cmpint (g_strv_length ((gchar**)a), ==, 5);

   sed_cell_clear( a[3] );
   sed_cell_clear( a[4] );
   a = sed_cell_array_delete_empty( a );
   g_assert_cmpint (g_strv_length ((gchar**)a), ==, 3);

   sed_cell_clear( a[0] );
   sed_cell_clear( a[1] );
   sed_cell_clear( a[2] );
   a = sed_cell_array_delete_empty( a );
   g_assert (a==NULL);

   sed_cell_array_free( a );
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  if (!sed_test_setup_sediment ("sediment"))
    eh_exit (EXIT_FAILURE);

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_cell/new", &test_cell_new         );
  g_test_add_func ("/libsed/sed_cell/new_classed", &test_cell_new_classed );
  g_test_add_func ("/libsed/sed_cell/destroy", &test_cell_destroy);
  g_test_add_func ("/libsed/sed_cell/cmp", &test_cell_cmp);
  g_test_add_func ("/libsed/sed_cell/copy", &test_cell_copy);
  g_test_add_func ("/libsed/sed_cell/clear", &test_cell_clear );
  g_test_add_func ("/libsed/sed_cell/getset", &test_cell_set_and_get_functions );
  g_test_add_func ("/libsed/sed_cell/add", &test_cell_add );
  g_test_add_func ("/libsed/sed_cell/resize", &test_cell_resize );
  g_test_add_func ("/libsed/sed_cell/resize_neg", &test_cell_resize_neg );
  g_test_add_func ("/libsed/sed_cell/resize_zero", &test_cell_resize_zero );
  g_test_add_func ("/libsed/sed_cell/separate_cell", &test_cell_separate_cell );
  g_test_add_func ("/libsed/sed_cell/separate", &test_cell_separate );
  g_test_add_func ("/libsed/sed_cell/separate_thickness", &test_cell_separate_thickness );
  g_test_add_func ("/libsed/sed_cell/separate_fraction", &test_cell_separate_fraction );
  g_test_add_func ("/libsed/sed_cell/is_valid", &test_cell_is_valid );
  g_test_add_func ("/libsed/sed_cell/array_delete", &test_cell_array_delete_empty );

  g_test_run ();
}

/*
Suite *sed_cell_suite( void )
{
   Suite *s = suite_create( "Sed_cell" );
   TCase *test_case_core  = tcase_create( "Core" );
   TCase *test_case_array = tcase_create( "Cell Array" );

   suite_add_tcase( s , test_case_core  );
   suite_add_tcase( s , test_case_array );

   tcase_add_test( test_case_core , test_cell_new         );
   tcase_add_test( test_case_core , test_cell_new_classed );
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

   tcase_add_test( test_case_array , test_cell_array_delete_empty );

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

*/
