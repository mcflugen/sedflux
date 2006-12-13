#include "sed_sedflux.h"

int test_sed_cell( void );
int test_sed_hydro( void );
int test_sed_wave( void );
int test_sed_column( void );
int test_sed_cube( void );
int test_sed_sediment( void );

int main( void )
{
   int n = 0;

   eh_init_glib();

   sed_sediment_set_env( sed_sediment_scan(SED_SEDIMENT_TEST_FILE) );

   eh_message( "Start test" );

   n += test_sed_cell();
   n += test_sed_hydro();
   n += test_sed_wave();
   n += test_sed_column();
   n += test_sed_cube();
   n += test_sed_sediment();

   sed_sediment_unset_env();

   fprintf( stdout , "-----------------------------------\n\n" );
   if ( n==0 )
      fprintf( stdout , "All tests passed!\n\n" );
   else
      fprintf( stdout , "There were %d failures\n\n" , n );
   fprintf( stdout , "-----------------------------------\n" );

   return n;
}

