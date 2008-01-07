#include <eh_utils.h>
#include <check.h>

gchar* test_dlmfile[] =
{
   "/* a single line comment */\n" ,
   "0, 1\n" ,
   "/* a multiple line comment\n" ,
   "   that doesn't end until here. */\n" ,
   "2, 3 // c++ style comment\n" ,
   "    \n" ,
   "4, 5, # another type of comment and an extra comma\n" ,
   "6; /* ; is also a valid delimiter */ 7\n" ,
   "// comment at the end without a newline character" ,
   NULL
};

START_TEST ( test_dlmread )
{
   char* tmp_file = tempnam( "/tmp" , "DLM_TEST_" );
   double** data;
   gint i, j;
   gint n_rows, n_cols;

   /* write a test file */
   {
      FILE* fp = eh_fopen( tmp_file , "w" );
      char** line;

      for ( line=test_dlmfile ; *line ; line++ )
         fprintf( fp , "%s" , *line );
      fclose( fp );
   }

   /* read the test file */
   {
      FILE* fp = eh_fopen( tmp_file , "r" );
      data = eh_dlm_read( tmp_file , ",;:" , &n_rows , &n_cols , NULL );
      fclose( fp );
   }

   fail_unless( n_rows==4 , "Number of rows read incorrectly" );
   fail_unless( n_cols==3 , "Number of columns read incorrectly" );

   for ( i=0 ; i<n_rows ; i++ )
      for ( j=0 ; j<2 ; j++ )
         fail_unless( eh_compare_dbl(data[i][j],2.*i+j,1e-12) ,
                      "Incorrect value read" );
   for ( i=0 ; i<n_rows ; i++ )
      fail_unless( eh_compare_dbl(data[i][2],0.,1e-12) ,
                   "Incorrect value read" );

   eh_free_2( data );
}
END_TEST

gchar* test_dlm_seq_file[] =
{
   "/* a single line comment */\n" ,
   " [ rec number: 1 ]" ,
   "0, 1\n" ,
   "/* a multiple line comment\n" ,
   "   that doesn't end until here. */\n" ,
   "    [ name: a new record \n" ,
   "      label: value] \n" ,
   "2, 3 // c++ style comment\n" ,
   "    \n" ,
   "4, 5, # another type of comment and an extra comma\n" ,
   "6; /* ; is also a valid delimiter */ 7\n" ,
   " \n" ,
   "    \n" ,
   NULL
};

START_TEST ( test_dlmread_seq )
{
   char* tmp_file = tempnam( "/tmp" , "DLM_TEST_" );
   double*** data;
   gchar** rec_data;
   gint *n_rows, *n_cols;

   /* write a test file */
   {
      FILE* fp = eh_fopen( tmp_file , "w" );
      char** line;

      for ( line=test_dlm_seq_file ; *line ; line++ )
         fprintf( fp , "%s" , *line );
      fclose( fp );
   }

   /* read the test file */
   {
      FILE* fp = eh_fopen( tmp_file , "r" );
      data = eh_dlm_read_full( tmp_file , ",;:" , &n_rows , &n_cols , &rec_data , -1 , NULL );
      fclose( fp );
   }

   fail_unless( data!=NULL && data[0]!=NULL && data[1]!=NULL && data[2]==NULL );

   fail_unless( n_rows[0]==1 , "Number of rows of first record read incorrectly" );
   fail_unless( n_cols[0]==2 , "Number of columns of first record read incorrectly" );

   fail_unless( n_rows[1]==3 , "Number of rows of second record read incorrectly" );
   fail_unless( n_cols[1]==3 , "Number of columns of second record read incorrectly" );

   fail_unless(    eh_compare_dbl(data[0][0][0],0,1e-12)
                && eh_compare_dbl(data[0][0][1],1,1e-12) ,
                "Incorrect value read" );

   fail_unless(    eh_compare_dbl(data[1][0][0],2,1e-12)
                && eh_compare_dbl(data[1][0][1],3,1e-12)
                && eh_compare_dbl(data[1][1][0],4,1e-12)
                && eh_compare_dbl(data[1][1][1],5,1e-12)
                && eh_compare_dbl(data[1][2][0],6,1e-12)
                && eh_compare_dbl(data[1][2][1],7,1e-12) ,
                "Incorrect value read" );

   fail_unless(    eh_compare_dbl(data[1][0][2],0,1e-12)
                && eh_compare_dbl(data[1][1][2],0,1e-12)
                && eh_compare_dbl(data[1][2][2],0,1e-12) ,
                "Incorrect value read" );

   eh_free_2( data );
}
END_TEST

Suite *eh_utils_suite( void )
{
   Suite *s = suite_create( "Eh_utils" );
   TCase *test_case_core = tcase_create( "Core" );

   suite_add_tcase( s , test_case_core );

   tcase_add_test( test_case_core , test_dlmread     );
   tcase_add_test( test_case_core , test_dlmread_seq );

   return s;
}

int test_utils( void )
{
   int n;

   {
      Suite *s = eh_utils_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}

