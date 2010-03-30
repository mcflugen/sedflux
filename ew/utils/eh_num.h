//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#ifndef __EH_NUM_H__
#define __EH_NUM_H__

#include <stdio.h>
#include <glib.h>
#include <utils/complex.h>

typedef double (*Eh_root_fcn)( double x , gpointer user_data );
typedef double (Cost_fcn)( double*, int );
typedef double (*Eh_dbl_func_with_data)(double x , gpointer data );
typedef double (*Eh_dbl_func)( double x );

typedef double (*Eh_dbl_cpy_func)( gpointer data , double* d );
typedef double (*Eh_dbl_get_func)( gpointer data , gint ind );
typedef double (*Eh_dbl_set_func)( gpointer data , gint ind , double val );
typedef double (*Eh_dbl_add_func)( gpointer data , gint ind , double val );

typedef void   (*Eh_array_func)  ( gpointer data , gpointer user_data );

double eh_safe_dbl_division( double a , double b );
gboolean eh_compare_dbl( double , double , double );

double    eh_gamma_log     ( double xx );
double    eh_gamma_p       ( double a , double x );
double    eh_gamma_q       ( double a , double x );
void      eh_gamma_cf      ( double* gammcf , double a , double x , double* gln );
void      eh_gamma_series  ( double* gamser , double a , double x , double* gln );
double    eh_factorial     ( gssize n );
double    eh_factorial_log ( gssize n );
double    eh_binomial_coef ( gssize n , gssize y );

void interpolate(double*,double*,gssize,double*,double*,gssize);
void interpolate_bad_val(double*,double*,gssize,double*,double*,gssize,double);
double poly_interpolate( double* xa , double* ya , gssize n , double x , double* dy );

typedef void(*Eh_poly_basis_func)(double,double*,gssize);

void       poly_basis_funcs   ( double  x , double* p , gssize n );
double*    eh_poly_fit        ( double* x , double* y , gssize len , gssize n );
double     eh_poly_r_squared  ( double* x , double* y , gssize len , double* p , gssize n );
double     eh_r_squared       ( double* x , double* y , gssize len , Eh_dbl_func_with_data f , gpointer data );
void       eh_svdfit          ( double* x , double* y , double* sig , gssize ndata ,
                                double* a , gssize ma ,
                                double** u , double** v , double* w , double* chisq ,
                                Eh_poly_basis_func funcs );
double     eh_pythag          ( double a , double b );
void       eh_svbksb          ( double** u , double* w , double** v , gssize m , gssize n ,
                                double*  b , double* x );
void       eh_svdcmp          ( double** a , gssize m , gssize n , double* w , double** v );
double     eh_poly_eval       ( double  x , double* p , gssize n );
double*    eh_linear_fit      ( double* x , double* y , gssize len );
void       eh_fit             ( double* x , double* y , gssize len , double* sig , gboolean mwt ,
                                double* a , double* b , double* siga , double* sigb ,
                                double* chi2 , double* q );


double trapzd( Eh_dbl_func_with_data func , double a , double b , gssize n , gpointer data );
double qtrap( Eh_dbl_func_with_data func , double a , double b , gpointer data );
double eh_integrate( Eh_dbl_func_with_data func , double a , double b );
double eh_integrate_with_data( Eh_dbl_func_with_data func , double a , double b , gpointer data );

gboolean eh_is_even( gssize n );
double eh_round( double , double );
double eh_reduce_angle( double angle );
double *tridiag( double *l , double *d , double *u , double *b , double *x , int n );
Complex *c_tridiag( Complex *l , Complex *d , Complex *u , Complex *b , Complex *x , int n );
double rtsafe(void (*funcd)(double, double *, double *, double *), double x1, double x2, double xacc, double *);
double eh_bisection( Eh_root_fcn f , double x_0 , double x_1 , double eps , gpointer user_data );
double *anneal( double *x , int n , Cost_fcn *f , double cost_min );

double bessel_i_0( double x );
double bessel_k_0( double x );
double eh_kei_0( double x );

double eh_erf_inv( double y );

long int i1mach_( long int *i );
double d1mach_( long int *i );

double*    eh_dbl_array_new         ( gssize n );
double*    eh_dbl_array_new_set     ( gssize n , double val );
double*    eh_dbl_array_dup         ( double* s , gssize n );
double*    eh_dbl_array_copy        ( double* d , double* s , gssize n );
double*    eh_dbl_col_to_array      ( double* d , double* col , gint n , gssize offset );
double*    eh_dbl_array_to_col      ( double* d , double* s   , gint n , gssize offset );
double*    eh_dbl_array_rebin_smaller( double* s , gssize n , double bin_size , gint* d_len );
double*    eh_dbl_array_rebin_larger( double* s , gssize n , double bin_size , gint* d_len );
double*    eh_dbl_array_rebin       ( double* s , gssize n , double bin_size , gint* d_len );
double     eh_dbl_array_min         ( const double* x , gsize n );
gssize     eh_dbl_array_min_ind     ( const double* x , gssize n );
double     eh_dbl_array_max         ( const double* x , gsize n );
gssize     eh_dbl_array_max_ind     ( const double* x , gssize n );
double     eh_dbl_array_abs_max     ( const double* x , gsize n );
gssize     eh_dbl_array_fprint      ( FILE* fp , double* x , gssize n );
gint       eh_dbl_array_write       ( FILE *fp , double *x , gint len );
double     eh_dbl_array_mean        ( const double *x , gsize n );
double*    eh_dbl_array_normalize   ( double* x , gsize n );
double*    eh_dbl_array_foreach     ( double* x , gssize n , Eh_dbl_func f );
double*    eh_dbl_array_mult        ( double* x , gsize n , double a );
double*    eh_dbl_array_mult_each   ( double* d , gssize n , double* s );
double*    eh_dbl_array_add         ( double* dest , double* src , gssize n ) G_GNUC_DEPRECATED;
double*    eh_dbl_array_add_each    ( double* d , gssize n , double* s );
double     eh_dbl_array_var         ( const double* x , gsize n );
double     eh_dbl_array_sum         ( const double* x , gsize n );
double     eh_dbl_array_abs_sum     ( const double* x , gsize n );
void       eh_dbl_array_fabs        ( double* x , gsize n );
double*    eh_dbl_array_diff        ( double* d , const double *x , gsize len , gssize n );
double*    eh_dbl_array_gradient    ( const double* x , gsize n , double dx );
double*    eh_dbl_array_set         ( double *x , gsize n , double set_val );
double*    eh_dbl_array_grid        ( double *x , gsize n , double start , double dx );
double*    eh_dbl_array_running_mean( double* x , gssize len , gssize n_left , gssize n_right );
double*    eh_dbl_array_conv        ( double *x , gsize len_x , double *y , gsize len_y );
double*    eh_low_pass_filter       ( double* x , gssize len );
double*    eh_dbl_array_cum_mean_dir( double *x , gsize n , gboolean forward );
double*    eh_dbl_array_cum_sum_dir ( double *x , gsize n , gboolean forward );
double*    eh_dbl_array_cum_max_dir ( double *x , gsize n , gboolean forward );
double*    eh_dbl_array_cum_min_dir ( double *x , gsize n , gboolean forward );
gboolean   eh_dbl_array_compare     ( double *x , double *y , gssize len , double eps );
gboolean   eh_dbl_array_cmp_ge      ( double* x , double* y , gssize len );
gboolean   eh_dbl_array_each_ge     ( double val , double *x , gssize len );
gboolean   eh_dbl_array_each_le     ( double val , double *x , gssize len );

gboolean   eh_dbl_array_is_monotonic_up  ( double *x , gsize n );
gboolean   eh_dbl_array_is_monotonic_down( double *x , gsize n );
gboolean   eh_dbl_array_is_monotonic     ( double *x , gsize n );
double*    eh_linspace                   ( double x1 , double x2 , gssize n );
gint*      eh_id_array                   (gint i_0, gint i_1, gint* n);
double*    eh_uniform_array              ( double x1 , double x2 , double dx , gint* n );

double*    eh_dbl_array_linspace         ( double* x , gssize n_x ,  double x_0 , double dx );


#define eh_dbl_array_cum_mean( x , n )     eh_dbl_array_cum_mean_dir(x,n,TRUE )
#define eh_dbl_array_cum_mean_rev( x , n ) eh_dbl_array_cum_mean_dir(x,n,FALSE)
#define eh_dbl_array_cum_sum( x , n )      eh_dbl_array_cum_sum_dir (x,n,TRUE )
#define eh_dbl_array_cum_sum_rev( x , n )  eh_dbl_array_cum_sum_dir (x,n,FALSE)
#define eh_dbl_array_cum_max( x , n )      eh_dbl_array_cum_max_dir (x,n,TRUE )
#define eh_dbl_array_cum_max_rev( x , n )  eh_dbl_array_cum_max_dir (x,n,FALSE)
#define eh_dbl_array_cum_min( x , n )      eh_dbl_array_cum_min_dir (x,n,TRUE )
#define eh_dbl_array_cum_min_rev( x , n )  eh_dbl_array_cum_min_dir (x,n,FALSE)

void convlv(double data[], unsigned long n, double respns[], unsigned long m,
	int isign, double ans[]);
void four1(double data[], unsigned long nn, int isign);
void realft(double data[], unsigned long n, int isign);
void twofft(double data1[], double data2[], double fft1[], double fft2[],
	unsigned long n);

double eh_dbl_array_mean_weighted( const double x[] , gint len , const double f[] );

#ifndef HAVE_LOG2
double log2( double x );
#endif

#undef OLD_EH_NAN

#if defined( OLD_EH_NAN )
float    eh_nan  ( void     );
int      eh_isnan( float x  );
#else
double   eh_nan  ( void     );
gboolean eh_isnan( double x );
#endif

void eh_rebin_dbl_array        ( double *x     , double *y     , gssize len ,
                                 double *x_bin , double *y_bin , gssize len_bin );
void eh_rebin_dbl_array_bad_val( double *x     , double *y     , gssize len     ,
                                 double *x_bin , double *y_bin , gssize len_bin ,
                                 double bad_val );

typedef enum
{
   EH_NUM_IMPLICIT ,
   EH_NUM_EXPLICIT
}
Eh_num_method;

double* eh_dbl_array_diffuse_implicit( double* x , gint len , double c );
double* eh_dbl_array_diffuse_explicit( double* x , gint len , double c );
double* eh_dbl_array_diffuse         ( double* x , gint len , double c , Eh_num_method method );

#endif
