#include "utils.h"
#include <check.h>

#include "sed_river.h"
#include "sed_hydro.h"

START_TEST ( test_sed_river_new )
{
   {
      Sed_riv r = sed_river_new( NULL );

      fail_if    ( r==NULL                            , "NULL returned instead of a new river"  );
      fail_unless( eh_compare_dbl(sed_river_width   (r),0,1e-12) , "Non-zero width" );
      fail_unless( eh_compare_dbl(sed_river_depth   (r),0,1e-12) , "Non-zero depth" );
      fail_unless( eh_compare_dbl(sed_river_velocity(r),0,1e-12) , "Non-zero velocity" );

      sed_river_destroy( r );
   }

   {
      Sed_riv r = sed_river_new( "Ebro" );

      fail_if    ( r==NULL                            , "NULL returned instead of a new river"  );
      fail_unless( eh_compare_dbl(sed_river_width   (r),0,1e-12) , "Non-zero width" );
      fail_unless( eh_compare_dbl(sed_river_depth   (r),0,1e-12) , "Non-zero depth" );
      fail_unless( eh_compare_dbl(sed_river_velocity(r),0,1e-12) , "Non-zero velocity" );
      fail_unless( sed_river_name_is(r,"Ebro")                   , "Incorrect name" );

      sed_river_destroy( r );
   }
}
END_TEST

START_TEST ( test_sed_river_copy )
{
   {
      Sed_riv dest = sed_river_new( "South Platte" );
      Sed_riv src  = sed_river_new( "Mississippi" );
      Sed_riv temp = dest;

      dest = sed_river_copy( dest , src );

      fail_unless( sed_river_name_is( dest , "Mississippi" ) , "River not copied correctly" );
      fail_if    ( dest == src                               , "River not copied" );
      fail_unless( dest == temp                              , "River not copied to correct location" );

      sed_river_destroy( src  );
      sed_river_destroy( dest );
   }
}
END_TEST

START_TEST ( test_sed_river_dup )
{
   {
      Sed_riv dest;
      Sed_riv src  = sed_river_new( "Mississippi" );

      dest = sed_river_copy( NULL , src );

      fail_unless( sed_river_name_is( dest , "Mississippi" ) , "River not duplicated correctly" );
      fail_if    ( dest == src                               , "River not duplicated"           );

      sed_river_destroy( src  );
      sed_river_destroy( dest );
   }
}
END_TEST

START_TEST ( test_sed_river_leaves )
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_mouth;

      sed_river_split( r );
      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );

      river_mouth = sed_river_leaves( r );

      fail_unless( g_strv_length( river_mouth )==4 , "Incorrect number of river mouths" );
   }
}
END_TEST

START_TEST ( test_sed_river_branches )
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      sed_river_split( r );
      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );

      river_branch = sed_river_branches( r );

      fail_unless( g_strv_length( river_branch )==7 , "Incorrect number of river branches" );
   }
}
END_TEST

START_TEST ( test_sed_river_n_branches )
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      fail_unless( sed_river_n_branches(NULL)==0 , "Size of NULL river is 0" );

      fail_unless( sed_river_n_branches(r)==1 , "River size is incorrect" );

      sed_river_split( r );
      fail_unless( sed_river_n_branches(r)==3 , "River size is incorrect" );

      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );
      fail_unless( sed_river_n_branches(r)==7 , "River size is incorrect" );
   }
}
END_TEST

START_TEST ( test_sed_river_longest )
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      fail_unless( sed_river_longest_branch(NULL)==NULL , "Longest branch of NULL is NULL" );

      fail_unless( sed_river_longest_branch(r)==r , "Return the trunk if it doesn't branch" );

      sed_river_split( r );
      fail_unless( sed_river_longest_branch(r)==sed_river_left(r) , "Left branch in the case of a tie" );

      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );
      fail_unless( sed_river_longest_branch(r)==sed_river_right(r) , "Error finding longest branch" );
   }
}
END_TEST

START_TEST ( test_sed_river_split )
{
   {
      Sed_riv r    = sed_river_new( NULL );
      Sed_hydro* h = sed_hydro_scan( NULL , NULL );

      sed_river_set_hydro(r,h[0]);
      sed_river_split( r );

      fail_unless( sed_river_has_children( r ) , "Children not created in split" );

      {
         double q   = sed_river_water_flux( r );
         double q_l = sed_river_water_flux( sed_river_left ( r ) );
         double q_r = sed_river_water_flux( sed_river_right( r ) );

         fail_unless( eh_compare_dbl(q,q_l+q_r,1e-12) , "Water balance error" );
      }

      {
         double qs   = sed_river_sediment_load( r );
         double qs_l = sed_river_sediment_load( sed_river_left ( r ) );
         double qs_r = sed_river_sediment_load( sed_river_right( r ) );

         fail_unless( eh_compare_dbl(qs,qs_l+qs_r,1e-12) , "Sediment balance error" );
      }
   }
}
END_TEST

START_TEST ( test_sed_river_set_angle )
{
   Sed_riv r = sed_river_new( NULL );

   sed_river_set_angle_limit( r , 0. , .5*G_PI );
   sed_river_set_angle      ( r , .25*G_PI );

   fail_unless( eh_compare_dbl( sed_river_angle(r) , .25*G_PI , 1e-12 ) , "Angle incorrectly set"         );

   sed_river_set_angle( r , .95*G_PI );

   fail_unless( eh_compare_dbl( sed_river_angle(r) , .5*G_PI , 1e-12 ) , "Angle out of range" );

   sed_river_set_angle( r , -.75*G_PI );
   fail_unless( eh_compare_dbl( sed_river_angle(r) , 0.      , 1e-12 ) , "Angle out of range" );
}
END_TEST

Suite *sed_river_suite( void )
{
   Suite *s = suite_create( "Sed_riv" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_sed_river_new   );
   tcase_add_test( test_case_core , test_sed_river_copy  );
   tcase_add_test( test_case_core , test_sed_river_dup   );
   tcase_add_test( test_case_core , test_sed_river_split );
   tcase_add_test( test_case_core , test_sed_river_set_angle );
   tcase_add_test( test_case_core , test_sed_river_leaves );
   tcase_add_test( test_case_core , test_sed_river_branches );
   tcase_add_test( test_case_core , test_sed_river_n_branches );
   tcase_add_test( test_case_core , test_sed_river_longest );

   return s;
}

int test_sed_river( void )
{
   int n;

   {
      Suite *s = sed_river_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


