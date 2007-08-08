#include "utils.h"
#include <check.h>

#include "sed_hydro.h"

START_TEST ( test_sed_hydro_new )
{
   Sed_hydro h = sed_hydro_new( 5 );

   fail_if    ( h==NULL              , "NULL is not a valid object" );
   fail_unless( sed_hydro_size(h)==5 , "Incorrect number of grain types" );

   {
      double qb = sed_hydro_bedload(h);
      double u  = sed_hydro_velocity(h);
      double w  = sed_hydro_width(h);
      double d  = sed_hydro_depth(h);
      double qs = sed_hydro_suspended_concentration(h);
      double dt = sed_hydro_duration(h);

      fail_unless( eh_compare_dbl( qb , 0 , 1e-12 ) , "Bedload should be 0" );
      fail_unless( eh_compare_dbl( u  , 0 , 1e-12 ) , "Veloctiy should be 0" );
      fail_unless( eh_compare_dbl( w  , 0 , 1e-12 ) , "Width should be 0" );
      fail_unless( eh_compare_dbl( d  , 0 , 1e-12 ) , "Depth should be 0" );
      fail_unless( eh_compare_dbl( qs , 0 , 1e-12 ) , "Depth should be 0" );
      fail_unless( eh_compare_dbl( dt , 1 , 1e-12 ) , "Duration should be 1" );
   }

   sed_hydro_destroy( h );
}
END_TEST

START_TEST ( test_sed_hydro_copy )
{
   Sed_hydro a   = sed_hydro_new( 5 );
   Sed_hydro a_0 = sed_hydro_new( 5 );
   Sed_hydro b   = sed_hydro_new( 5 );
   Sed_hydro b_0;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   sed_hydro_set_width   ( a_0 , 500  );
   sed_hydro_set_depth   ( a_0 , 7    );
   sed_hydro_set_velocity( a_0 , 1.23 );
   sed_hydro_set_duration( a_0 , 33   );

   b_0 = sed_hydro_copy( b , a );

   fail_unless( b_0==b                   , "Destination should be returned" );
   fail_unless( sed_hydro_is_same(b,a)   , "Destination should be same a source" );
   fail_unless( sed_hydro_is_same(a,a_0) , "Source should not change" );

   sed_hydro_destroy( a   );
   sed_hydro_destroy( a_0 );
   sed_hydro_destroy( b   );
}
END_TEST

START_TEST ( test_sed_hydro_add_cell )
{
   double f[5]   = { .2 , .2 , .2 , .2 , .2 };
   Sed_hydro a   = sed_hydro_new( 4 );
   Sed_cell  c   = sed_cell_new_sized( 5 , 1. , f );
   double volume = 12.;
   double load_0, load_1, cell_load;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   load_0 = sed_hydro_total_load( a );

   sed_cell_resize( c , volume );
   sed_hydro_add_cell( a , c );

   load_1 = sed_hydro_total_load( a );

   cell_load = sed_cell_density( c )*volume;

   fail_unless( eh_compare_dbl( load_1 , load_0+cell_load , 1e-12 ) , "" );

   sed_hydro_destroy( a );
   sed_cell_destroy ( c );
}
END_TEST

START_TEST ( test_sed_hydro_subtract_cell )
{
   double f[5]   = { .2 , .2 , .2 , .2 , .2 };
   Sed_hydro a   = sed_hydro_new( 4 );
   Sed_cell  c   = sed_cell_new_sized( 5 , 1. , f );
   double volume = 12.;
   double load_0, load_1, cell_load;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   sed_cell_resize( c , volume );
   sed_hydro_add_cell( a , c );

   load_0 = sed_hydro_total_load( a );

   sed_cell_resize( c , volume/5. );
   sed_hydro_subtract_cell( a , c );

   load_1    = sed_hydro_total_load( a );
   cell_load = sed_cell_density( c )*volume/5.;

   fail_unless( eh_compare_dbl( load_1 , load_0-cell_load , 1e-12 ) , "" );

   sed_cell_resize( c , volume );
   sed_hydro_subtract_cell( a , c );

   load_1 = sed_hydro_total_load( a );

   fail_unless( eh_compare_dbl( load_1 , 0 , 1e-12 ) , "" );

   sed_hydro_destroy( a );
   sed_cell_destroy ( c );
}
END_TEST

START_TEST ( test_sed_hydro_copy_null )
{
   Sed_hydro a   = sed_hydro_new( 5 );
   Sed_hydro b;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   b = sed_hydro_copy( NULL , a );

   fail_if( b==NULL                      , "Duplicate should be valid" );
   fail_if( b==a                         , "A copy should be made" );
   fail_unless( sed_hydro_is_same(b,a)   , "Destination should be same a source" );

   sed_hydro_destroy( a   );
   sed_hydro_destroy( b   );
}
END_TEST

START_TEST ( test_sed_hydro_init )
{
   Sed_hydro* a = sed_hydro_scan( NULL , NULL );
   Sed_hydro  b = sed_hydro_new( 4 );

   sed_hydro_set_duration( b , 365. );
   sed_hydro_set_bedload ( b , 23.1 );
   sed_hydro_set_velocity( b , 1.06 );
   sed_hydro_set_width   ( b , 263 );
   sed_hydro_set_depth   ( b , 8.3 );

   {
      double qb = sed_hydro_bedload (a[0]);
      double u  = sed_hydro_velocity(a[0]);
      double w  = sed_hydro_width   (a[0]);
      double d  = sed_hydro_depth   (a[0]);
      double dt = sed_hydro_duration(a[0]);

      fail_unless( eh_compare_dbl( qb , 23.1 , 1e-12 ) , "Bedload not scanned properly"  );
      fail_unless( eh_compare_dbl( u  , 1.06 , 1e-12 ) , "Veloctiy not scanned properly" );
      fail_unless( eh_compare_dbl( w  , 263  , 1e-12 ) , "Width not scanned properly"    );
      fail_unless( eh_compare_dbl( d  , 8.3  , 1e-12 ) , "Depth not scanned properly"    );
      fail_unless( eh_compare_dbl( dt , 365  , 1e-12 ) , "Duration not scanned properly" );
   }

   fail_unless( sed_hydro_is_same(b,a[0]) , "Record not scanned properly" );

   sed_hydro_array_destroy( a );
   sed_hydro_destroy( b );
}
END_TEST

START_TEST ( test_sed_hydro_init_file )
{
   Sed_hydro* a = sed_hydro_scan( NULL , NULL );
   Sed_hydro* b = sed_hydro_scan( SED_HYDRO_TEST_INLINE_FILE , NULL );

   fail_unless( sed_hydro_is_same(b[0],a[0]) , "Record not scanned properly" );

   sed_hydro_array_destroy( a );
   sed_hydro_array_destroy( b );
}
END_TEST

START_TEST ( test_sed_hydro_file_new_inline )
{
   Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_INLINE_FILE , SED_HYDRO_INLINE , FALSE , TRUE , NULL );
   Sed_hydro a = sed_hydro_file_read_record( f );
   Sed_hydro b = sed_hydro_file_read_record( f );

   {
      double qb = sed_hydro_bedload (a);
      double u  = sed_hydro_velocity(a);
      double w  = sed_hydro_width   (a);
      double d  = sed_hydro_depth   (a);
      double dt = sed_hydro_duration(a);

      fail_unless( eh_compare_dbl( qb , 23.1 , 1e-12 ) , "Bedload not scanned properly"  );
      fail_unless( eh_compare_dbl( u  , 1.06 , 1e-12 ) , "Veloctiy not scanned properly" );
      fail_unless( eh_compare_dbl( w  , 263  , 1e-12 ) , "Width not scanned properly"    );
      fail_unless( eh_compare_dbl( d  , 8.3  , 1e-12 ) , "Depth not scanned properly"    );
      fail_unless( eh_compare_dbl( dt , 365  , 1e-12 ) , "Duration not scanned properly" );
   }

   fail_unless( sed_hydro_is_same(a,b) , "File should rewind at EOF" );

   sed_hydro_destroy( a );
   sed_hydro_destroy( b );
   sed_hydro_file_destroy( f );
}
END_TEST

START_TEST ( test_sed_hydro_file_new_binary )
{
   Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE , SED_HYDRO_HYDROTREND , FALSE , TRUE , NULL );
   Sed_hydro a = sed_hydro_file_read_record( f );

   {
      double qb = sed_hydro_bedload (a);
      double u  = sed_hydro_velocity(a);
      double w  = sed_hydro_width   (a);
      double d  = sed_hydro_depth   (a);
      double dt = sed_hydro_duration(a);

      fail_unless( eh_compare_dbl( qb , 4.467378  , 1e-6 ) , "Bedload not scanned properly"  );
      fail_unless( eh_compare_dbl( u  , .901985   , 1e-6 ) , "Veloctiy not scanned properly" );
      fail_unless( eh_compare_dbl( w  , 95.524864 , 1e-6 ) , "Width not scanned properly"    );
      fail_unless( eh_compare_dbl( d  , 4.236207  , 1e-6 ) , "Depth not scanned properly"    );
      fail_unless( eh_compare_dbl( dt , 1         , 1e-6 ) , "Duration not scanned properly" );
   }

   sed_hydro_destroy( a );
   sed_hydro_file_destroy( f );
}
END_TEST

START_TEST ( test_sed_hydro_file_new_buffer )
{
   double a_load, b_load;

   {
      Sed_hydro a;
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE ,
                                             SED_HYDRO_HYDROTREND ,
                                             TRUE , TRUE , NULL );
      double dt;

      for ( dt=0 ; dt<365 ; )
      {
         a   = sed_hydro_file_read_record( f );
         dt += sed_hydro_duration( a );
         if ( dt <= 365 )
            a_load += sed_hydro_total_load( a );
         sed_hydro_destroy( a );
      }

      sed_hydro_file_destroy( f );
   }

   {
      gssize i;
      Sed_hydro b;
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE ,
                                             SED_HYDRO_HYDROTREND ,
                                             FALSE , TRUE , NULL );

      for ( i=0,b_load=0 ; i<365 ; i++ )
      {
         b       = sed_hydro_file_read_record( f );
         b_load += sed_hydro_total_load( b );
         sed_hydro_destroy( b );
      }

      sed_hydro_file_destroy( f );
   }

   fail_unless( eh_compare_dbl(a_load,b_load,1e-12) , "Mass balance error" );
}
END_TEST

Suite *sed_hydro_suite( void )
{
   Suite *s = suite_create( "Sed_hydro" );
   TCase *test_case_core = tcase_create( "Core" );
   TCase *test_case_limits = tcase_create( "Limits" );

   suite_add_tcase( s , test_case_core );
   suite_add_tcase( s , test_case_limits );

   tcase_add_test( test_case_core , test_sed_hydro_new  );
   tcase_add_test( test_case_core , test_sed_hydro_copy );
   tcase_add_test( test_case_core , test_sed_hydro_add_cell );
   tcase_add_test( test_case_core , test_sed_hydro_subtract_cell );
   tcase_add_test( test_case_core , test_sed_hydro_init );
   tcase_add_test( test_case_core , test_sed_hydro_init_file );
   tcase_add_test( test_case_core , test_sed_hydro_file_new_inline );
   tcase_add_test( test_case_core , test_sed_hydro_file_new_binary );
   tcase_add_test( test_case_core , test_sed_hydro_file_new_buffer );

   tcase_add_test( test_case_limits , test_sed_hydro_copy_null );

   return s;
}

int test_sed_hydro( void )
{
   int n;

   {
      Suite *s = sed_hydro_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


