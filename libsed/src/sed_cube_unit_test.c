#include "utils.h"
#include <check.h>

#include "sed_cube.h"

START_TEST ( test_sed_cube_new )
{
   {
      Sed_cube c = sed_cube_new( 2 , 5 );

      fail_if    ( c==NULL                             , "NULL is not a valid cube" );
      fail_unless( sed_cube_n_x(c)==2                  , "New is of incorrect dimension" );
      fail_unless( sed_cube_n_y(c)==5                  , "New is of incorrect dimension" );
      fail_unless( eh_compare_dbl(sed_cube_mass(c),0,1e-12) , "New cube should be empty" );

      sed_cube_destroy( c );
   }
}
END_TEST

START_TEST ( test_sed_cube_destroy )
{
   Sed_cube c = sed_cube_new( 5 , 2 );

   c = sed_cube_destroy(c);

   fail_unless( c==NULL , "Destroyed cube should be NULL" );
}
END_TEST

gchar* test_seqfile[] =
{
   " # Begin the first record\n" ,
   "[ TiMe: 1 ] /* Label is case insensitive*/\n" ,
   "0, -1\n" ,
   "10, 1\n" ,
   "/* The second record.\n" ,
   "*/\n" ,
   "[ time : 3 ]\n" ,
   "0, -1\n" ,
   "5, -2\n" ,
   "9, 1// file ending without an empty line" ,
   NULL
};

START_TEST ( test_sequence_2 )
{
   char* file = tempnam( "/tmp" , "SED_CUBE_TEST_" );
   Eh_sequence* s;
   gint n_y = 10;
   double* y = eh_linspace( 0 , 9 , n_y );

   {
      FILE* fp = eh_fopen( file , "w" );
      gchar** line;

      for ( line=test_seqfile ; *line ; line++ )
         fprintf( fp , "%s" , *line );

      fclose( fp );
   }

   {
      GError* err = NULL;

      s = sed_get_floor_sequence_2( file , y , n_y , &err );

      if ( err )
         fprintf( stderr , "%s" , err->message );

   }

   fail_unless( s!=NULL   , "NULL is not a valid sequence" );
   fail_unless( s->len==2 , "Incorrect number of records read" );


}
END_TEST

Suite *sed_cube_suite( void )
{
   Suite *s = suite_create( "Sed_cube" );
   TCase *test_case_core   = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core   );

   tcase_add_test( test_case_core , test_sed_cube_new      );
   tcase_add_test( test_case_core , test_sed_cube_destroy  );
   tcase_add_test( test_case_core , test_sequence_2        );

   return s;
}

int test_sed_cube( void )
{
   int n;

   {
      Suite *s = sed_cube_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


