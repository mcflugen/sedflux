#include "utils.h"

int test_utils       ( void );
int test_grid        ( void );
int test_project     ( void );
int test_num         ( void );
int test_symbol_table( void );
int test_input_val   ( void );

int main( void )
{
   int n = 0;

   eh_init_glib();

   n += test_grid();
   n += test_project();
   n += test_num();
   n += test_symbol_table();
   n += test_input_val();
   n += test_utils();

   fprintf( stdout , "-----------------------------------\n\n" );
   if ( n==0 )
      fprintf( stdout , "All tests passed!\n\n" );
   else
      fprintf( stdout , "There were %d failures\n\n" , n );
   fprintf( stdout , "-----------------------------------\n" );

   return n;
}
