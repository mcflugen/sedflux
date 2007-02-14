#include "sed_sedflux.h"

int test_sed_cell( void );
int test_sed_hydro( void );
int test_sed_wave( void );
int test_sed_column( void );
int test_sed_cube( void );
int test_sed_sediment( void );

int main( void )
{
   int          n     = 0;
   Sed_sediment s     = NULL;
   GError*      error = NULL;

   eh_init_glib();

   s = sed_sediment_scan( SED_SEDIMENT_TEST_FILE , &error );

   if ( s )
      sed_sediment_set_env( s );
   else
      eh_error( "%s: Unable to read sediment file: %s" , SED_SEDIMENT_TEST_FILE , error->message  );

   eh_message( "Start test" );

   n += test_sed_cell();
   n += test_sed_hydro();
   n += test_sed_wave();
   n += test_sed_column();
   n += test_sed_cube();
   n += test_sed_sediment();
   n += test_sed_river();

   sed_sediment_unset_env();

   fprintf( stdout , "-----------------------------------\n\n" );
   if ( n==0 )
      fprintf( stdout , "All tests passed!\n\n" );
   else
      fprintf( stdout , "There were %d failures\n\n" , n );
   fprintf( stdout , "-----------------------------------\n" );

   return n;
}

