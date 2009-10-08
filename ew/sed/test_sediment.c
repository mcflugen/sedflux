#include <glib.h>
#include "utils/utils.h"

#include "sed_sediment.h"
#include "sed_input_files.h"

void
test_sed_sediment_new (void)
{
   {
      Sed_sediment s = sed_sediment_new( );

      g_assert (s!=NULL);
      g_assert_cmpint (sed_sediment_n_types(s), ==, 0);

      sed_sediment_destroy(s);
   }
}


void
test_sed_type_new (void)
{
   {
      Sed_type t = sed_type_new();

      g_assert (t!=NULL);
      g_assert (eh_compare_dbl (sed_type_grain_size(t), 0, 1e-12));
      g_assert (eh_compare_dbl (sed_type_density_0(t), 0, 1e-12));

      sed_type_destroy( t );
   }
}


void
test_sed_type_copy (void)
{
   {
      Sed_type t_1 = sed_type_new();
      Sed_type t_2 = sed_type_new();
      Sed_type temp;

      sed_type_set_grain_size( t_1 , 1945 );
      sed_type_set_grain_size( t_2 , 1973 );

      temp = sed_type_copy( t_1 , t_2 );

      g_assert (sed_type_is_same_size(t_1,t_2));
      g_assert (temp==t_1);

      sed_type_destroy( t_1 );
      sed_type_destroy( t_2 );
   }

   {
      Sed_type t_1 = sed_type_new();
      Sed_type t_2;

      sed_type_set_grain_size( t_1 , 1945 );
      t_2 = sed_type_copy( NULL , t_1 );

      g_assert (sed_type_is_same_size (t_1, t_2));
      g_assert (t_2!=t_1);

      sed_type_destroy( t_1 );
      sed_type_destroy( t_2 );
   }
}


void
test_sed_sediment_add (void)
{
   {
      Sed_sediment s = sed_sediment_new();
      Sed_sediment s_0;
      Sed_type     t = sed_type_new();
      Sed_type     t_0;

      sed_type_set_grain_size( t , 142 );

      s_0 = sed_sediment_add_type( s , t );

      g_assert_cmpint (sed_sediment_n_types(s), ==, 1);
      g_assert (s_0==s);

      t_0 = sed_sediment_type(s,0);

      g_assert (t_0!=t);
      g_assert (sed_type_is_same(t,t_0));

      sed_sediment_destroy( s );
      sed_type_destroy( t );
   }
}

void
test_sed_sediment_scan (void)
{
   {
      GError* error = NULL;
      gchar* buffer = g_strjoinv ("\n",_default_sediment_file);
      Sed_sediment s = sed_sediment_scan_text (buffer, error);

      g_assert (s!=NULL);
      g_assert (error==NULL);
      g_assert_cmpint (sed_sediment_n_types(s), ==, 5);

      sed_sediment_destroy( s );
   }
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_sediment/new", &test_sed_sediment_new);
  g_test_add_func ("/libsed/sed_sediment/add", &test_sed_sediment_add);
  g_test_add_func ("/libsed/sed_sediment/scan", &test_sed_sediment_scan);
  g_test_add_func ("/libsed/sed_sediment/new_type", &test_sed_type_new);

  g_test_run ();
}

