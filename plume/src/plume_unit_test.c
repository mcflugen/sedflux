#include "utils.h"
#include "sed_sedflux.h"
#include <check.h>

#include "plume_types.h"
#include "plumeinput.h"
#include "plume_approx.h"

gboolean plume3d( Plume_inputs *plume_const , Plume_river river       ,
                  int n_grains              , Plume_sediment *sedload ,
                  Eh_dbl_grid *deposit      , Plume_data *data );

START_TEST ( test_plume )
{
   Plume_inputs plume_const;
   Plume_river  river;
   Plume_sediment* sedload;
   Eh_dbl_grid* deposit;
   Plume_data plume_data;
   gssize n;
   double mass_in, mass_out;

   plume_const.current_velocity    = 0.;
   plume_const.ocean_concentration = 0.;
   plume_const.plume_width         = 3000.;
   plume_const.ndx                 = 1;
   plume_const.ndy                 = 3;

   river.Cs         = eh_new( double , 4 );
   river.Cs[0]      = .071;
   river.Cs[1]      = .071;
   river.Cs[2]      = .089;
   river.Cs[3]      = .124;
   river.rdirection = G_PI_2;
   river.u0         = 1.06;
   river.b0         = 263.;
   river.d0         = 8.3;
   river.Q          = river.u0*river.b0*river.d0;
   river.rma        = 0;

   sedload          = eh_new( Plume_sediment , 4 );
   sedload[0].lambda = 16.8/S_SECONDS_PER_DAY;
   sedload[1].lambda = 9./S_SECONDS_PER_DAY;
   sedload[3].lambda = 3.2/S_SECONDS_PER_DAY;
   sedload[2].lambda = 2.4/S_SECONDS_PER_DAY;
   sedload[0].rho    = 1800.;
   sedload[1].rho    = 1750.;
   sedload[2].rho    = 1650.;
   sedload[3].rho    = 1600.;

   plume_data_init( &plume_data );

   deposit = eh_new( Eh_dbl_grid , 4 );
   for ( n=0 ; n<4 ; n++ )
   {
      deposit[n] = eh_grid_new( double , 2 , 800 );

      eh_grid_set_x_lin( deposit[n] , -30000 , 30000 );
      eh_grid_set_y_lin( deposit[n] , 0      , 100 );
   }

   for ( n=0,mass_in=0 ; n<4 ; n++ )
      mass_in += river.Q*river.Cs[n]*S_SECONDS_PER_DAY;

   plume3d( &plume_const , river , 4 , sedload , deposit , &plume_data );

   for ( n=0,mass_out=0 ; n<4 ; n++ )
   {
      mass_out += eh_dbl_grid_sum( deposit[n] )
                * sedload[n].rho
                * ( eh_grid_x(deposit[n])[1] - eh_grid_x(deposit[n])[0] )
                * ( eh_grid_y(deposit[n])[1] - eh_grid_y(deposit[n])[0] );
   }

//   eh_dbl_grid_fprintf( stderr , "%g, " , deposit[3] );

   fail_unless( eh_compare_dbl( mass_in , mass_out , 1e-3 ) , "Mass-balance error in plume" );
}
END_TEST

/*
START_TEST ( test_plume_from_file )
{
   Plume_inputs plume_const;
   Plume_data plume_data;
   Eh_dbl_grid* deposit;

   plume_const.current_velocity    = 0.;
   plume_const.ocean_concentration = 0.;
   plume_const.plume_width         = 3000.;
   plume_const.ndx                 = 1;
   plume_const.ndy                 = 3;

   plume_data_init( &plume_data );

   {
      gssize n;
      deposit = eh_new( Eh_dbl_grid , 4 );
      for ( n=0 ; n<4 ; n++ )
      {
         deposit[n] = eh_grid_new( double , 2 , 800 );

         eh_grid_set_x_lin( deposit[n] , -30000 , 30000 );
         eh_grid_set_y_lin( deposit[n] , 0      , 100 );
      }
   }

   {
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE , HYDRO_HYDROTREND , TRUE );
      Sed_hydro river;
      double dt;

      for ( dt=0 ; dt<365 ; )
      {
         river = sed_hydro_file_read_record( f );
         dt += sed_hydro_duration( river );
         if ( dt <= 365 )
            plume_3d( &plume_const , river , sed_sediment_env() , deposit , &plume_data );
         sed_hydro_destroy( river );
      }

      sed_hydro_file_destroy( f );
   }

}
END_TEST
*/

START_TEST ( test_i_bar )
{
   gssize n_x = 1000;
   double x_max = 40;
   double dy = x_max/n_x;
//   double dy = .1;
   double l  = .087;
   double** i_bar;
   double* x = eh_linspace( 0 , x_max , n_x );
   gssize n_y;

   i_bar = plume_i_bar( x , n_x , l , &n_y , dy);

   {
      gssize i, j;

      for ( i=0 ; i<n_x ; i++ )
      {
         for ( j=0 ; j<n_y ; j++ )
            fprintf( stderr , "%g, " , i_bar[i][j] );
         fprintf( stderr , "\n" );
      }
   }

   eh_free( x        );
   eh_free( i_bar[0] );
   eh_free( i_bar    );
}
END_TEST

START_TEST ( test_approx_nd )
{
   double inv;

   inv = plume_centerline_inv_at( 0 , 1 );
   fail_unless( eh_compare_dbl( inv , 1 , 1e-12 ) , "Inventory is unity at origin" );

   inv = plume_centerline_inv_at( PLUME_XA , 1 );
   fail_unless( eh_compare_dbl( inv , exp(-PLUME_XA) , 1e-12 ) , "Inventory incorrect at x_a" );

   inv = plume_centerline_inv_at( G_MAXDOUBLE , 1 );
   fail_unless( eh_compare_dbl( inv , 0 , 1e-12 ) , "Inventory should be zero at large distance" );

   {
      double x;

      for ( x=0 ; x<5000 ; x+=1 )
      {
         inv = plume_centerline_inv_at( x , 1. );
         fail_unless( ~eh_isnan(inv) , "Inventory must not be NaN" );
         fail_unless( inv>=0         , "Inventory is positive everywhere" );
         fail_unless( inv<=1         , "Inventory is less than 1 everywhere" );
         fail_unless( inv<=exp(-x)   , "Inventory must be less than exponential falloff" );
      }
   }
}
END_TEST

START_TEST ( test_approx_deposit_nd )
{
   gssize i;
   double* dep;
   gssize len = 1000;
   double* x = eh_new( double , len );

   for ( i=0 ; i<len ; i++ )
      x[i] = i/10.;

   dep = plume_centerline_deposit_nd( NULL , x , len , 1. );

   fail_if( dep==NULL , "Deposit array should be created" );

   for ( i=0 ; i<len ; i++ )
   {
      fail_unless( ~eh_isnan(dep[i]) , "Deposit thickness is non-NaN" );
      fail_unless( dep[i]>=0         , "Deposit thickness is positive" );
   }

   eh_free( dep );
   eh_free( x   );
}
END_TEST

START_TEST ( test_approx_from_file )
{
   gssize len = 1000;
   double* x   = eh_new( double , len );
   double* dep = eh_new( double , len );

   {
      Sed_type this_type;
      gssize n, i;
      gssize n_grains = sed_sediment_env_n_types();
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE , SED_HYDRO_HYDROTREND , FALSE , TRUE , NULL );
      Sed_hydro river;
      double dt;
      double l;
      double* total = eh_new0( double , len );

      for ( n=1 ; n<n_grains ; n++ )
      {
         this_type = sed_sediment_type( NULL , n );
         for ( dt=0 ; dt<365 ; )
         {
            river = sed_hydro_file_read_record( f );

            l = plume_non_dim_lambda( sed_type_lambda(this_type) , river );
            for ( i=0 ; i<len ; i++ )
               x[i] = plume_non_dim_distance( i*100 , river );

            dt += sed_hydro_duration( river );
            if ( dt <= 365 )
               dep = plume_centerline_deposit_nd( dep , x , len , l );

            eh_dbl_array_add_each( total , len , dep );

            sed_hydro_destroy( river );
         }
      }

      sed_hydro_file_destroy( f );
      eh_free( total );
   }

   eh_free( x );
   eh_free( dep );
}
END_TEST

Suite *sed_plume_suite( void )
{
   Suite *s = suite_create( "Plume" );
   TCase *test_case_core   = tcase_create( "Core" );
   TCase *test_case_approx = tcase_create( "Approximation" );
   TCase *test_case_num    = tcase_create( "Numerical" );

   suite_add_tcase( s , test_case_core );
   suite_add_tcase( s , test_case_approx );
   suite_add_tcase( s , test_case_num );

   tcase_set_timeout( test_case_num , 0 );
/*
   tcase_add_test( test_case_core   , test_plume      );
//   tcase_add_test( test_case_core   , test_plume_from_file );


   tcase_add_test( test_case_approx , test_approx_nd  );
   tcase_add_test( test_case_approx , test_approx_deposit_nd  );
   tcase_add_test( test_case_approx , test_approx_from_file  );
*/
   tcase_add_test( test_case_num , test_i_bar  );

   return s;
}

int main( void )
{
   int n;
   GError*      error    = NULL;
   Sed_sediment this_sed = NULL;

   eh_init_glib();

   this_sed = sed_sediment_scan( SED_SEDIMENT_TEST_FILE , &error );

   if ( !this_sed )
      eh_error( "%s: Unable to scan sediment file: %s" , SED_SEDIMENT_TEST_FILE , error->message );

   sed_sediment_set_env( this_sed );

   {
      Suite *s = sed_plume_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   sed_sediment_unset_env();

   return n;
}


