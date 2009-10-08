#include <eh_utils.h>
#include <glib.h>

void
test_nan (void)
{
   double x;

   x = eh_nan();
   g_assert (isnan(x));
   g_assert (eh_isnan(x));

//   x = 1945/0.;
//   g_assert (eh_isnan(x) , "eh_isnan does not detect divide-by-zero NaN" );

   x = sqrt(-1);
   g_assert (eh_isnan(x));

}


void
test_interpolate (void)
{
   double  x[5] = {1,2,3,4,5},  y[5]={.1,.2,.3,.4,.5};
   double xi, yi;

   xi = -1;
   interpolate( x,y,5,&xi,&yi,1 );
   g_assert (eh_isnan(yi));

   xi = 6;
   interpolate( x,y,5,&xi,&yi,1 );
   g_assert (eh_isnan(yi));

   xi = 3;
   interpolate( x,y,5,&xi,&yi,1 );
   g_assert (eh_compare_dbl (yi, .3, 1e-12));

   xi = 1;
   interpolate( x,y,1,&xi,&yi,1 );
   g_assert (eh_compare_dbl (yi, .1, 1e-12));

   xi = .5;
   interpolate( x,y,1,&xi,&yi,1 );
   g_assert (eh_isnan(yi));

   xi = 1.5;
   interpolate( x,y,1,&xi,&yi,1 );
   g_assert (eh_isnan(yi));
}


void
test_poly_interpolate (void)
{
   {
      double* x = eh_new( double , 2 );
      double* y = eh_new( double , 2 );
      double y_int, err;

      x[0] = 0;
      x[1] = 1;
      y[0] = 3;
      y[1] = 5;

      y_int = poly_interpolate( x , y , 2 , .75 , &err );

      g_assert (eh_compare_dbl (y_int, 4.5, 1e-12));
//      g_assert (eh_compare_dbl (err)<1e-6       , "Error should be zero"        );

      eh_free( x );
      eh_free( y );
   }

   {
      double* x = eh_new( double , 3 );
      double* y = eh_new( double , 3 );
      double y_int, err;

      x[0] = -1;
      x[1] = 0;
      x[2] = 1;
      y[0] = pow(x[0],2);
      y[1] = pow(x[1],2);
      y[2] = pow(x[2],2);

      y_int = poly_interpolate( x , y , 3 , .75 , &err );

      g_assert (eh_compare_dbl (y_int, pow(.75,2), 1e-12));
//      g_assert (eh_compare_dbl (err)<1e-6              , "Error should be zero"           );

      eh_free( x );
      eh_free( y );
   }
   
}


double linear_f( double x , gpointer data )
{
   return 2*x + 1;
}

double exp_f( double x , gpointer data )
{
   return exp(x);
}

typedef struct
{
   double c;
   double l;
   double y;
}
User_data;

#define M1 (1.2)
#define M2 (0.6)
#define P1 (0.8)
#define F2 (0.9)
#define XA (5.176)

#if HAVE_IEEEFP_H
# include <ieeefp.h>
#else
int finite( double dsrc );
#endif

double inventory( double x , double s , double l )
{
   double i;
   double g = exp( pow(M2*s,2.) )*F2;

   if ( !finite( g ) )
      i = 0;
   else
      i = exp( -pow(M1*s,2.) )
        * pow(XA/x,P1)
        * exp( -2*l/(3.*sqrt(XA))*( pow(x,1.5) - pow(XA,1.5) )*g );
   return i;
}

double inventory_f( double x , gpointer data )
{
   return inventory( ((User_data*)data)->y , ((User_data*)data)->c*x , ((User_data*)data)->l );
}

void
test_trapazoid (void)
{
   gssize i;
   double sum;

   sum = trapzd( linear_f , 0 , 1 , 1 , NULL );
   g_assert (eh_compare_dbl (sum, 2, 1e-12));

   for ( i=1 ; i<=5 ; i++ )
      sum = trapzd( linear_f , 0 , 1 , i , NULL );
   g_assert (eh_compare_dbl (sum, 2, 1e-12));

   for ( i=1 ; i<=9 ; i++ )
      sum = trapzd( exp_f , 0 , 1 , i , NULL );
   g_assert (eh_compare_dbl (sum, exp(1)-exp(0), 1e-4));

   sum = qtrap( exp_f , 0 , 1 , NULL );
   g_assert (eh_compare_dbl (sum, exp(1)-exp(0), 1e-4));

}


void
test_integrate (void)
{
   double sum;

   sum = eh_integrate( linear_f , 0 , 1 );
   g_assert (eh_compare_dbl (sum, 2, 1e-4));

   sum = eh_integrate( exp_f , 0 , 1 );
   g_assert (eh_compare_dbl (sum, exp(1)-exp(0), 1e-4));

   {
      User_data data;
      gssize i;
      double total=0;
      double dx = XA*.1;

      for ( i=0 ; i<40 ; i++ )
      {
         data.l = ( 50 / 86400. )*1000. / 2.;
         data.y = XA + i*dx;
         data.c = 1./( sqrt(2)*.109)*data.y;
         sum = eh_integrate_with_data( inventory_f , 0 , 20 , &data );
         total += 2.*sum*dx;
      }
   }
}


void
test_compare_dbl (void)
{
   g_assert ( eh_compare_dbl(1,1,0));
   g_assert ( eh_compare_dbl(1,1-1e-10,1e-6));
   g_assert (!eh_compare_dbl(1,1-1e-10,1e-12));
   g_assert ( eh_compare_dbl(1,1-.99e-10,1e-10));
}


void
test_round (void)
{
   double x;

   x = eh_round( 23.1 , 1 );
   g_assert (eh_compare_dbl( x , 23 , 1e-12 ));

   x = eh_round( 23.9 , 1 );
   g_assert (eh_compare_dbl( x , 24 , 1e-12 ));

   x = eh_round( 23.5 , 1 );
   g_assert (eh_compare_dbl( x , 24 , 1e-12 ));

   x = eh_round( 23. , 1 );
   g_assert (eh_compare_dbl( x , 23 , 1e-12 ));

   x = eh_round( -1945.1 , 1 );
   g_assert (eh_compare_dbl( x , -1945 , 1e-12 ));

   x = eh_round( -1945.9 , 1 );
   g_assert (eh_compare_dbl( x , -1946 , 1e-12 ));

   x = eh_round( 0 , 1 );
   g_assert (eh_compare_dbl( x , 0 , 1e-12 ));

   x = eh_round( 1973 , 10 );
   g_assert (eh_compare_dbl( x , 1970 , 1e-12 ));

   x = eh_round( 1973 , 5 );
   g_assert (eh_compare_dbl( x , 1975 , 1e-12 ));

   x = eh_round( G_PI , .01 );
   g_assert (eh_compare_dbl( x , 3.14 , 1e-12 ));
}


void
test_reduce_angle (void)
{
   double a;

   a = eh_reduce_angle(G_PI + G_PI*4 );
   g_assert (eh_compare_dbl( a , -G_PI , 1e-12 ));

   a = eh_reduce_angle(G_PI-1e-14 + G_PI*4 );
   g_assert (eh_compare_dbl( a , G_PI , 1e-12 ));

   a = eh_reduce_angle(-G_PI - G_PI*4 );
   g_assert (eh_compare_dbl( a , -G_PI , 1e-12 ));

   a = eh_reduce_angle(G_PI/4 + G_PI*22 );
   g_assert (eh_compare_dbl( a , G_PI/4 , 1e-12 ));

   a = eh_reduce_angle( 0 );
   g_assert (eh_compare_dbl( a , 0 , 1e-12 ));

}


void
test_is_even (void)
{
   g_assert ( eh_is_even(2));
   g_assert (!eh_is_even(1));
   g_assert ( eh_is_even(0));
   g_assert (!eh_is_even(G_MAXINT32));
   g_assert (!eh_is_even(-17));
   g_assert ( eh_is_even(-6));
}


void
test_diff (void)
{
   gssize i;
   gssize len = 100;
   double *f = eh_new( double , len );
   double *fx, *fxx, *fxxx, *fxxxx;
   double *x = eh_new( double , len );

   for ( i=0 ; i<len ; i++ )
   {
      x[i] = i;
      f[i] = .1*pow( x[i] , 4 );
   }

   fx    = eh_dbl_array_diff( NULL , f , len , 1 );
   fxx   = eh_dbl_array_diff( NULL , f , len , 2 );
   fxxx  = eh_dbl_array_diff( NULL , f , len , 3 );
   fxxxx = eh_dbl_array_diff( NULL , f , len , 4 );

   g_assert (fx!=NULL);
   g_assert (fxx!=NULL);
   g_assert (fxxx!=NULL);
   g_assert (fxxxx!=NULL);

   for ( i=0 ; i<len-1 ; i++ )
      g_assert (eh_compare_dbl (fx[i], f[i+1]-f[i], 1e-12 ));

   for ( i=1 ; i<len-1 ; i++ )
      g_assert (eh_compare_dbl (fxx[i], f[i+1]-2*f[i]+f[i-1], 1e-12));

   for ( i=1 ; i<len-2 ; i++ )
      g_assert (eh_compare_dbl (fxxx[i], f[i+2]+3*f[i]-3*f[i+1]-f[i-1], 1e-12));

   for ( i=2 ; i<len-2 ; i++ )
      g_assert (eh_compare_dbl (fxxxx[i], f[i+2]+6*f[i]+f[i-2]-4*f[i+1]-4*f[i-1], 1e-12));

   g_assert (eh_compare_dbl(fx[len-1],fx[len-1],1e-12));

   g_assert (eh_compare_dbl(fxx[0],fxx[1],1e-12));
   g_assert (eh_compare_dbl(fxx[len-1],fxx[len-2],1e-12));

   eh_free( fx );
   eh_free( fxx );
   eh_free( fxxx );
   eh_free( fxxxx );
   eh_free( f );
   eh_free( x );
}


void
test_binomial_coef (void)
{
   double c;

   c = eh_binomial_coef( 1 , 0 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 1 , 1 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 2 , 0 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 2 , 1 );
   g_assert (eh_compare_dbl( c , 2 , 1e-12 ));

   c = eh_binomial_coef( 2 , 2 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 3 , 0 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 3 , 1 );
   g_assert (eh_compare_dbl( c , 3 , 1e-12 ));

   c = eh_binomial_coef( 3 , 2 );
   g_assert (eh_compare_dbl( c , 3 , 1e-12 ));

   c = eh_binomial_coef( 3 , 3 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 4 , 0 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 4 , 1 );
   g_assert (eh_compare_dbl( c , 4 , 1e-12 ));

   c = eh_binomial_coef( 4 , 2 );
   g_assert (eh_compare_dbl( c , 6 , 1e-12 ));

   c = eh_binomial_coef( 4 , 3 );
   g_assert (eh_compare_dbl( c , 4 , 1e-12 ));

   c = eh_binomial_coef( 4 , 4 );
   g_assert (eh_compare_dbl( c , 1 , 1e-12 ));

   c = eh_binomial_coef( 52 , 5 );
   g_assert (eh_compare_dbl( c , 2598960 , 1e-12 ));

}


void
test_factorial (void)
{
   double x;
   double prod;

   x = eh_factorial( 0 );
   g_assert (eh_compare_dbl(x,1,1e-12));

   // Test the first 32 factorials
   {
      gssize i;
      for ( i=1,prod=1 ; i<32 ; i++ )
      {
         prod *= i;
         x = eh_factorial( i );
         g_assert (eh_compare_dbl(x,prod,1e-12));
      }
   }

   x = eh_factorial( -1 );
   g_assert (eh_isnan(x));
}


void
test_gamma_p (void)
{
   double x;

   x = eh_gamma_p( g_random_double() , 0 );
   g_assert (eh_compare_dbl( x , 0 , 1e-12 ));

   x = eh_gamma_p( g_random_double() , G_MAXDOUBLE );
   g_assert (eh_compare_dbl( x , 1 , 1e-12 ));

   x = eh_gamma_p( 2. , 1. );
   g_assert (eh_compare_dbl( x , 0.26424111765712 , 1e-6 ));
   
}


void
test_gamma_q (void)
{
   double x;

   x = eh_gamma_q( g_random_double() , 0 );
   g_assert (eh_compare_dbl( x , 1 , 1e-12 ));

   x = eh_gamma_q( g_random_double() , G_MAXDOUBLE );
   g_assert (eh_compare_dbl( x , 0 , 1e-12 ));

   x = eh_gamma_q( 2. , 1. );
   g_assert (eh_compare_dbl( x , 1.-0.26424111765712 , 1e-6 ));
   
}


void
test_gamma_series (void)
{
   double a = g_random_double();
   double p;
   double gam_log;

   eh_gamma_series( &p , a , 0 , &gam_log );
   g_assert (eh_compare_dbl( p , 0 , 1e-12 ));
   g_assert (eh_compare_dbl( gam_log , eh_gamma_log(a) , 1e-12 ));

   eh_gamma_series( &p , 2. , 1. , &gam_log );
   g_assert (eh_compare_dbl( p , 0.26424111765712 , 1e-6 ));
   
   eh_gamma_series( &p , .5 , .75 , &gam_log );
   g_assert (eh_compare_dbl( p       , 0.77932863808015 , 1e-6 ));
   g_assert (eh_compare_dbl( gam_log , 0.57236494292470 , 1e-6 ));

   eh_gamma_series( &p , 1 , 2 , &gam_log );
   g_assert (eh_compare_dbl( p       , 0.86466471676339 , 1e-6 ));
   g_assert (eh_compare_dbl( gam_log , 0 , 1e-6 ));
}


void
test_gamma_cf (void)
{
   double a = g_random_double();
   double q;
   double gam_log;

   eh_gamma_cf( &q , a , G_MAXDOUBLE , &gam_log );
   g_assert (eh_compare_dbl( q , 0 , 1e-12 ));
   g_assert (eh_compare_dbl( gam_log , eh_gamma_log(a) , 1e-12 ));

   eh_gamma_cf( &q , .25 , 2. , &gam_log  );
   g_assert (eh_compare_dbl( q , 1.-0.98271398814048 , 1e-6 ));
   g_assert (eh_compare_dbl( gam_log , 1.28802252469808 , 1e-6 ));
   
   eh_gamma_cf( &q , 1 , 2 , &gam_log );
   g_assert (eh_compare_dbl( q       , 1-0.86466471676339 , 1e-6 ));
   g_assert (eh_compare_dbl( gam_log , 0 , 1e-6 ));
}


void
test_gamma_log (void)
{
   double x;

   x = eh_gamma_log( 1 );
   g_assert (eh_compare_dbl( x , 0 , 1e-12 ));

   x = eh_gamma_log( .5 );
   g_assert (eh_compare_dbl( x , 0.57236494292470 , 1e-6 ));
   
   x = eh_gamma_log( .1 );
   g_assert (eh_compare_dbl( x , 2.25271265173421 , 1e-6 ));
   
}


void
test_linear_fit (void)
{
   {
      double x[2] = { 0 , 1 };
      double y[2] = { 1 , 4 };
      double* p = eh_linear_fit( x , y , 2 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , 1 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , 3 , 1e-12 ));

      eh_free( p );
   }

   {
      double x[1] = { 0 };
      double y[1] = { 1 };
      double* p = eh_linear_fit( x , y , 1 );

      g_assert (p==NULL);
   }

   {
      double y[3];
      double* p = eh_linear_fit( NULL , y , 3 );

      g_assert (p==NULL);
   }

   {
      double x[3];
      double* p = eh_linear_fit( x , NULL , 3 );

      g_assert (p==NULL);
   }

   {
      double x[3] = { 0 , .5 , 1 };
      double y[3] = { 1 , 2.5 , 4 };
      double* p = eh_linear_fit( x , y , 3 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , 1 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , 3 , 1e-12 ));

      eh_free( p );
   }

   {
      double x[3] = { 1 , 2 , 3 };
      double y[3] = { 3 , 7 , 8 };
      double* p = eh_linear_fit( x , y , 3 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , 1 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , 2.5 , 1e-12 ));

      eh_free( p );
   }

   {
      gssize i;
      double* p;
      double* x = eh_new( double , 100 );
      double* y = eh_new( double , 100 );

      for ( i=0 ; i<100 ; i++ )
      {
         x[i] = i/100.;
         y[i] = exp(-x[i]);
      }

      p = eh_linear_fit( x , y , 100 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] ,  0.94463333233568 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , -0.62494323848573 , 1e-12 ));

      eh_free( p );
      eh_free( y );
      eh_free( x );
   }
}


void
test_poly_fit (void)
{
   {
      double x[2] = { 0 , 1 };
      double y[2] = { 1 , 4 };
      double* p     = eh_poly_fit  ( x , y , 2 , 1 );
      double* p_lin = eh_linear_fit( x , y , 2 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , p_lin[0] , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , p_lin[1] , 1e-12 ));

      eh_free( p     );
      eh_free( p_lin );
   }

   {
      gssize i;
      double* p;
      double* p_lin;
      double* x = eh_new( double , 100 );
      double* y = eh_new( double , 100 );

      for ( i=0 ; i<100 ; i++ )
      {
         x[i] = i/100.;
         y[i] = exp(-x[i]);
      }

      p     = eh_poly_fit  ( x , y , 100 , 1 );
      p_lin = eh_linear_fit( x , y , 100 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , p_lin[0] , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , p_lin[1] , 1e-12 ));

      eh_free( p_lin );
      eh_free( p );
      eh_free( y );
      eh_free( x );
   }

   {
      double x[3] = {0,1,2};
      double y[3] = {2,1,-8};
      double* p = eh_poly_fit( x , y , 3 , 2 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] , 2 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , 3 , 1e-12 ));
      g_assert (eh_compare_dbl( p[2] , -4 , 1e-12 ));

      eh_free( p );
   }

   {
      gssize i;
      double* p;
      double* x = eh_new( double , 100 );
      double* y = eh_new( double , 100 );

      for ( i=0 ; i<100 ; i++ )
      {
         x[i] = i/100.;
         y[i] = exp(-x[i]);
      }

      p = eh_poly_fit  ( x , y , 100 , 2 );

      g_assert (p!=NULL);
      g_assert (eh_compare_dbl( p[0] ,  0.99480295546506 , 1e-12 ));
      g_assert (eh_compare_dbl( p[1] , -0.93210419642070 , 1e-12 ));
      g_assert (eh_compare_dbl( p[2] ,  0.31026359387371 , 1e-12 ));

      eh_free( p );
      eh_free( y );
      eh_free( x );
   }

   {
      double x[2] = { 0 , 1 };
      double y[2] = { 1 , 4 };
      double* p = eh_poly_fit( x , y , 2 , 5 );

      g_assert (p==NULL);
   }

}


void
test_r_squared (void)
{
   {
      double x[3] = {0,1,2};
      double y[3] = {2,1,-8};
      double* p = eh_poly_fit( x , y , 3 , 2 );
      double r_sq = eh_poly_r_squared( x , y , 3 , p , 2 );

      g_assert (eh_compare_dbl( r_sq , 1. , 1e-12 ));

      eh_free( p );
   }
}


void
test_running_mean (void)
{
   {
      double x[5] = { 1,1,2,3,5 };

      eh_dbl_array_running_mean( x , 5 , 2 , 2 );

      g_assert (eh_compare_dbl( x[2] , 12./5. , 1e-12 ));
   }

   {
      double x[5] = { 1,1,2,3,5 };

      eh_dbl_array_running_mean( x , 5 , 0 , 4 );

      g_assert (eh_compare_dbl( x[0] , 12./5. , 1e-12 ));
   }

   {
      double x[5] = { 1,1,2,3,5 };

      eh_dbl_array_running_mean( x , 5 , 4 , 0 );

      g_assert (eh_compare_dbl( x[4] , 12./5. , 1e-12 ));
   }

   {
      double x[5] = { 1,1,2,3,5 };
      double y[5] = { 1,1,2,3,5 };

      eh_dbl_array_running_mean( x , 5 , 0 , 0 );

      g_assert (eh_dbl_array_compare(x,y,5,1e-12));
   }

   {
      double x[5] = { 1,1    ,2,3     ,5 };
      double y[5] = { 0,4./3.,2,10./3.,0 };

      eh_dbl_array_running_mean( x , 5 , 1 , 1 );

      g_assert (eh_dbl_array_compare(x+1,y+1,3,1e-12));
   }
}


void
test_rebin (void)
{
   double* s = eh_dbl_array_new( 100 );
   double* d;
   gint    len;

   eh_dbl_array_set( s , 100 , 1. );
   d = eh_dbl_array_rebin( s , 100 , 2. , &len );

   g_assert (d!=NULL);
   g_assert (len==50);
   g_assert (eh_compare_dbl(eh_dbl_array_sum(d,len),100,1e-12));

   eh_free( d );

   d = eh_dbl_array_rebin( s , 100 , 1.25 , &len );

   g_assert (d!=NULL);
   g_assert (len==80);
   g_assert (eh_compare_dbl(eh_dbl_array_sum(d,len),100,1e-12));

   eh_free( d );

   d = eh_dbl_array_rebin( s , 100 , sqrt(2) , &len );

   g_assert (d!=NULL);
   g_assert (len==71);
   g_assert (eh_compare_dbl(eh_dbl_array_sum(d,len),100,1e-12));

   eh_free( d );

   d = eh_dbl_array_rebin( s , 100 , 1. , &len );
   
   g_assert (d!=NULL);
   g_assert (d!=s);
   g_assert (len==100);
   g_assert (eh_dbl_array_compare(s,d,100,1e-12));

   d = eh_dbl_array_rebin( s , 100 , 1. , NULL );
   
   g_assert (d!=NULL);
   g_assert (eh_dbl_array_compare(s,d,100,1e-12));

   eh_free( d );
   eh_free( s );

   s = eh_linspace( 1 , 100 , 100 );
   d = eh_dbl_array_rebin( s , 100 , G_PI , &len );

   g_assert (d!=NULL);
   g_assert (eh_compare_dbl( eh_dbl_array_sum(s,100),
             eh_dbl_array_sum(d,len), 1e-12));

   eh_free( d );

   eh_dbl_array_set( s , 100 , 1. );
   d = eh_dbl_array_rebin( s , 100 , .5*sqrt(2.) , &len );

   g_assert (d!=NULL);
   g_assert (len==142);
   g_assert (eh_compare_dbl(eh_dbl_array_sum(d,len),100,1e-12));

   eh_free( d );

   eh_dbl_array_set( s , 100 , 1. );
   d = eh_dbl_array_rebin( s , 100 , .75 , &len );

   g_assert (d!=NULL);
   g_assert (len==134);
   g_assert (eh_compare_dbl(eh_dbl_array_sum(d,len),100,1e-12));

   eh_free( d );

}


void
test_convolve (void)
{
   gssize len_x = 32;
   double* x = eh_new( double , len_x );
   double* z;
   gssize i;

   for ( i=0 ; i<len_x ; i++ )
   {
      x[i] = pow(i,.5) + g_random_double()-.5;
   }

//   eh_dbl_array_fprint( stdout , x , len_x );

   z = eh_low_pass_filter( x , len_x );

//   eh_dbl_array_fprint( stdout , z , len_x );
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/num/core/nan", &test_nan );
  g_test_add_func ("/utils/num/core/interpolate", &test_interpolate );
  g_test_add_func ("/utils/num/core/interpolate_poly", &test_poly_interpolate );
  g_test_add_func ("/utils/num/core/trapazoid", &test_trapazoid );
  g_test_add_func ("/utils/num/core/integrate", &test_integrate );
  g_test_add_func ("/utils/num/core/compare_dbl", &test_compare_dbl );
  g_test_add_func ("/utils/num/core/round", &test_round );
  g_test_add_func ("/utils/num/core/reduce_angle", &test_reduce_angle );
  g_test_add_func ("/utils/num/core/is_even", &test_is_even );
  g_test_add_func ("/utils/num/core/diff", &test_diff );
  g_test_add_func ("/utils/num/core/binomial", &test_binomial_coef );
  g_test_add_func ("/utils/num/core/factorial", &test_factorial );
  g_test_add_func ("/utils/num/core/convolve", &test_convolve );
  g_test_add_func ("/utils/num/core/running_mean", &test_running_mean );
  g_test_add_func ("/utils/num/core/rebin", &test_rebin );

  g_test_add_func ("/utils/num/gamma/p", &test_gamma_p);
  g_test_add_func ("/utils/num/gamma/q", &test_gamma_q);
  g_test_add_func ("/utils/num/gamma/series", &test_gamma_series);
  g_test_add_func ("/utils/num/gamma/cf", &test_gamma_cf);
  g_test_add_func ("/utils/num/gamma/log", &test_gamma_log);

  g_test_add_func ("/utils/num/fit/linear", &test_linear_fit);
  g_test_add_func ("/utils/num/fit/poly", &test_poly_fit);
  g_test_add_func ("/utils/num/fit/r_squared", &test_r_squared);

  g_test_run ();
}

