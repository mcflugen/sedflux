#include "utils/utils.h"
#include <glib.h>

#include "sed_wave.h"

void
test_sed_wave_new (void)
{
   {
      Sed_wave w = sed_wave_new( 0 , 2 , 3 );

      g_assert (w!=NULL);
      g_assert (eh_compare_dbl (sed_wave_height(w), 0., 1e-12));
      g_assert (eh_compare_dbl (sed_wave_number(w), 2., 1e-12));
      g_assert (eh_compare_dbl (sed_wave_frequency(w), 3., 1e-12));

      sed_wave_destroy( w );
   }

   { /* Wave can not have negative frequency */
      Sed_wave w = sed_wave_new( 0 , 0 , -1 );
      g_assert (w==NULL);
   }

   { /* Wave can not have negative number */
      Sed_wave w = sed_wave_new( 0 , -1 , 0 );
      g_assert (w==NULL);
   }

   { /* Wave can not have negative height */
      Sed_wave w = sed_wave_new( -1 , 0 , 0 );
      g_assert (w==NULL);
   }

}

void
test_sed_wave_copy (void)
{
   {
      Sed_wave dest = sed_wave_new( 0 , 0 , 0 );
      Sed_wave src  = sed_wave_new( 1 , 2 , 3 );
      Sed_wave temp = dest;

      dest = sed_wave_copy( dest , src );

      g_assert (sed_wave_is_same(dest,src));
      g_assert (dest!=src);
      g_assert (dest==temp);

      sed_wave_destroy( src  );
      sed_wave_destroy( dest );
   }
}

int
main (int argc, char* argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_wave/new", &test_sed_wave_new);
  g_test_add_func ("/libsed/sed_wave/copy", &test_sed_wave_copy);

  g_test_run ();
}

