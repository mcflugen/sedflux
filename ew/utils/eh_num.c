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

#include <math.h>
#include <eh_utils.h>

double
eh_safe_dbl_division( double a , double b )
{
   return ( b<1 && a>b*G_MAXDOUBLE ) ? G_MAXDOUBLE : ( ( (b>1 && a<b*G_MINDOUBLE) || a==0 ) ? 0 : a/b );
}

gboolean
eh_compare_dbl( double a , double b , double eps )
{
   const gboolean STRONG = TRUE;
   double diff = fabs(a-b);
   double d1   = eh_safe_dbl_division( diff , fabs(a) );
   double d2   = eh_safe_dbl_division( diff , fabs(b) );

   if ( STRONG ) return (d1<=eps && d2<=eps);
   else          return (d1<=eps || d2<=eps);
}

/** Calculate the log of the gamma function

This is taken from Numerical Recipes

@param xx A floating point number

@return The log of the gamma function
*/
double eh_gamma_log( double xx )
{
   double x,y,tmp,ser;
   static double cof[6]={76.18009172947146,-86.50532032941677,
      24.01409824083091,-1.231739572450155,
      0.1208650973866179e-2,-0.5395239384953e-5};
   gssize j;

   y=x=xx;
   tmp=x+5.5;
   tmp -= (x+0.5)*log(tmp);
   ser=1.000000000190015;
   for (j=0;j<=5;j++) ser += cof[j]/++y;
   return -tmp+log(2.5066282746310005*ser/x);
}

/** The incomplete Gamma Function, \f$ P(a,x) \f$

This is taken from Numerical Recipes.

Calculate the Gamma function, \f$ P(a,b) \f$,

@param a A floating point number
@param x A floating point number

@return The incomplete gamma function
*/
double eh_gamma_p( double a , double x )
{
   double ans;
   double gamser,gammcf,gln;

   eh_require( x>=0 );
   eh_require( a>=0 );

   if (x < (a+1.0))
   {
      eh_gamma_series(&gamser,a,x,&gln);
      ans = gamser;
   } else {
      eh_gamma_cf(&gammcf,a,x,&gln);
      ans = 1.0-gammcf;
   }
   return ans;
}

/** The incomplete gamma function

This is taken from Numerical Recipes.

Calculate the incomplete Gamma function, \f$ Q(a,b) \f$,

\f[
Q(a,b) \equiv 1 - P(a,x)
\f]

@param a A floating point number
@param x A floating point number

@return The incomplete gamma function
*/
double eh_gamma_q( double a , double x )
{
   double ans;
   double gamser,gammcf,gln;

   eh_require( x >= 0 );
   eh_require( a >= 0 );

   if ( x < (a+1.0) )
   {
      eh_gamma_series(&gamser,a,x,&gln);
      ans = 1.0-gamser;
   }
   else
   {
      eh_gamma_cf(&gammcf,a,x,&gln);
      ans = gammcf;
   }

   return ans;
}

/** The incomplete gamma function, \f$ Q \f$

This is taken from Numerical Recipes.

Calculate the incomplete Gamma function, \f$ Q(a,b) \f$,

\f[
Q(a,b) \equiv 1 - P(a,x)
\f]

using its continued fraction representation.  This converges rapidly for
\f$ x > a+1 \f$.

@param gammcf The incomplete Gamma function evaluated at \f$ (a,x) \f$
@param a      A floating point number
@param x      A floating point number
@param gln    Log the the Gamma function at \f$ a \f$

@return The incomplete gamma function
*/
void eh_gamma_cf( double* gammcf , double a , double x , double* gln )
{
   gssize i;
   double an,b,c,d,del,h;
//   double EPS   = 3.0e-7;
//   double FPMIN = 1.0e-30;
   double EPS   = 1e-12;
   double FPMIN = G_MINDOUBLE;
   gint   ITMAX = 100;

   if ( x<a+1 )
      eh_warning( "This function should be used for x>a+1." );

   *gln=eh_gamma_log(a);
   b=x+1.0-a;
   c=1.0/FPMIN;
   d=1.0/b;
   h=d;
   for ( i=0 ; i<ITMAX ; i++ )
   {
      an = -(i+1)*((i+1)-a);
      b += 2.0;
      d=an*d+b;
      if (fabs(d) < FPMIN) d=FPMIN;
      c=b+an/c;
      if (fabs(c) < FPMIN) c=FPMIN;
      d=1.0/d;
      del=d*c;
      h *= del;
      if (fabs(del-1.0) < EPS) break;
   }
   if (i >= ITMAX) eh_warning("a (%f) too large, ITMAX (%d) too small in gamma_cf" , a , ITMAX );
   *gammcf=exp(-x+a*log(x)-(*gln))*h;
}

/** The incomplete gamma function \f$ P \f$

This is taken from Numerical Recipes.

Calculate the incomplete Gamma function, \f$ P(a,b) \f$, using its series representation.
This converges rapidly for \f$ x < a+1 \f$.

@param gamser The incomplete Gamma function evaluated at \f$ (a,x) \f$
@param a      A floating point number
@param x      A floating point number
@param gln    Log the the Gamma function at \f$ a \f$

@return The incomplete gamma function
*/
void eh_gamma_series( double* gamser , double a , double x , double* gln )
{
   gssize n;
   double sum,del,ap;
//   double EPS   = 3.0e-7;
   double EPS   = 1e-12;
   gint   ITMAX = 100;

   if ( x>a+1 )
      eh_warning( "This function should be used for x<a+1." );

   eh_require( x>=0 );

   *gln=eh_gamma_log(a);
   if ( eh_compare_dbl( x , 0 , 1e-12 ) )
   {
      *gamser=0.0;
   }
   else
   {
      ap=a;
      del=sum=1.0/a;
      for (n=0;n<ITMAX;n++)
      {
         ++ap;
         del *= x/ap;
         sum += del;
         if (fabs(del) < fabs(sum)*EPS) {
            *gamser=sum*exp(-x+a*log(x)-(*gln));
            return;
         }
      }
      eh_error("a too large, ITMAX too small in routine gamma_series: %f, %d." , a , ITMAX );
   }

   return;
}

/** Calculate the factorial of an integer.

This is taken from Numerical Recipes

@param n A non-negative integer

@return The factorial of the input argument
*/
double eh_factorial( gssize n )
{
   static gssize ntop=4;
   static double a[33]={1.0,1.0,2.0,6.0,24.0};
   gssize j;

   eh_return_val_if_fail( n>=0 , eh_nan() );

   if (n > 32)
      return exp(eh_gamma_log(n+1.0));

   while (ntop<n) {
      j=ntop++;
      a[ntop]=a[j]*ntop;
   }

   return a[n];
}

/** Calculate the log of a factorial of an integer.

This is taken from Numerical Recipes

@param n A non-negative integer

@return The log of the factorial of the input argument
*/
double eh_factorial_log( gssize n )
{
   static double a[101];

   eh_require( n>=0 );

   if (n <= 1)
      return 0.0;
   if (n <= 100)
      return a[n] ? a[n] : (a[n]=eh_gamma_log(n+1.0));
   else
      return eh_gamma_log(n+1.0);
}

/** Calculate the binomial coefficient

The binomial coefficent is defined as

\f[
n \choose y = { n! \over y! \left( n-y \right)! }
\f]

This is taken from Numerical Recipes

@param n A non-negative integer
@param y A non-negative integer

@return The binomial coefficient
*/
double eh_binomial_coef( gssize n , gssize y )
{
   return floor( .5+exp( eh_factorial_log(n) - eh_factorial_log(y) - eh_factorial_log(n-y) ) );
}

void interpolate( double *x     , double *y     , gssize len ,
                  double *x_new , double *y_new , gssize len_new )
{
   interpolate_bad_val( x , y , len , x_new , y_new , len_new , eh_nan() );
}

void interpolate_bad_val( double *x     , double *y     , gssize len     ,
                          double *x_new , double *y_new , gssize len_new ,
                          double bad_val )
{
   gint i,j;
   double m, b, x0;

   // initialize y_new with NaN's.
   for ( j=0 ; j<len_new ; y_new[j]=bad_val , j++ );

   // Make sure the x values are monotonically increasing.
   for ( i=1 ; i<len ; i++ )
   {
      if ( x[i]<=x[i-1] )
      {
         eh_error( "x values must be monotonically increasing" );
         eh_require_not_reached();
      }
   }

   if ( x_new[len_new-1]<x[0] || x_new[0]>x[len-1] )
      return;

   if ( len>1 )
   {
      // set j to the first index inside of the given data.
      for ( j=0 ; x_new[j]<x[0] ; j++ );

      // interpolate linearly between points.
      for ( i=0 ; i<len-1 ; i++ )
      {
         m = (y[i+1]-y[i])/(x[i+1]-x[i]);
         b = y[i];
         x0 = x[i];
         while ( j<len_new && x_new[j] <= x[i+1] )
         {
            y_new[j] = m*(x_new[j]-x0)+b;
            j++;
         }
      }
   }
   else
   {
      for ( i=0 ; i<len_new ; i++ )
         if ( x[0] == x_new[i] )
            y_new[i] = y[0];
   }

   return;
}

double poly_interpolate( double* xa , double* ya , gssize n , double x , double* dy )
{
   double y;
   double err;
   int i,m,ns=1;
   double den,dif,dift,ho,hp,w;
   double *c,*d;

   eh_require( xa );
   eh_require( ya );

   dif=fabs(x-xa[0]);
   c = eh_new( double , n );
   d = eh_new( double , n );
   for (i=0;i<n;i++)
   {
      if ( (dift=fabs(x-xa[i])) < dif)
      {
         ns=i;
         dif=dift;
      }
      c[i]=ya[i];
      d[i]=ya[i];
   }
   y=ya[ns--];
   for ( m=1 ; m<n ; m++ )
   {
      for (i=0;i<n-m;i++)
      {
         ho=xa[i]-x;
         hp=xa[i+m]-x;
         w=c[i+1]-d[i];
         if ( (den=ho-hp) == 0.0)
            eh_error("Error in routine polint");
         den=w/den;
         d[i]=hp*den;
         c[i]=ho*den;
      }
      y += (err=(2*ns < (n-m-1) ? c[ns+1] : d[ns--]));
   }

   if ( dy )
      *dy = err;

   eh_free( d );
   eh_free( c );

   return y;
}

void poly_basis_funcs( double x , double* p , gssize n )
{
   gssize i;

   p[0] = 1.;
   for ( i=1 ; i<n ; i++ )
      p[i] = p[i-1]*x;

   return;
}

double* eh_poly_fit( double* x , double* y , gssize len , gssize n )
{
   double* poly = NULL;

   if ( len > n )
   {
      double chisq;
      double** u = eh_new_2( double , len , n+1 );
      double** v = eh_new_2( double , len , n+1 );
      double*  w = eh_new  ( double , n+1 );
      double* sig = eh_new ( double , len );

      poly = eh_new( double , n+1 );

      {
         gssize i;
         for ( i=0 ; i<len ; i++ )
            sig[i] = 1.;
      }

      eh_svdfit(x,y,sig,len,poly,n+1,u,v,w,&chisq,poly_basis_funcs);

      eh_free  ( sig );
      eh_free  ( w );
      eh_free_2( v );
      eh_free_2( u );
   }

   return poly;
}

double eh_poly_r_squared( double* x , double* y , gssize len , double* p , gssize n )
{
   double ssr = 0;
   double sse = 0;
   double y_hat, y_bar;
   gssize i;

   y_bar = eh_dbl_array_mean( y , len );

   for ( i=0 ; i<len ; i++ )
   {
      y_hat = eh_poly_eval( x[i] , p , n );
      ssr += pow( y_hat - y_bar , 2 );
      sse += pow( y_hat - y[i]  , 2 );
   }

   return 1. - sse / ( ssr+sse );
}

double eh_r_squared( double* x , double* y , gssize len , Eh_dbl_func_with_data f , gpointer data )
{
   double ssr = 0;
   double sse = 0;
   double y_hat, y_bar;
   gssize i;

   y_bar = eh_dbl_array_mean( y , len );

   for ( i=0 ; i<len ; i++ )
   {
      y_hat = f( x[i] , data );
      ssr += pow( y_hat - y_bar , 2 );
      sse += pow( y_hat - y[i]  , 2 );
   }

   return 1. - sse / ( ssr+sse );
}

void eh_svdfit( double* x , double* y , double* sig , gssize ndata ,
                double* a , gssize ma , double** u , double** v , double* w , double* chisq ,
                Eh_poly_basis_func funcs )
{
   gssize j,i;
   double wmax,tmp,thresh,sum,*b,*afunc;
//   double TOL = 1e-5;
   double TOL = 1e-12;

   b     = eh_new( double , ndata );
   afunc = eh_new( double , ma    );

   for ( i=0 ; i<ndata ; i++ )
   {
      (*funcs)( x[i] , afunc , ma );
      tmp=1.0/sig[i];
      for ( j=0; j<ma ; j++ ) u[i][j]=afunc[j]*tmp;
      b[i]=y[i]*tmp;
   }

   eh_svdcmp(u,ndata,ma,w,v);

   wmax=0.0;
   for ( j=0 ; j<ma ; j++ )
      if ( w[j] > wmax) wmax=w[j];
   thresh=TOL*wmax;
   for ( j=0 ; j<ma ; j++ )
      if (w[j] < thresh) w[j]=0.0;

   eh_svbksb(u,w,v,ndata,ma,b,a);

   *chisq=0.0;
   for ( i=0 ; i<ndata ; i++ )
   {
      (*funcs)(x[i],afunc,ma);
      for (sum=0.0,j=0;j<ma;j++) sum += a[j]*afunc[j];
      *chisq += (tmp=(y[i]-sum)/sig[i],tmp*tmp);
   }
   eh_free( afunc );
   eh_free( b     );
}

double eh_pythag( double a , double b )
{
   double absa = fabs(a);
   double absb = fabs(b);

   if (absa > absb)
      return absa*sqrt(1.0+eh_sqr(absb/absa));
   else
      return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+eh_sqr(absa/absb)));
}

void eh_svbksb( double** u , double* w , double** v , gssize m , gssize n ,double* b , double* x )
{
   gssize jj,j,i;
   double s,*tmp;

   tmp = eh_new( double , n );
   for (j=0;j<n;j++)
   {
      s=0.0;
      if (w[j]) {
         for (i=0;i<m;i++) s += u[i][j]*b[i];
         s /= w[j];
      }
      tmp[j]=s;
   }
   for (j=0;j<n;j++)
   {
      s=0.0;
      for (jj=0;jj<n;jj++) s += v[j][jj]*tmp[jj];
      x[j]=s;
   }
   eh_free( tmp );
}

void eh_svdcmp( double** a , gssize m , gssize n , double* w , double** v )
{
   int flag,i,its,j,jj,k,l,nm;
   double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

   rv1 = eh_new( double , n );
   g=scale=anorm=0.0;
   for (i=0;i<n;i++) {
      l=i+1;
      rv1[i]=scale*g;
      g=s=scale=0.0;
      if (i <= m) {
         for (k=i;k<m;k++) scale += fabs(a[k][i]);
         if (scale) {
            for (k=i;k<m;k++) {
               a[k][i] /= scale;
               s += a[k][i]*a[k][i];
            }
            f=a[i][i];
            g = -eh_nrsign(sqrt(s),f);
            h=f*g-s;
            a[i][i]=f-g;
            for (j=l;j<n;j++) {
               for (s=0.0,k=i;k<m;k++) s += a[k][i]*a[k][j];
               f=s/h;
               for (k=i;k<m;k++) a[k][j] += f*a[k][i];
            }
            for (k=i;k<m;k++) a[k][i] *= scale;
         }
      }
      w[i]=scale *g;
      g=s=scale=0.0;
      if (i <= m && i != n) {
         for (k=l;k<n;k++) scale += fabs(a[i][k]);
         if (scale) {
            for (k=l;k<n;k++) {
               a[i][k] /= scale;
               s += a[i][k]*a[i][k];
            }
            f=a[i][l];
            g = -eh_nrsign(sqrt(s),f);
            h=f*g-s;
            a[i][l]=f-g;
            for (k=l;k<n;k++) rv1[k]=a[i][k]/h;
            for (j=l;j<m;j++) {
               for (s=0.0,k=l;k<n;k++) s += a[j][k]*a[i][k];
               for (k=l;k<n;k++) a[j][k] += s*rv1[k];
            }
            for (k=l;k<n;k++) a[i][k] *= scale;
         }
      }
      anorm=eh_max(anorm,(fabs(w[i])+fabs(rv1[i])));
   }
   for (i=n-1;i>=0;i--) {
      if (i < n) {
         if (g) {
            for (j=l;j<n;j++)
               v[j][i]=(a[i][j]/a[i][l])/g;
            for (j=l;j<n;j++) {
               for (s=0.0,k=l;k<n;k++) s += a[i][k]*v[k][j];
               for (k=l;k<n;k++) v[k][j] += s*v[k][i];
            }
         }
         for (j=l;j<n;j++) v[i][j]=v[j][i]=0.0;
      }
      v[i][i]=1.0;
      g=rv1[i];
      l=i;
   }
   for (i=eh_min(m,n)-1;i>=0;i--) {
      l=i+1;
      g=w[i];
      for (j=l;j<n;j++) a[i][j]=0.0;
      if (g) {
         g=1.0/g;
         for (j=l;j<n;j++) {
            for (s=0.0,k=l;k<m;k++) s += a[k][i]*a[k][j];
            f=(s/a[i][i])*g;
            for (k=i;k<m;k++) a[k][j] += f*a[k][i];
         }
         for (j=i;j<m;j++) a[j][i] *= g;
      } else for (j=i;j<m;j++) a[j][i]=0.0;
      ++a[i][i];
   }
   for (k=n-1;k>=0;k--) {
      for (its=0;its<30;its++) {
         flag=1;
         for (l=k;l>=0;l--) {
            nm=l-1;
            if ((double)(fabs(rv1[l])+anorm) == anorm) {
               flag=0;
               break;
            }
            if ((double)(fabs(w[nm])+anorm) == anorm) break;
         }
         if (flag) {
            c=0.0;
            s=1.0;
            for (i=l;i<k;i++) {
               f=s*rv1[i];
               rv1[i]=c*rv1[i];
               if ((double)(fabs(f)+anorm) == anorm) break;
               g=w[i];
               h=eh_pythag(f,g);
               w[i]=h;
               h=1.0/h;
               c=g*h;
               s = -f*h;
               for (j=0;j<m;j++) {
                  y=a[j][nm];
                  z=a[j][i];
                  a[j][nm]=y*c+z*s;
                  a[j][i]=z*c-y*s;
               }
            }
         }
         z=w[k];
         if (l == k) {
            if (z < 0.0) {
               w[k] = -z;
               for (j=0;j<n;j++) v[j][k] = -v[j][k];
            }
            break;
         }
         if (its == 30) eh_error("no convergence in 30 svdcmp iterations");
         x=w[l];
// NOTE: should this be nm=k ? Probably.  Used to be nm = k-1
         nm=k;

         y=w[nm];
         g=rv1[nm];
         h=rv1[k];
         f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
         g=eh_pythag(f,1.0);
         f=((x-z)*(x+z)+h*((y/(f+eh_nrsign(g,f)))-h))/x;
         c=s=1.0;
         for (j=l;j<nm;j++) {
            i=j+1;
            g=rv1[i];
            y=w[i];
            h=s*g;
            g=c*g;
            z=eh_pythag(f,h);
            rv1[j]=z;
            c=f/z;
            s=h/z;
            f=x*c+g*s;
            g = g*c-x*s;
            h=y*s;
            y *= c;
            for (jj=0;jj<n;jj++) {
               x=v[jj][j];
               z=v[jj][i];
               v[jj][j]=x*c+z*s;
               v[jj][i]=z*c-x*s;
            }
            z=eh_pythag(f,h);
            w[j]=z;
            if (z) {
               z=1.0/z;
               c=f*z;
               s=h*z;
            }
            f=c*g+s*y;
            x=c*y-s*g;
            for (jj=0;jj<m;jj++) {
               y=a[jj][j];
               z=a[jj][i];
               a[jj][j]=y*c+z*s;
               a[jj][i]=z*c-y*s;
            }
         }
         rv1[l]=0.0;
         rv1[k]=f;
         w[k]=x;
      }
   }
   eh_free( rv1 );
}

double eh_poly_eval( double x , double* p , gssize n )
{
   double val = 0.;
   gssize i;

   for ( i=n ; i>0 ; i-- )
      val = (val + p[i])*x;
   val += p[0];

   return val;
}

double* eh_linear_fit( double* x , double* y , gssize len )
{
   double* poly = NULL;

   eh_return_val_if_fail( x , NULL );
   eh_return_val_if_fail( y , NULL );

   if ( len>1 )
   {
      double sig_a, sig_b, chi_2, q;
      poly = eh_new( double , 2 );

      eh_fit( x , y , len , NULL , FALSE , poly , poly+1 , &sig_a , &sig_b , &chi_2 , &q );
   }

   return poly;
}

void eh_fit( double* x , double* y , gssize len , double* sig , gboolean mwt ,
             double* a , double* b , double* siga , double* sigb , double* chi2 , double* q )
{
   gssize i;
   double wt,t,sxoss,sx=0.0,sy=0.0,st2=0.0,ss,sigdat;

   *b=0.0;
   if ( mwt )
   {
      ss=0.0;
      for ( i=0 ; i<len ; i++ )
      {
         wt=1.0/eh_sqr(sig[i]);
         ss += wt;
         sx += x[i]*wt;
         sy += y[i]*wt;
      }
   }
   else
   {
      for ( i=0 ; i<len ; i++ )
      {
         sx += x[i];
         sy += y[i];
      }
      ss=len;
   }
   sxoss=sx/ss;
   if ( mwt )
   {
      for ( i=0 ; i<len ; i++ )
      {
         t=(x[i]-sxoss)/sig[i];
         st2 += t*t;
         *b += t*y[i]/sig[i];
      }
   }
   else
   {
      for ( i=0 ; i<len ; i++ )
      {
         t=x[i]-sxoss;
         st2 += t*t;
         *b += t*y[i];
      }
   }
   *b /= st2;
   *a=(sy-sx*(*b))/ss;
   *siga=sqrt((1.0+sx*sx/(ss*st2))/ss);
   *sigb=sqrt(1.0/st2);
   *chi2=0.0;
   if ( !mwt )
   {
      for ( i=0 ; i<len ; i++ )
         *chi2 += eh_sqr(y[i]-(*a)-(*b)*x[i]);
      *q=1.0;
      sigdat=sqrt((*chi2)/(len-2));
      *siga *= sigdat;
      *sigb *= sigdat;
   }
   else
   {
      for ( i=0 ; i<len ; i++ )
         *chi2 += eh_sqr((y[i]-(*a)-(*b)*x[i])/sig[i]);
      *q=eh_gamma_q(0.5*(len-2),0.5*(*chi2));
   }

   return;
}

double trapzd( Eh_dbl_func_with_data func , double a , double b , gssize n , gpointer data )
{
   double x,tnm,sum,del;
   static double s;
   gssize it,j;

   if (n == 1)
   {
      return (s=0.5*(b-a)*( func(a,data)+func(b,data) ));
   }
   else
   {
      for (it=1,j=1;j<n-1;j++) it <<= 1;

      tnm=it;
      del=(b-a)/tnm;
      x=a+0.5*del;
      for (sum=0.0,j=1;j<=it;j++,x+=del) sum += func(x,data);
      s=0.5*(s+(b-a)*sum/tnm);
      return s;
   }
}

#define EH_QTRAP_EPS 1.0e-5
#define EH_QTRAP_JMAX 20

double qtrap( Eh_dbl_func_with_data func , double a , double b , gpointer data )
{
   gssize j;
   double s, olds;

   olds = -1.0e30;
   for ( j=0 ; j<EH_QTRAP_JMAX ; j++ )
   {
      s = trapzd( func , a , b , j , data );
      if ( fabs(s-olds) < EH_QTRAP_EPS*fabs(olds) )
         return s;
      olds = s;
   }
   eh_error( "Too many steps in routine qtrap" );
   return 0.0;
}

#define EH_QROMB_EPS 1.0e-3
#define EH_QROMB_JMAX 20
#define EH_QROMB_JMAXP (EH_QROMB_JMAX+1)
#define EH_QROMB_K 5

double eh_integrate( Eh_dbl_func_with_data func , double a , double b )
{
   return eh_integrate_with_data( func , a , b , NULL );
}

double eh_integrate_with_data( Eh_dbl_func_with_data func , double a , double b , gpointer data )
{
   double ss,dss;
   double s[EH_QROMB_JMAXP+1],h[EH_QROMB_JMAXP+1];
   gssize j;

   h[0]=1.0;
   for (j=0;j<EH_QROMB_JMAX;j++)
   {
      s[j]=trapzd(func,a,b,j,data);
      if (j >= EH_QROMB_K)
      {
         ss = poly_interpolate(&h[j-EH_QROMB_K],&s[j-EH_QROMB_K],EH_QROMB_K,0.0,&dss);
         if (fabs(dss) < EH_QROMB_EPS*fabs(ss))
            return ss;
      }
      s[j+1]=s[j];
      h[j+1]=0.25*h[j];
   }
   eh_error("Too many steps in routine integrate");
   return 0.0;
}

/** Is this integer even

@param n An integer

@return TRUE if the integer is even
*/
gboolean eh_is_even( gssize n )
{
   return !(n%2);
}

/** Round-off a value

@param val Value to round
@param rnd Multiple to round to

@return The value rounded to the nearest multiple
*/
double eh_round( double val , double rnd )
{
   if ( !eh_compare_dbl( val , 0 , 1e-12 ) )
      return ((int)(val/rnd+(val>0?1:-1)*.5))*rnd;
   else
      return 0.;
}

/** Round-off a value to nearest integer

@param val Value to round
@param rnd Multiple to round to

@return The value rounded to the nearest multiple
*/
int
eh_round_to_int (double val, int rnd)
{
   if (!eh_compare_dbl (val, 0, 1e-12))
      return lround (val/rnd)*rnd;
   else
      return 0;
}

/** Find the equivalent angle between -PI and PI

\param angle An angle (in radians)

\return An angle between -PI and PI
*/
double eh_reduce_angle( double angle )
{
   return angle - 2*M_PI*floor(( angle + M_PI )/(2*M_PI)); 
}

/** Solve a tridiagonal set of equations.

Solve the system of linear equations for \f$ x \f$ in,

\f[ \left[ \matrix { 
   d_1 & u_1    & 0 \cr
   l_2 & d_2    & u_2    & \ddots \cr
   0   & \ddots & \ddots & \ddots & 0 \cr
       & \ddots & l_{n-1}& d_{n-1}& u_{n-1} \cr
       &        & 0      & l_n    & d_n } \right]
   \left\{ \matrix{ x_1\cr x_2\cr \vdots\cr x_{n-1}\cr x_n } \right\} =
   \left\{ \matrix{ b_1\cr b_2\cr \vdots\cr b_{n-1}\cr b_n } \right\}
\f]

@param l   entries on the lower diagonal.
@param d   entries on the diagonal.
@param u   entries on the upper diagonal.
@param b   entries of the right hand side column vector.
@param x   the output column vector.
@param n   the number of equations in the system.

*/
double*
tridiag( double *l , double *d , double *u , double *b , double *x , int n )
{
   if ( !eh_compare_dbl( d[0] , 0. , 1e-12 ) )
   {
      gint    i;
      double  beta  = d[0];
      double* gamma = eh_new( double , n );

      x[0] = b[0]/d[0];

      for ( i=1 ; i<n ; i++ )
      {
         gamma[i] = u[i-1]/beta;
         beta     = d[i] - l[i]*gamma[i];
         if ( beta == 0.0 )
         {
            eh_free( gamma );
            return NULL;
         }
         x[i] = (b[i]-l[i]*x[i-1])/beta;
      }

      for ( i=n-2 ; i>=0 ; i-- )
         x[i] -= gamma[i+1]*x[i+1];

      eh_free( gamma );
   }
   else
      x = NULL;

   return x;
}

#include "complex.h"

Complex *c_tridiag( Complex *l , Complex *d , Complex *u , Complex *b , Complex *x , int n )
{
   int i;
   Complex beta, *gamma;

   gamma = eh_new( Complex , n );

   if ( c_abs(d[0]) == 0.0 )
      return NULL;

   x[0] = c_div(b[0],d[0]);
   beta = d[0];

   for ( i=1 ; i<n ; i++ )
   {
      gamma[i] = c_div(u[i-1],beta);
      beta     = c_sub( d[i] , c_mul(l[i],gamma[i]) );
      if ( c_abs(beta) == 0.0 )
         return NULL;
      x[i] = c_div( c_sub(b[i],c_mul(l[i],x[i-1])) , beta );
   }

   for ( i=n-2 ; i>=0 ; i-- )
   {
      x[i] = c_sub( x[i] , c_mul(gamma[i+1],x[i+1]) );
   }

   eh_free( gamma );

   return u;
}

#include <math.h>

#define MAXIT 100

double rtsafe(void (*funcd)(double, double *, double *, double *), double x1, double x2, double xacc, double *data )
{
   int j;
   double df,dx,dxold,f,fh,fl;
   double temp,xh,xl,rts;

   (*funcd)(x1,&fl,&df,data);
   (*funcd)(x2,&fh,&df,data);
   if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0))
   {
      eh_warning( "Root must be bracketed in rtsafe" );
      return eh_nan();
   }
   if (fl == 0.0) return x1;
   if (fh == 0.0) return x2;
   if (fl < 0.0) {
      xl=x1;
      xh=x2;
   } else {
      xh=x1;
      xl=x2;
   }
   rts=0.5*(x1+x2);
   dxold=fabs(x2-x1);
   dx=dxold;
   (*funcd)(rts,&f,&df,data);
   for (j=1;j<=MAXIT;j++) {
      if ((((rts-xh)*df-f)*((rts-xl)*df-f) >= 0.0)
         || (fabs(2.0*f) > fabs(dxold*df))) {
         dxold=dx;
         dx=0.5*(xh-xl);
         rts=xl+dx;
         if (xl == rts) return rts;
      } else {
         dxold=dx;
         dx=f/df;
         temp=rts;
         rts -= dx;
         if (temp == rts) return rts;
      }
      if (fabs(dx) < xacc) return rts;
      (*funcd)(rts,&f,&df,data);
      if (f < 0.0)
         xl=rts;
      else
         xh=rts;
   }
   eh_warning( "Maximum number of iterations exceeded in rtsafe" );
   return eh_nan();
}
#undef MAXIT

void nrerror(const char error_text[])
{
   fprintf( stderr , "%s\n" , error_text );
   eh_exit(-1);
}

double eh_bisection( Eh_root_fcn f , double x_0 , double x_1 , double eps , gpointer user_data )
{
   double f_0, f_1, f_mid;
   double x_mid, dx;
   double rtb;
   int i;

   f_0 = (*f)( x_0 , user_data );
   f_1 = (*f)( x_1 , user_data );

   eh_return_val_if_fail( f_0*f_1<0 , eh_nan() );

   rtb = (f_0<0)?(dx=x_1-x_0,x_0):(dx=x_0-x_1,x_1);

   for ( i=0 ; i<40 ; i++ )
   {
      dx    *= .5;
      x_mid  = rtb + dx;
      f_mid  = (*f)( x_mid , user_data );

      if ( f_mid<=0 )
         rtb = x_mid;
      if ( fabs(dx)<eps || f_mid==0. )
         return rtb;
   }

   eh_return_val_if_fail( FALSE , eh_nan() );
}

#define MAXIT 3000

double *anneal( double *x , int n , Cost_fcn *f , double cost_min )
{
   double cost_before, cost_after;
   int i, j;
   int itr=0, max_itr=MAXIT;

   do
   {

      cost_before = (*f)( x , n );

      i = eh_get_fuzzy_int( 0 , n-1 );
      do
      {
         j = eh_get_fuzzy_int( 0 , n-1 );
      }
      while ( j==i );
   
      swap_dbl_vec( x , i , j );

      cost_after = (*f)( x , n );

      if ( cost_after>cost_before )
         swap_dbl_vec( x , i , j );

   }
   while ( cost_after>cost_min && ++itr<max_itr );

   return x;
}

#undef MAXIT

double bessel_i_0( double x )
{
   double ax,ans;
   double y;

   if ((ax=fabs(x)) < 3.75)
   {
      y=x/3.75;
      y*=y;
      ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
         +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
   }
   else
   {
      y=3.75/ax;
      ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
         +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
         +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
         +y*0.392377e-2))))))));
   }

   return ans;
}

double bessel_k_0( double x )
{
   double y,ans;

   if (x <= 2.0) {
      y=x*x/4.0;
      ans=(-log(x/2.0)*bessel_i_0(x))+(-0.57721566+y*(0.42278420
         +y*(0.23069756+y*(0.3488590e-1+y*(0.262698e-2
         +y*(0.10750e-3+y*0.74e-5))))));
   } else {
      y=2.0/x;
      ans=(exp(-x)/sqrt(x))*(1.25331414+y*(-0.7832358e-1
         +y*(0.2189568e-1+y*(-0.1062446e-1+y*(0.587872e-2
         +y*(-0.251540e-2+y*0.53208e-3))))));
   }

   return ans;
}

void zbesk_( double* , double* , double* , long int* , long int* , double* , double* , long int* , long int* );

double eh_kei_0( double x )
{
   double n=0;
   long int n_mem=1;
   long int kode=1;
   long int n_err=0;
   long int err=0;
   double z[2];
   double ans[2];

   eh_require( x>=0 );

   z[0] = x*cos(M_PI/4.);
   z[1] = x*sin(M_PI/4.);

   if ( z[0]<1e-5 && z[1]<1e-5 )
      ans[1] = -M_PI/4.;
   else
      zbesk_( &(z[0])   , &(z[1])   , &n     , &kode , &n_mem ,
              &(ans[0]) , &(ans[1]) , &n_err , &err );

   if ( err )
   {
      switch ( err )
      {
         case 1:
            eh_message( "Illegal arguments." );
            break;
         case 2:
            eh_message( "Overflow." );
            break;
         case 3:
            eh_message( "Some loss of accuracy in argument reduction." );
            break;
         case 4:
            eh_message( "Complete loss of accuracy, z or nu too large." );
            break;
         case 5:
            eh_message( "No convergence." );
            break;
         default:
            eh_error( "Illegal error flag." );
      }
   }

   return ans[1];
}

#include <math.h>

/** Inverse error function.

X = ERFINV(Y) is the inverse error function for each element of Y.
The inverse error function satisfies y = erf(x), for -1 <= y <= 1
and -inf <= x <= inf.

@param y

@return x

*/
double eh_erf_inv( double y )
{
   double x;
   double y_0, z, u;
   static double a[4] =
      {  0.886226899 , -1.645349621 ,  0.914624893 , -0.140543331 };
   static double b[4] =
      { -2.118377725 ,  1.442710462 , -0.329097515 ,  0.012229801 };
   static double c[4] =
      { -1.970840454 , -1.624906493 ,  3.429567803 ,  1.641345311 };
   static double d[2] =
      {  3.543889200 ,  1.637067800 };

   // Exceptional cases.
   if ( y == -1 )
      return -1./0.;
   if ( y ==  1 )
      return +1./0.;
   if ( fabs(y) > 1 )
      return eh_nan();
   if ( eh_isnan(y) )
      return eh_nan();

   // Central range.
   y_0 = .7;
   if ( fabs(y) < y_0 )
   {
      z = pow(y,2);
      x = y
        *   ( ( (a[3]*z + a[2])*z + a[1])*z + a[0])
        / ( ( ( (b[3]*z + b[2])*z + b[1])*z + b[0])*z + 1. );
   }

   // Near end points of range.
   else if ( y_0<y && y<1 )
   {
      z = sqrt( -log((1.-y)/2.) );
      x = (((c[3]*z+c[2])*z+c[1])*z+c[0])
        / ((d[1]*z+d[0])*z+1);
   }

   else if ( -y_0>y && y>-1 )
   {
      z = sqrt( -log((1.+y)/2.) );
      x = -(((c[3]*z+c[2])*z+c[1])*z+c[0])
        /  ((d[1]*z+d[0])*z+1);
   }

   //---
   // The relative error of the approximation has absolute value less
   // than 8.9e-7.  One iteration of Halley's rational method (third
   // order) gives full machine precision.
   //
   // Newton's method: new x = x - f/f'
   // Halley's method: new x = x - 1/(f'/f - (f"/f')/2)
   // This function: f = erf(x) - y, f' = 2/sqrt(pi)*exp(-x^2), f" = -2*x*f'
   //---

   // Newton's correction.
   u = ( erf(x) - y ) / ( 2./sqrt(M_PI) * exp(-x*x));

   // Halley's step.
   x = x - u / (1+x*u);

   return x;
}

long int i1mach_( long int *i )
{
   switch (*i)
   {
      case 9:
         return G_MAXLONG;
      case 14:
         return 53;
      case 15:
         return -1021;
      case 16:
         return 1024;
      default:
         eh_require_not_reached();
         return G_MINLONG;
   }

}

double d1mach_( long int *i )
{
   switch ( *i )
   {
      case 1:
         return G_MINDOUBLE;
      case 2:
         return G_MAXDOUBLE;
      case 4:
         return pow(2,1-53);
      case 5:
         return 0.30102999566398;
      default:   
         eh_require_not_reached();
         return eh_nan();
   }
}

#if defined( ENABLE_BLAS )
#include <cblas.h>
#endif

double* eh_dbl_array_new( gssize n )
{
   double* x = NULL;

   if ( n>0 )
      x = eh_new( double , n );

   return x;
}

double* eh_dbl_array_new_set( gssize n , double val )
{
   double* x = NULL;

   if ( n>0 )
   {
      gssize i;
      x = eh_new( double , n );
      for ( i=0 ; i<n ; i++ )
         x[i] = val;
   }

   return x;
}

double*
eh_dbl_array_dup( double* s , gssize n )
{
   return eh_dbl_array_copy( NULL , s , n );
}

double*
eh_dbl_array_copy( double* d , double* s , gssize n )
{
   if ( s )
   {
      if ( !d )
         d = eh_dbl_array_new( n );

      g_memmove( d , s , sizeof(double)*n );
   }

   return d;
}

double*
eh_dbl_col_to_array( double* d , double* col , gint n , gssize offset )
{
   if ( col )
   {
      gint i;

      if ( !d )
         d = eh_dbl_array_new( n );

      for ( i=0 ; i<n ; i++ )
         d[i] = col[i*offset];
   }
   return d;
}

double*
eh_dbl_array_to_col( double* d , double* s , gint n , gssize offset )
{
   if ( s )
   {
      gint i;

      if ( !d )
         d = eh_new( double , n*offset );

      for ( i=0 ; i<n ; i++ )
         d[i*offset] = s[i];
   
   }
   return d;
}

gint
eh_dbl_array_rebin_len( double* s , gssize n , double bin_size )
{
   gint len = 0;

   if ( s )
      len = lround (ceil (n/bin_size));

   return len;
}

double*
eh_dbl_array_rebin_smaller( double* s , gssize n , double bin_size , gint* d_len )
{
   double* d = NULL;

   eh_require( bin_size<=1. );

   if      ( eh_compare_dbl( bin_size , 1. , 1e-12 ) )
   {
      d = eh_dbl_array_dup( s , n );
      if ( d_len )
         *d_len = n;
   }
   else if ( s )
   {
      gint   len   = lround (ceil (n/bin_size));
      gint   top_i = (gint) floor (n/bin_size);
      gint   i, j;
      double x;
      double f;

      d = eh_new( double , len );

      f    = 1.-bin_size;
      d[0] = bin_size*s[0];
      for ( i=1,x=bin_size ; i<top_i ; i++,x+=bin_size )
      {
         j = (gint)(x+bin_size);
         d[i]  = s[j-1]*f + s[j]*(bin_size-f);
         f     = 1. - (bin_size-f);
         f     = f - (gint)f;
      }

      if ( len!=top_i )
      {
         d[i] = s[n-1]*f;
      }

      if ( d_len )
         *d_len = len;
   }

   return d;
}

double*
eh_dbl_array_rebin_larger( double* s , gssize n , double bin_size , gint* d_len )
{
   double* d = NULL;

   eh_require( bin_size>=1. );

   if      ( eh_compare_dbl( bin_size , 1. , 1e-12 ) )
   {
      d = eh_dbl_array_dup( s , n );
      if ( d_len )
         *d_len = n;
   }
   else if ( s )
   {
      gint   len   = lround (ceil (n/bin_size));
      gint   top_i = (int )floor (n/bin_size);
      gint   i;
      double x;

      d = eh_new( double , len );

      for ( i=0,x=0 ; i<top_i ; i++,x+=bin_size )
      {
         d[i]  = eh_dbl_array_sum( s+(gint)x , (gint)(x+bin_size - (gint)x ) );
         d[i] -= (x          - (gint)(x)         )*s[(gint)(x)         ];
         d[i] += (x+bin_size - (gint)(x+bin_size))*s[(gint)(x+bin_size)];
      }

      if ( len!=top_i )
      {
         gint n_bins = n - (gint)x;

         d[i]  = eh_dbl_array_sum( s+(gint)x , n_bins );
         d[i] -= (x - (gint)(x) )*s[(gint)(x)         ];
      }

      if ( d_len )
         *d_len = len;
   }

   return d;
}

double*
eh_dbl_array_rebin( double* s , gssize n , double bin_size , gint* d_len )
{
   double* d = NULL;

   if ( bin_size<1. )
      d = eh_dbl_array_rebin_smaller( s , n , bin_size , d_len );
   else
      d = eh_dbl_array_rebin_larger ( s , n , bin_size , d_len );

   return d;
}

gssize eh_dbl_array_min_ind( const double* x , gssize n )
{
   gssize ind = -1;

   eh_require( x )
   {
      double min = G_MAXDOUBLE;
      gssize i;
      for ( i=0 ; i<n ; i++ )
         if ( x[i]<min )
         {
            min = x[i];
            ind = i;
         }
   }
   return ind;
}

double eh_dbl_array_min( const double* x , gsize n )
{
   double min = G_MAXDOUBLE;

   eh_require( x )
   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         if ( x[i]<min )
            min = x[i];
   }
   return min;
}

gssize eh_dbl_array_max_ind( const double* x , gssize n )
{
   gssize ind = -1;

   eh_require( x )

   {
      double max = -G_MAXDOUBLE;
      gssize i;
      for ( i=0 ; i<n ; i++ )
         if ( x[i]>max )
         {
            max = x[i];
            ind = i;
         }
   }

   return ind;
}

double eh_dbl_array_max( const double* x , gsize n )
{
   double max = -G_MAXDOUBLE;

   eh_require( x )

   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         if ( x[i]>max )
            max = x[i];
   }

   return max;
}

double eh_dbl_array_abs_max( const double* x , gsize n )
{
   double max = -G_MAXDOUBLE;

   eh_require( x )

#if !defined( ENABLE_BLAS )
   {
      gssize i;
      double val;
      for ( i=0 ; i<n ; i++ )
      {
         val = fabs( x[i] );
         if ( val>max )
            max = val;
      }
   }
#else
   {
      gssize i = cblas_idamax( n , x , 1 );
      max = x[i];
   }
#endif
   return max;
}

gssize
eh_dbl_array_fprint( FILE* fp , double* x , gssize n )
{
   gssize total_bytes = 0;

   eh_require( x  );
   eh_require( fp );

   {
      gssize i, top_i=n-1;
      for ( i=0 ; i<top_i ; i++ )
         total_bytes += fprintf( fp , "%f " , x[i] );
      total_bytes = fprintf( fp , "%f\n" , x[i] );
   }

   return total_bytes;
}

gint
eh_dbl_array_write( FILE *fp , double *x , gint len )
{
   size_t s=0;

   eh_require( fp );
   eh_require( x  );

   if ( fp && x )
   {
      gint n_i = len;
      gint n, i, i_0;
      gint el_size = sizeof(double);
      gint one = 1, size;
      gdouble this_val;

      for ( i_0=0 ; i_0<n_i ; i_0+=n )
      {
         if ( i_0==n_i-1 || x[i_0] == x[i_0+1] )
         {
            this_val = x[i_0];

            for ( i=i_0,n=0 ;
                  i<n_i && x[i]==this_val ;
                  i++,n++ );

            s += fwrite( &el_size  , sizeof(int) , 1 , fp )*sizeof(int);
            s += fwrite( &n        , sizeof(int) , 1 , fp )*sizeof(int);
            s += fwrite( &this_val , el_size     , 1 , fp )*el_size;
         }
         else
         {
            for ( i=i_0+1,n=1 ;
                  i<n_i && x[i-1]!=x[i] ;
                  i++,n++ );

            if ( i<n_i )
               n--;

            size = n*el_size;

            s += fwrite( &size      , sizeof(int) , 1 , fp )*sizeof(int);
            s += fwrite( &one       , sizeof(int) , 1 , fp )*sizeof(int);
            s += fwrite( &(x[i_0])  , size        , 1 , fp )*size;
         }
      }
   }

   return s;
}

double eh_dbl_array_mean( const double *x , gsize n )
{
   return eh_dbl_array_sum( x , n )/(double)n;
}

double* eh_dbl_array_normalize( double* x , gsize n )
{
   eh_require( x )
   {
      double sum = eh_dbl_array_sum( x , n );
      if ( fabs(sum)>1e-12 )
         eh_dbl_array_mult( x , n , 1./sum );
   }
   return x;
}

double* eh_dbl_array_foreach( double* x , gssize n , Eh_dbl_func f )
{
   eh_require( x   );
   eh_require( n>0 );

   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         x[i] = f(x[i]);
   }

   return x;
}

double* eh_dbl_array_add_scalar( double* x , gssize n , double a )
{
   eh_require( x   );
   eh_require( n>0 );

   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         x[i] += a;
   }

   return x;
}

double*
eh_dbl_array_add_each( double* d , gssize n , double* s )
{
   eh_require( d );
   eh_require( s );

#if !defined( ENABLE_BLAS )
   if ( d && s )
   {
      gint i;
      for ( i=0 ; i<n ; i++ )
         d[i] += s[i];
   }
#else
   cblas_daxpy( n , 1. , s , 1 , d , 1 );
#endif
   return d;
}

double* eh_dbl_array_add( double* dest , double* src , gssize n )
{
   eh_require( dest );
   eh_require( src  );

#if !defined( ENABLE_BLAS )
   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         dest[i] += src[i];
   }
#else
   cblas_daxpy( n , 1. , src , 1 , dest , 1 );
#endif

   return dest;
}

double* eh_dbl_array_mult( double* x , gsize n , double a )
{
   eh_require( x );
#if !defined( ENABLE_BLAS )
   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         x[i] *= a;
   }
#else
   cblas_dscal( n , a , x , 1 );
#endif
   return x;
}

double*
eh_dbl_array_mult_each( double* x , gssize n , double* y )
{
   eh_require( x );
   eh_require( y );

   if ( x && y )
   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         x[i] *= y[i];
   }
   return x;
}

double eh_dbl_array_var( const double* x , gsize n )
{
   double var = eh_nan();

   eh_require( x )
   {
      gssize i;
      double m = eh_dbl_array_mean( x , n );
      for ( var=0,i=0 ; i<n ; i++ )
         var += pow( x[i] - m , 2. );
      var /= (double)n;
   }

   return var;
}

double
eh_dbl_array_sum( const double *x , gsize n )
{
   double sum=0;

   eh_return_val_if_fail( n>0 , eh_nan() );
   eh_return_val_if_fail( x   , eh_nan() );

   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         sum += x[i];
   }

   return sum;
}

double eh_dbl_array_abs_sum( const double *x , gsize n )
{
   double sum=0;

   eh_return_val_if_fail( n>0 , eh_nan() );
   eh_return_val_if_fail( x   , eh_nan() );

#if !defined( ENABLE_BLAS )
   {
      gssize i;
      for ( i=0 ; i<n ; i++ )
         sum += fabs(x[i]);
   }
#else
   sum = cblas_dasum( n , x , 1 );
#endif
   return sum;
}

void eh_dbl_array_fabs( double* x , gsize n )
{
   eh_require( x );
   eh_require( n>0 );
   if ( x && n>0 )
   {
      gsize i;
      for ( i=0 ; i<n ; i++ )
         x[i] = fabs(x[i]);
   }
}

double *eh_dbl_array_diff( double *d , const double *x , gsize len , gssize n )
{
   eh_return_val_if_fail( x     , NULL );
   eh_return_val_if_fail( len>0 , NULL );

   eh_require( n>=0 );

   if ( !d )
      d = eh_new0( double , len );

   if ( n>0 )
   {
      gssize i, j;
      double* c = eh_new( double , n+1 );

      for ( j=0 ; j<=n ; j++ )
         c[j] = eh_binomial_coef(n,j);

      if ( eh_is_even( n ) )
      {
         gssize k = n/2;
         gssize top_i = len-k-1;

         for ( i=k ; i<=top_i ; i++ )
         {
            for ( j=0 ; j<=n ; j+=2 )
               d[i] += c[j]*x[i+k-j];

            for ( j=1 ; j<=n ; j+=2 )
               d[i] -= c[j]*x[i+k-j];
         }

         for ( i=0 ; i<k ; i++ )
            d[i] = d[k];
         for ( i=top_i+1 ; i<len ; i++ )
            d[i] = d[top_i];

      }
      else
      {
         gssize k = (n-1)/2;
         gssize top_i = len-k-2;

         for ( i=k ; i<=top_i ; i++ )
         {
            for ( j=0,d[i]=0 ; j<=n ; j+=2 )
               d[i] += c[j]*x[i+k+1-j];

            for ( j=1 ; j<=n ; j+=2 )
               d[i] -= c[j]*x[i+k+1-j];
         }

         for ( i=0 ; i<k ; i++ )
            d[i] = d[k];
         for ( i=top_i+1 ; i<len ; i++ )
            d[i] = d[top_i];
      }

      eh_free( c );
   }
   else
      g_memmove( d , x , sizeof(double)*len );

   return d;
}

double* eh_dbl_array_gradient( const double *y , gsize n , double dx )
{
   eh_require( y    );
   eh_require( n>1  );
   eh_require( dx>0 );

   if ( n>1 )
   {
      double* dy_dx = eh_new( double , n );
      double two_dx = 2.*dx;
      gsize   top_n = n-1;
      gsize i;

      for ( i=1 ; i<top_n ; i++ )
         dy_dx[i] = (y[i+1]-y[i-1]) / two_dx;
      dy_dx[0]     = (y[1]-y[0])/dx;
      dy_dx[top_n] = (y[top_n]-y[top_n-1])/dx;

      return dy_dx;
   }
   else
      return NULL;
      
}

double*
eh_dbl_array_set( double *x , gsize n , double set_val )
{
   gsize i;

   if ( !x ) x = eh_new( double , n );

   for ( i=0 ; i<n ; i++ )
      x[i] = set_val;

   return x;
}

double *eh_dbl_array_grid( double *x , gsize n , double start , double dx )
{
   gssize i;

   for ( i=1,x[0]=start ; i<n ; i++ )
      x[i] = x[i-1]+dx;

   return x;
}

double* eh_dbl_array_running_mean( double* x , gssize len , gssize n_left , gssize n_right )
{
   eh_require( n_left+n_right+1 <= len );
   eh_require( n_left>=0  );
   eh_require( n_right>=0 );

   eh_return_val_if_fail( x!=NULL , NULL );

   if ( n_left>0 || n_right>0 )
   {
      gint64 i;
      gint64 top_i = len - n_right;
      gint64 win_size = n_left + n_right + 1;
      double a = 1./win_size;
      double mean = 0;

      eh_dbl_array_mult( x , len , a );
      mean = eh_dbl_array_sum( x , win_size );
         
      x[n_left] = mean;
      for ( i=n_left+1 ; i<top_i ; i++ )
      {
         mean += x[i+n_right] - x[i-n_left];
         x[i] = mean;
      }
   }

   return x;
}


double *eh_dbl_array_conv( double *x , gsize len_x , double *y , gsize len_y )
{
   double *ans, *data, *resp;
   gulong n;

   if ( len_x==0 || !x || len_y==0 || !y || len_y>len_x )
      return NULL;

   n = len_x+len_y;

   if ( g_bit_nth_msf(n,-1)!=g_bit_nth_lsf(n,-1) )
      n = 1<<( g_bit_nth_msf( n , -1 )+1 );

   data = eh_new0( double , n );
   resp = eh_new0( double , n );
   ans  = eh_new0( double , 2*n );

   memcpy( data         , x , sizeof(double)*len_x );
   memcpy( resp         , y , sizeof(double)*len_y );

   convlv( data-1 , n , resp-1 , len_y , 1 , ans-1 );

   eh_free( data );
   eh_free( resp );

   return ans;
}

void savgol(double* c, gssize np , gssize nl , gssize nr , gssize ld , gssize m );
void convlv(double data[], unsigned long n, double respns[], unsigned long m,
	int isign, double ans[]);

double* eh_low_pass_filter( double* x , gssize len )
{
   gssize n_left  = 2;
   gssize n_right = 2;
   gssize ld = 0;
   gssize m  = 0;
   gssize np = 5;
   double* ans = eh_new0( double , 2*len );
   double* c   = eh_new0( double , len   );

   savgol( c-1 , np , n_left , n_right , ld , m );

   convlv( x-1 , len , c-1 , np , 1 , ans-1 );

   eh_free( c );

   return ans;
}

double *eh_dbl_array_cum_mean_dir( double *x , gsize n , gboolean forward )
{
   gssize i;
   double *mean;
   if ( n==0 || !x )
      return NULL;
   mean = eh_dbl_array_cum_sum_dir( x , n , forward );
   if ( forward )
      for ( i=1 ; i<n ; i++ )
         mean[i] = mean[i]/((double)i+1.);
   else
      for ( i=n-2 ; i>=0 ; i-- )
         mean[i] = mean[i]/((double)(n-i));
   return mean;
}

double *eh_dbl_array_cum_sum_dir( double *x , gsize n , gboolean forward )
{
   gssize i;
   double *sum;

   if ( n==0 || !x )
      return NULL;
   sum = eh_new( double , n );
   if ( forward )
      for ( sum[0]=x[0],i=1 ; i<n ; i++ )
         sum[i] = sum[i-1]+x[i];
   else
      for ( sum[n-1]=x[n-1],i=n-2 ; i>=0 ; i-- )
         sum[i] = sum[i+1]+x[i];
   return sum;
}

double *eh_dbl_array_cum_max_dir( double *x , gsize n , gboolean forward )
{
   gssize i;
   double *max;
   if ( n==0 || !x )
      return NULL;
   max = eh_new( double , n );
   if ( forward )
      for ( max[0]=x[0],i=1 ; i<n ; i++ )
         max[i] = (x[i]>max[i-1])?x[i]:max[i-1];
   else
      for ( max[n-1]=x[n-1],i=n-2 ; i>=0 ; i-- )
         max[i] = (x[i]>max[i+1])?x[i]:max[i+1];
   return max;
}

double *eh_dbl_array_cum_min_dir( double *x , gsize n , gboolean forward )
{
   gssize i;
   double *min;
   if ( n==0 || !x )
      return NULL;
   min = eh_new( double , n );
   if ( forward )
      for ( min[0]=x[0],i=1 ; i<n ; i++ )
         min[i] = (x[i]<min[i-1])?x[i]:min[i-1];
   else
      for ( min[n-1]=x[n-1],i=n-2 ; n>=0 ; i-- )
         min[i] = (x[i]<min[i+1])?x[i]:min[i+1];
   return min;
}

gboolean eh_dbl_array_compare( double *x , double *y , gssize len , double eps )
{
   gboolean is_same = TRUE;

   if ( x!=y && (x && y) )
   {
      gssize i;
      for ( i=0 ; i<len && !is_same ; i++ )
         if ( !eh_compare_dbl(x[i],y[i],eps) )
            is_same = FALSE;
   }

   return is_same;
}

gboolean
eh_dbl_array_cmp_ge( double* x , double* y , gssize len )
{
   gboolean is_ge = TRUE;

   if ( x && y && ( x!=y ) )
   {
      gint i;
      for ( i=0 ; i<len && is_ge ; i++ )
         is_ge = is_ge && ( x[i] >= y[i] );
   }
   else
      is_ge = FALSE;

   return is_ge;
}

gboolean
eh_dbl_array_each_ge( double val , double *x , gssize len )
{
   gboolean is_ge = TRUE;

   eh_return_val_if_fail( x , FALSE );

   {
      gint i;
      for ( i=0 ; i<len && is_ge ; i++ )
         is_ge = is_ge && ( x[i] >= val );
   }

   return is_ge;
}

gboolean
eh_dbl_array_each_le( double val , double *x , gssize len )
{
   gboolean is_le = TRUE;

   eh_return_val_if_fail( x , FALSE );

   {
      gint i;
      for ( i=0 ; i<len && is_le ; i++ )
         is_le = is_le && ( x[i] <= val );
   }

   return is_le;
}

gboolean
eh_dbl_array_is_monotonic_up ( double *x , gsize n )
{
   gsize i;

   for ( i=1 ; i<n ; i++ )
      if ( x[i-1] >= x[i] )
         return FALSE;

   return TRUE;
}

gboolean
eh_dbl_array_is_monotonic_down ( double *x , gsize n )
{
   gsize i;

   for ( i=1 ; i<n ; i++ )
      if ( x[i-1] <= x[i] )
         return FALSE;

   return TRUE;
}

/** Generate an array of equally spaced numbers.

Generate an array of \a n linearly equally spaced points between \a x1 and
\a x2.  returns a pointer to a newly allocated array.  should be freed
using eh_free.

\param x1 Lower bound
\param x2 Lower bound
\param n  Number of points

\return An array of equally spaced points

\see eh_logspace

*/
double *eh_linspace( double x1 , double x2 , gssize n )
{
   double *x = NULL;

   eh_require( !eh_compare_dbl(x1,x2,1e-12) );
   eh_require( n>1 );

   {
      gssize i;
      double dx   = fabs(x2-x1)/(n-1.);
      gssize sign = (x2>x1)?1:-1;

      x = eh_new( double , n );
   
      for ( i=1 , x[0]=x1 ; i<n ; i++ )
         x[i] = x[i-1] + sign*dx;
   }

   return x;
}

gint*
eh_id_array (gint i_0, gint i_1, gint* n)
{
   gint* id = NULL;

   eh_require( i_1>=i_0 );

   if ( i_1>=i_0 )
   {
      gint i;
      gint len = i_1 - i_0 + 1;

      id = eh_new (gint, len+1);

      for ( i=0 ; i<len ; i++ )
         id[i] = i_0+i;
      id[len] = -1;

      if ( n )
         *n = len;
   }
   else
   {
      if ( n )
         *n = 0;
   }

   return id;
}

double*
eh_uniform_array( double x1 , double x2 , double dx , gint* n )
{
   double* x = NULL;

   eh_require( n     );
   eh_require( x2>x1 );
   eh_require( dx>0  );

   *n = (gint) ((x2-x1)/dx);

   if ( *n>0 )
   {
      gssize i, len=*n;

      x = eh_new( double , len );
      for ( i=1,x[0]=x1 ; i<len ; i++ )
         x[i] = x[i-1] + dx;

   }
   else
      *n = 0;

   return x;
}

double*
eh_dbl_array_linspace( double* x , gssize n_x ,  double x_0 , double dx )
{
   eh_require( x      );
   eh_require( n_x>=0 );

   eh_return_val_if_fail( x && n_x>=0 , NULL )
   {
      gssize i;
      for ( i=0 ; i<n_x ; i++ )
         x[i] = i*dx + x_0;
      return x;
   }
}

/** Check if an array is monotonic (either increasing of decreasing)

\param x A pointer to an array of doubles
\param len The length of the array

\return TRUE if the array is either monotonically increasing or decreasing.
*/
gboolean
eh_dbl_array_is_monotonic ( double* x , gsize len )
{
   gssize i;

   eh_require( x );
   eh_require( len>0 );

   if ( len==1 )
      return TRUE;
   else if ( x[1]>x[0] )
   {
      for ( i=2 ; i<len ; i++ )
         if ( x[i]<=x[i-1]  )
            return FALSE;
   }
   else if ( x[1]<x[0] )
   {
      for ( i=2 ; i<len ; i++ )
         if ( x[i]<=x[i-1]  )
            return FALSE;
   }
   else
      return FALSE;

   return TRUE;
}

/** Generate an array of equally spaced numbers.

Generate a row vector of n logarithmically equally spaced points between
decades 10^d1 and 10^d2.  returns a pointer to a newly allocated array.
should be freed using eh_free.

\param d1 Lower bound
\param d2 Lower bound
\param n  Number of points

\return An array of equally spaced points

\see eh_linspace
*/
double *eh_logspace( double d1 , double d2 , int n )
{
   int i;
   double *x = eh_linspace( d1 , d2 , n );
   for ( i=0 ; i<n ; i++ )
      x[i] = pow( 10. , x[i] );
   return x;
}

void lubksb( double** a , gssize n , gssize* indx , double* b )
{
   gssize i,ii=0,ip,j;
   double sum;

   for (i=1;i<=n;i++)
   {
      ip=indx[i];
      sum=b[ip];
      b[ip]=b[i];
      if (ii)
         for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
      else if (sum) ii=i;
      b[i]=sum;
   }
   for (i=n;i>=1;i--) {
      sum=b[i];
      for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
      b[i]=sum/a[i][i];
   }
}

#define TINY 1.0e-20;

void ludcmp( double** a , gssize n , gssize* indx , double* d )
{
   gssize i,imax,j,k;
   double big,dum,sum,temp;
   double *vv;

   vv = eh_new( double , n ) - 1;
   *d=1.0;
   for (i=1;i<=n;i++) {
      big=0.0;
      for (j=1;j<=n;j++)
         if ((temp=fabs(a[i][j])) > big) big=temp;
      if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
      vv[i]=1.0/big;
   }
   for (j=1;j<=n;j++) {
      for (i=1;i<j;i++) {
         sum=a[i][j];
         for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
      }
      big=0.0;
      for (i=j;i<=n;i++) {
         sum=a[i][j];
         for (k=1;k<j;k++)
            sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
         if ( (dum=vv[i]*fabs(sum)) >= big) {
            big=dum;
            imax=i;
         }
      }
      if (j != imax) {
         for (k=1;k<=n;k++) {
            dum=a[imax][k];
            a[imax][k]=a[j][k];
            a[j][k]=dum;
         }
         *d = -(*d);
         vv[imax]=vv[j];
      }
      indx[j]=imax;
      if (a[j][j] == 0.0) a[j][j]=TINY;
      if (j != n) {
         dum=1.0/(a[j][j]);
         for (i=j+1;i<=n;i++) a[i][j] *= dum;
      }
   }
   vv += 1;
   eh_free( vv );
}

#undef TINY

void savgol(double* c, gssize np , gssize nl , gssize nr , gssize ld , gssize m )
{
   //void lubksb(),ludcmp();
   gssize imj,ipj,j,k,kk,mm,*indx;
   double d,fac,sum,**a,*b;

   eh_require( np >= nl+nr+1 );
   eh_require( nl >= 0       );
   eh_require( nr >= 0       );
   eh_require( ld <= m       );
   eh_require( nl+nr >= m    );

   indx = eh_new( gssize , m+1 ) - 1;
   a    = eh_new_2( double , m+1 , m+1 );
   {
      gssize i;
      for ( i=0 ; i<m+1 ; i++ )
         a[i] -= 1;
      a -= 1;
   }
   b    = eh_new( double , m+1 ) - 1;
/*
   indx=ivector(1,m+1);
   a=matrix(1,m+1,1,m+1);
   b=vector(1,m+1);
*/

   for (ipj=0;ipj<=(m << 1);ipj++) {
      sum=(ipj ? 0.0 : 1.0);
      for (k=1;k<=nr;k++) sum += pow((double)k,(double)ipj);
      for (k=1;k<=nl;k++) sum += pow((double)-k,(double)ipj);
//      mm=FMIN(ipj,2*m-ipj);
      mm=eh_min(ipj,2*m-ipj);
      for (imj = -mm;imj<=mm;imj+=2) a[1+(ipj+imj)/2][1+(ipj-imj)/2]=sum;
   }
   ludcmp(a,m+1,indx,&d);
   for (j=1;j<=m+1;j++) b[j]=0.0;
   b[ld+1]=1.0;
   lubksb(a,m+1,indx,b);
   for (kk=1;kk<=np;kk++) c[kk]=0.0;
   for (k = -nl;k<=nr;k++) {
      sum=b[1];
      fac=1.0;
      for (mm=1;mm<=m;mm++) sum += b[mm+1]*(fac *= k);
      kk=((np-k) % np)+1;
      c[kk]=sum;
   }
   b    += 1;
   a    += 1;
   a[0] += 1;
   indx += 1;
   eh_free(b);
   eh_free( a );
   eh_free(indx);
}

double *vector( long l , long h );
void free_vector( double *x , long l , long h );

void convlv(double data[], unsigned long n, double respns[], unsigned long m,
	int isign, double ans[])
{
	void realft(double data[], unsigned long n, int isign);
	void twofft(double data1[], double data2[], double fft1[], double fft2[],
		unsigned long n);
	unsigned long i,no2;
	double dum,mag2,*fft;

//	fft=vector(1,n<<1);
	fft = eh_new( double , n<<1 ) - 1;
	for (i=1;i<=(m-1)/2;i++)
		respns[n+1-i]=respns[m+1-i];
	for (i=(m+3)/2;i<=n-(m-1)/2;i++)
		respns[i]=0.0;
	twofft(data,respns,fft,ans,n);
	no2=n>>1;
	for (i=2;i<=n+2;i+=2) {
		if (isign == 1) {
			ans[i-1]=(fft[i-1]*(dum=ans[i-1])-fft[i]*ans[i])/no2;
			ans[i]=(fft[i]*dum+fft[i-1]*ans[i])/no2;
		} else if (isign == -1) {
			if ((mag2=pow(ans[i-1],2.)+pow(ans[i],2.)) == 0.0)
				nrerror("Deconvolving at response zero in convlv");
			ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/mag2/no2;
			ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/mag2/no2;
		} else nrerror("No meaning for isign in convlv");
	}
	ans[2]=ans[n+1];
	realft(ans,n,-1);
//	free_vector(fft,1,n<<1);
   fft += 1;
   eh_free( fft );
}

double *vector( long l , long h )
{
   double *x;
   x = eh_new( double , h-l+1 );
   return x-l;
}

void free_vector( double *x , long l , long h )
{
   x += l;
   eh_free( x );
}

void four1(double data[], unsigned long nn, int isign)
{
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	double tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			swap_dbl(data[j],data[i]);
			swap_dbl(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=isign*(6.28318530717959/mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}

void realft(double data[], unsigned long n, int isign)
{
	void four1(double data[], unsigned long nn, int isign);
	unsigned long i,i1,i2,i3,i4,np3;
	double c1=0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta;

	theta=3.141592653589793/(double) (n>>1);
	if (isign == 1) {
		c2 = -0.5;
		four1(data,n>>1,1);
	} else {
		c2=0.5;
		theta = -theta;
	}
	wtemp=sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi=sin(theta);
	wr=1.0+wpr;
	wi=wpi;
	np3=n+3;
	for (i=2;i<=(n>>2);i++) {
		i4=1+(i3=np3-(i2=1+(i1=i+i-1)));
		h1r=c1*(data[i1]+data[i3]);
		h1i=c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i=c2*(data[i1]-data[i3]);
		data[i1]=h1r+wr*h2r-wi*h2i;
		data[i2]=h1i+wr*h2i+wi*h2r;
		data[i3]=h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wr=(wtemp=wr)*wpr-wi*wpi+wr;
		wi=wi*wpr+wtemp*wpi+wi;
	}
	if (isign == 1) {
		data[1] = (h1r=data[1])+data[2];
		data[2] = h1r-data[2];
	} else {
		data[1]=c1*((h1r=data[1])+data[2]);
		data[2]=c1*(h1r-data[2]);
		four1(data,n>>1,-1);
	}
}

void twofft(double data1[], double data2[], double fft1[], double fft2[],
	unsigned long n)
{
	void four1(double data[], unsigned long nn, int isign);
	unsigned long nn3,nn2,jj,j;
	double rep,rem,aip,aim;

	nn3=1+(nn2=2+n+n);
	for (j=1,jj=2;j<=n;j++,jj+=2) {
		fft1[jj-1]=data1[j];
		fft1[jj]=data2[j];
	}
	four1(fft1,n,1);
	fft2[1]=fft1[2];
	fft1[2]=fft2[2]=0.0;
	for (j=3;j<=n+1;j+=2) {
		rep=0.5*(fft1[j]+fft1[nn2-j]);
		rem=0.5*(fft1[j]-fft1[nn2-j]);
		aip=0.5*(fft1[j+1]+fft1[nn3-j]);
		aim=0.5*(fft1[j+1]-fft1[nn3-j]);
		fft1[j]=rep;
		fft1[j+1]=aim;
		fft1[nn2-j]=rep;
		fft1[nn3-j] = -aim;
		fft2[j]=aip;
		fft2[j+1] = -rem;
		fft2[nn2-j]=aip;
		fft2[nn3-j]=rem;
	}
}

/* Compute a weighted average of the data in vector x, given the weights in vector f.
   Vectors are of length len.
*/
double
eh_dbl_array_mean_weighted( const double x[] , gint len , const double f[] )
{
   gint   i   = 0;
   double sum = 0;
   
   for ( ; i<len ; i++ )
      sum += f[i]*x[i];
   
   return sum;
}

#ifndef M_LN2
# define M_LN2 0.69314718055994530942
#endif

#ifndef HAVE_LOG2
double
log2(double x)
{
   return log(x)/M_LN2;
}
#endif

#undef M_LN2

#if defined( OLD_EH_NAN )

float eh_nan(void)
{
   gint32 a=0x7FF00000L;
   return *((float*)(&a));
}

int eh_isnan(float x)
{
   return (    ((*(gint32*)&(x) & 0x7F800000L) == 0x7F800000L)
            && ((*(gint32*)&(x) & 0x007FFFFFL) != 0x00000000L) );
}
#else

double eh_nan( void )
{
//   return sqrt(-1);
   return g_strtod( "NAN" , NULL );
}

gboolean eh_isnan( double x )
{
   return isnan(x);
}
#endif

void eh_rebin_dbl_array( double *x     , double *y     , gssize len ,
                         double *x_bin , double *y_bin , gssize len_bin )
{
   eh_rebin_dbl_array_bad_val( x     , y     , len     ,
                               x_bin , y_bin , len_bin ,
                               eh_nan() );
}

void
eh_rebin_dbl_array_bad_val( double *x     , double *y     , gssize len     ,
                            double *x_bin , double *y_bin , gssize len_bin ,
                            double bad_val )
{
   eh_require( x );
   eh_require( y );
   eh_require( x_bin );
   eh_require( y_bin );
   //eh_require( len>2 );
   //eh_require( len_bin>2 );

   if ( x && y && x_bin && y_bin )
   {
      gint i, j;
      gint top_i, top_j, lower_j, upper_j;
      double left_bin, right_bin, lower, upper, upper_bin, lower_bin;
      double sum;
      double *x_edge;

      eh_require( eh_dbl_array_is_monotonic_up( x     , len     ) );
      eh_require( eh_dbl_array_is_monotonic_up( x_bin , len_bin ) );

      // initialize y_bin with NaN's.
      eh_dbl_array_set( y_bin , len_bin , bad_val );
      //for ( i=0 ; i<len_bin ; y_bin[i]=bad_val , i++ );

      top_i     = len_bin-1;
      top_j     = len-1;
      lower_bin = x_bin[0]     - ( x_bin[1]     - x_bin[0]       ) * .5;
      upper_bin = x_bin[top_i] + ( x_bin[top_i] - x_bin[top_i-1] ) * .5;
      lower     = x[0]         - ( x[1]         - x[0]           ) * .5;
      upper     = x[top_j]     + ( x[top_j]     - x[top_j-1]     ) * .5;

      { /* Define the edges of the data */
         x_edge      = eh_new( double , len+1 );
         x_edge[0]   = x[0] - ( x[1] - x[0] ) * .5;
         x_edge[len] = x[top_j] + ( x[top_j] - x[top_j-1] ) * .5;
         for ( j=1 ; j<len ; j++ )
            x_edge[j] = ( x[j-1] + x[j] ) * .5;
      }

      upper_j = 0;
      for ( i=0 ; i<len_bin && j<=len ; i++ )
      {
         // Integrate the data in y over the entire bin.
         left_bin  = (i!=0)    ?( x_bin[i]   + x_bin[i-1] ) * .5 : lower_bin;
         right_bin = (i!=top_i)?( x_bin[i+1] + x_bin[i]   ) * .5 : upper_bin;

         // Find the lower j.
         for ( j=upper_j ; j<=len && x_edge[j]<=left_bin ; j++ );
         lower_j = j-1;
         eh_clamp( lower_j , 0 , len );

         // Find the upper j.
         for ( j=lower_j ; j<=len && x_edge[j]<=right_bin ; j++ );
         upper_j = j;
         eh_clamp( upper_j , 0 , len );

         // Integrate the data over these j.
//      sum  = y[lower_j]*(left_bin-x_edge[lower_j])/(right_bin-left_bin);
//      sum += y[upper_j]*(right_bin-x_edge[upper_j])/(right_bin-left_bin);

         eh_require_critical( upper_j<=len );
         eh_require_critical( lower_j<=len );
         eh_require_critical( upper_j>=0   );
         eh_require_critical( lower_j>=0   );
         sum  = 0;
         if ( left_bin > x_edge[upper_j] || right_bin < x_edge[lower_j] )
            sum = bad_val;
         else if ( upper_j-lower_j>1 )
         {
            eh_require_critical( lower_j<len );

            if ( left_bin >= x_edge[lower_j] )
               sum += y[lower_j]
                    * (x_edge[lower_j+1]-left_bin);
            else
               sum += y[lower_j]
                    * (x_edge[lower_j+1]-x_edge[lower_j]);

            eh_require_critical( upper_j>0 )

            if ( right_bin <= x_edge[upper_j] )
               sum += y[upper_j-1] 
                    * (right_bin-x_edge[upper_j-1]);
            else
               sum += y[upper_j-1] 
                    * (x_edge[upper_j]-x_edge[upper_j-1]);

            for ( j=lower_j+1 ; j<upper_j-1 ; j++ )
            {
eh_require_critical( j>=0  );
eh_require_critical( j<len );
               sum += y[j]*(x_edge[j+1]-x_edge[j]);
            }
            sum /= (right_bin-left_bin);
         }
         else
         {
eh_require_critical( lower_j>=0   );
eh_require_critical( upper_j<=len );
            for ( j=lower_j ; j<upper_j ; j++ )
               sum += y[j];
         }

         y_bin[i] = sum;
      }

      eh_free( x_edge );
   }

   return;
}

#include <math.h>
//#include "utils.h"

double*
eh_dbl_array_diffuse_implicit( double* x , gint len , double c )
{
   if ( x )
   {
      double* l = eh_new( double , len );
      double* d = eh_new( double , len );
      double* u = eh_new( double , len );
      double* b = eh_dbl_array_dup( x , len );

      eh_dbl_array_set( l , len , -c      );
      eh_dbl_array_set( d , len , 1.+2.*c );
      eh_dbl_array_set( u , len , -c      );
      eh_dbl_array_set( x , len , 0.      );

      l[len-1] = -c;
      d[len-1] = 1+c;
      u[0]     = -c;
      d[0]     = 1+c;

      tridiag( l , d , u , b , x , len );

      eh_free( l );
      eh_free( d );
      eh_free( u );
      eh_free( b );
   }
   return x;
}

double*
eh_dbl_array_diffuse_explicit( double* x , gint len , double c )
{
   if ( x )
   {
      gint         i;
      double*      x_new  = eh_dbl_array_dup( x , len );
      const gint   top_i  = len-1;

      for ( i=1 ; i<top_i ; i++ )
         x_new[i] += c * ( x[i-1] - 2.*x[i] + x[i+1] );

      x_new[0]     += c * ( -x[0]     + x[1]       );
      x_new[top_i] += c * ( -x[top_i] + x[top_i-1] );

      eh_dbl_array_copy( x , x_new , len );

      eh_free( x_new );
   }

   return x;
}

double*
eh_dbl_array_diffuse( double* x , gint len , double c , Eh_num_method method )
{
   switch ( method )
   {
      case EH_NUM_IMPLICIT: eh_dbl_array_diffuse_implicit( x , len , c ); break;
      case EH_NUM_EXPLICIT: eh_dbl_array_diffuse_explicit( x , len , c ); break;
      default: eh_require_not_reached();
   }
   return x;
}

