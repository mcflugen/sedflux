#include <glib.h>
#include "utils/utils.h"

#include "sed_river.h"
#include "sed_hydro.h"

void
test_sed_river_new (void)
{
   {
      Sed_riv r = sed_river_new( NULL );

      g_assert (r!=NULL);
      g_assert (eh_compare_dbl (sed_river_width (r), 0, 1e-12));
      g_assert (eh_compare_dbl (sed_river_depth (r), 0, 1e-12));
      g_assert (eh_compare_dbl (sed_river_velocity (r),0,1e-12));

      sed_river_destroy( r );
   }

   {
      Sed_riv r = sed_river_new( "Ebro" );

      g_assert (r!=NULL);
      g_assert (eh_compare_dbl (sed_river_width (r), 0, 1e-12));
      g_assert (eh_compare_dbl (sed_river_depth (r), 0, 1e-12));
      g_assert (eh_compare_dbl (sed_river_velocity (r),0,1e-12));
      g_assert (sed_river_name_is (r,"Ebro"));

      sed_river_destroy( r );
   }
}


void
test_sed_river_copy (void)
{
   {
      Sed_riv dest = sed_river_new( "South Platte" );
      Sed_riv src  = sed_river_new( "Mississippi" );
      Sed_riv temp = dest;

      dest = sed_river_copy( dest , src );

      g_assert (sed_river_name_is (dest, "Mississippi"));
      g_assert (dest!=src);
      g_assert (dest==temp);

      sed_river_destroy (src);
      sed_river_destroy (dest);
   }
}


void
test_sed_river_dup (void)
{
   {
      Sed_riv dest;
      Sed_riv src  = sed_river_new( "Mississippi" );

      dest = sed_river_copy( NULL , src );

      g_assert (sed_river_name_is (dest, "Mississippi"));
      g_assert (dest!=src);

      sed_river_destroy( src  );
      sed_river_destroy( dest );
   }
}


void
test_sed_river_leaves (void)
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_mouth;

      sed_river_split( r );
      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );

      river_mouth = sed_river_leaves( r );

      g_assert_cmpint (g_strv_length ((gchar**)river_mouth), ==, 4);
   }
}


void
test_sed_river_branches (void)
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      sed_river_split( r );
      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );

      river_branch = sed_river_branches( r );

      g_assert_cmpint (g_strv_length ((gchar**)river_branch), ==, 7);
   }
}


void
test_sed_river_n_branches (void)
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      g_assert_cmpint (sed_river_n_branches (NULL), ==, 0);

      g_assert_cmpint (sed_river_n_branches (r), ==, 1);

      sed_river_split( r );
      g_assert_cmpint (sed_river_n_branches (r), ==, 3);

      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );
      g_assert_cmpint (sed_river_n_branches(r), ==, 7);
   }
}


void
test_sed_river_longest (void)
{
   {
      Sed_riv r = sed_river_new( NULL );
      Sed_riv* river_branch;

      g_assert (sed_river_longest_branch (NULL)==NULL);

      g_assert (sed_river_longest_branch(r)==r);

      sed_river_split( r );
      g_assert (sed_river_longest_branch(r)==sed_river_left(r));

      sed_river_split( sed_river_left(r) );
      sed_river_split( sed_river_right(sed_river_left(r)) );
      g_assert (sed_river_longest_branch(r)==sed_river_right(r));
   }
}


void
test_sed_river_split (void)
{
   {
      Sed_riv r    = sed_river_new( NULL );
      Sed_hydro* h = sed_hydro_scan( NULL , NULL );

      sed_river_set_hydro(r,h[0]);
      sed_river_split( r );

      g_assert (sed_river_has_children (r));

      {
         double q   = sed_river_water_flux( r );
         double q_l = sed_river_water_flux( sed_river_left ( r ) );
         double q_r = sed_river_water_flux( sed_river_right( r ) );

         g_assert (eh_compare_dbl (q, q_l+q_r, 1e-12));
      }

      {
         double qs   = sed_river_sediment_load( r );
         double qs_l = sed_river_sediment_load( sed_river_left ( r ) );
         double qs_r = sed_river_sediment_load( sed_river_right( r ) );

         g_assert (eh_compare_dbl (qs, qs_l+qs_r, 1e-12));
      }
   }
}


void
test_sed_river_set_angle (void)
{
   Sed_riv r = sed_river_new( NULL );

   sed_river_set_angle_limit( r , 0. , .5*G_PI );
   sed_river_set_angle      ( r , .25*G_PI );

   g_assert (eh_compare_dbl (sed_river_angle(r), .25*G_PI, 1e-12));

   sed_river_set_angle( r , .95*G_PI );

   g_assert (eh_compare_dbl (sed_river_angle(r), .5*G_PI, 1e-12));

   sed_river_set_angle( r , -.75*G_PI );
   g_assert (eh_compare_dbl (sed_river_angle(r), 0., 1e-12));

   {
     const double min_angle = .75*G_PI;
     const double max_angle = 1.25*G_PI;

     sed_river_set_angle_limit (r, min_angle, max_angle);
     g_assert (eh_compare_dbl (sed_river_min_angle(r), min_angle, 1e-12));
     g_assert (eh_compare_dbl (sed_river_max_angle(r), max_angle, 1e-12));

     sed_river_set_angle (r, G_PI);
     g_assert (eh_compare_dbl (sed_river_angle(r), G_PI, 1e-12));
   }

   {
     const double min_angle = .75*G_PI;
     const double max_angle = 2.5*G_PI;

     sed_river_set_angle_limit (r, min_angle, max_angle);
     g_assert (eh_compare_dbl (sed_river_min_angle(r), min_angle, 1e-12));
     g_assert (eh_compare_dbl (sed_river_max_angle(r), max_angle, 1e-12));

     sed_river_set_angle (r, G_PI);
     g_assert (eh_compare_dbl (sed_river_angle(r), G_PI, 1e-12));

     sed_river_set_angle (r, 0);
     g_assert (eh_compare_dbl (sed_river_angle(r), 2.*G_PI, 1e-12));

     sed_river_set_angle (r, .25*G_PI);
     g_assert (eh_compare_dbl (sed_river_angle(r), 2.25*G_PI, 1e-12));

     sed_river_set_angle (r, -.5*G_PI);
     g_assert (eh_compare_dbl (sed_river_angle(r), 1.5*G_PI, 1e-12));
   }
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_river/new", &test_sed_river_new);
  g_test_add_func ("/libsed/sed_river/copy", &test_sed_river_copy);
  g_test_add_func ("/libsed/sed_river/dup", &test_sed_river_dup);
  g_test_add_func ("/libsed/sed_river/leaves", &test_sed_river_leaves);
  g_test_add_func ("/libsed/sed_river/branches", &test_sed_river_branches);
  g_test_add_func ("/libsed/sed_river/n_branches", &test_sed_river_n_branches);
  g_test_add_func ("/libsed/sed_river/set/angle", &test_sed_river_set_angle);

  g_test_run ();
}

