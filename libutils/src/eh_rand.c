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

#include "eh_rand.h"
#include "utils.h"

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

double eh_ran0(long *idum)
{
	long k;
	double ans;

	*idum ^= MASK;
	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	if (*idum < 0) *idum += IM;
	ans=AM*(*idum);
	*idum ^= MASK;
	return ans;
}
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef MASK

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

double eh_ran1(long *idum)
{
	int j;
	long k;
	static long iy=0;
	static long iv[NTAB];
	double temp;

	if (*idum <= 0 || !iy) {
		if (-(*idum) < 1) *idum=1;
		else *idum = -(*idum);
		for (j=NTAB+7;j>=0;j--) {
			k=(*idum)/IQ;
			*idum=IA*(*idum-k*IQ)-IR*k;
			if (*idum < 0) *idum += IM;
			if (j < NTAB) iv[j] = *idum;
		}
		iy=iv[0];
	}
	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	if (*idum < 0) *idum += IM;
	j=iy/NDIV;
	iy=iv[j];
	iv[j] = *idum;
	if ((temp=AM*iy) > RNMX) return RNMX;
	else return temp;
}
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

double eh_ran2(long *idum)
{
	int j;
	long k;
	static long idum2=123456789;
	static long iy=0;
	static long iv[NTAB];
	double temp;

	if (*idum <= 0) {
		if (-(*idum) < 1) *idum=1;
		else *idum = -(*idum);
		idum2=(*idum);
		for (j=NTAB+7;j>=0;j--) {
			k=(*idum)/IQ1;
			*idum=IA1*(*idum-k*IQ1)-k*IR1;
			if (*idum < 0) *idum += IM1;
			if (j < NTAB) iv[j] = *idum;
		}
		iy=iv[0];
	}
	k=(*idum)/IQ1;
	*idum=IA1*(*idum-k*IQ1)-k*IR1;
	if (*idum < 0) *idum += IM1;
	k=idum2/IQ2;
	idum2=IA2*(idum2-k*IQ2)-k*IR2;
	if (idum2 < 0) idum2 += IM2;
	j=iy/NDIV;
	iy=iv[j]-idum2;
	iv[j] = *idum;
	if (iy < 1) iy += IMM1;
	if ((temp=AM*iy) > RNMX) return RNMX;
	else return temp;
}
#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX

#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

double eh_ran3(long *idum)
{
	static int inext,inextp;
	static long ma[56];
	static int iff=0;
	long mj,mk;
	int i,ii,k;

	if (*idum < 0 || iff == 0) {
		iff=1;
		mj=MSEED-(*idum < 0 ? -*idum : *idum);
		mj %= MBIG;
		ma[55]=mj;
		mk=1;
		for (i=1;i<=54;i++) {
			ii=(21*i) % 55;
			ma[ii]=mk;
			mk=mj-mk;
			if (mk < MZ) mk += MBIG;
			mj=ma[ii];
		}
		for (k=1;k<=4;k++)
			for (i=1;i<=55;i++) {
				ma[i] -= ma[1+(i+30) % 55];
				if (ma[i] < MZ) ma[i] += MBIG;
			}
		inext=0;
		inextp=31;
		*idum=1;
	}
	if (++inext == 56) inext=1;
	if (++inextp == 56) inextp=1;
	mj=ma[inext]-ma[inextp];
	if (mj < MZ) mj += MBIG;
	ma[inext]=mj;
	return mj*FAC;
}
#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

double eh_ran4(long *idum)
{
	void eh_psdes(unsigned long *lword, unsigned long *irword);
	unsigned long irword,itemp,lword;
	static long idums = 0;
#if defined(vax) || defined(_vax_) || defined(__vax__) || defined(VAX)
	static unsigned long jflone = 0x00004080;
	static unsigned long jflmsk = 0xffff007f;
#else
	static unsigned long jflone = 0x3f800000;
	static unsigned long jflmsk = 0x007fffff;
#endif

	if (*idum < 0) {
		idums = -(*idum);
		*idum=1;
	}
	irword=(*idum);
	lword=idums;
	eh_psdes(&lword,&irword);
	itemp=jflone | (jflmsk & irword);
	++(*idum);
	return (*(double *)&itemp)-1.0;
}

#define NITER 4

void eh_psdes(unsigned long *lword, unsigned long *irword)
{
	unsigned long i,ia,ib,iswap,itmph=0,itmpl=0;
	static unsigned long c1[NITER]={
		0xbaa96887L, 0x1e17d32cL, 0x03bcdc3cL, 0x0f33d1b2L};
	static unsigned long c2[NITER]={
		0x4b0f3b58L, 0xe874f0c3L, 0x6955c5a6L, 0x55a7ca46L};

	for (i=0;i<NITER;i++) {
		ia=(iswap=(*irword)) ^ c1[i];
		itmpl = ia & 0xffff;
		itmph = ia >> 16;
		ib=itmpl*itmpl+ ~(itmph*itmph);
		*irword=(*lword) ^ (((ia = (ib >> 16) |
			((ib & 0xffff) << 16)) ^ c2[i])+itmpl*itmph);
		*lword=iswap;
	}
}
#undef NITER

#include <math.h>

#define PI 3.14159265359

// Return a random number between 0 and 1 using the pdf,
//   p(x) = a*cos(x*a) ( a = pi/2 )
double eh_cosdev(long *idum)
{
   double n;
   
   n=(double)eh_ran1(idum);
   
   return asin(n)*2/PI;
}

#undef PI


double eh_reject(double (*p)(double),double (*f)(double),double (*F)(double))
{
   double eh_ran1(long *idum);
   double x, n;
   static long seed[1];
   
   x = (*F)((double)eh_ran1(seed));
   n = (double)eh_ran1(seed)*(*f)(x);
   
   if ( n > (*p)(x) )
      x = eh_reject(p,f,F);
   
   return x;

}

#include <math.h>

double eh_expdev(long *idum)
{
   double eh_ran1(long *idum);
   double dum;
   
   do
      dum=eh_ran1(idum);
   while (dum == 0.0 );
   
   return -log(dum);
}

#include <math.h>

double eh_gasdev(long *idum)
{
   double eh_ran1(long *idum);
   static int iset=0;
   static double gset;
   double fac,rsq, v1, v2;
   
   if ( iset == 0 )
   {
      do
      {
         v1 = 2.0*eh_ran1(idum)-1.0;
         v2 = 2.0*eh_ran1(idum)-1.0;
         rsq = v1*v1 +v2*v2;
      } while ( rsq >= 1.0 || rsq == 0.0 );
      fac = sqrt(-2.0*log(rsq)/rsq);
      gset=v1*fac;
      iset=1;
      return v2*fac;
   }
   else
   {
      iset = 0;
      return gset;
   }
}

#include <math.h>

// Picks a random number with a pdf,
//        f(x) = a^x     ( |a| <= 1 )
//
double eh_powdev(double a,long *idum)
{
   double eh_ran2(long *idum);
   double dum;
   
   do
      dum=eh_ran2(idum);
   while (dum == 1.0 );
   
   return log(1-dum)/log(a);
}

double eh_maxpowdev(double a,double n,long *idum)
{
   double eh_ran2(long *idum);
   double dum;
   
   do
      dum=eh_ran2(idum);
   while (dum == 1.0 );
   
   return log(1-pow(dum,1./n))/log(a);
}

/** \defgroup rand_group Random number distributions
@{
*/

/** Random number from an exponential distribution.

Pick a random number from an exponential distribution with mean, \a mu.

\f[
   f(x) = {1\over \mu}e^{-{x\over\mu}}
\f]

\param rand  A GRand
\param mu    The scale parameter of the distribution

\return A random number
*/
double eh_rand_exponential( GRand *rand , double mu )
{
   double dum;

   do
   {
      dum = g_rand_double( rand );
   }
   while ( dum == 0. );

   return - mu * log( dum );
}

/** Maximum of a series of exponential random numbers

The maximum of a series of numbers drawn from an exponential distribution

\param rand A GRand
\param mu   Mean of the distribution
\param n    The number of numbers picked

\return A random number
*/
double eh_rand_max_exponential( GRand *rand , double mu , double n )
{
   double dum;

   do
   {
      dum = g_rand_double( rand );
   }
   while ( pow(dum,1./n) == 1. );


   return - mu * log( 1 - pow(dum,1./n) );
}

/** Random number from a log-normal distribution.

Pick a random number from a log-normal distribution with mean, \a mu and standard deviation,
\a sigma.

\f[
   f(x) = {1 \over \sigma \sqrt{2 \pi} } e^{ \left(\log\left( x-\mu \right)\right)^2 \over 2 \sigma^2 }
\f]

\param rand  A GRand
\param mu    The scale parameter of the distribution
\param sigma The shape parameter of the distribution

\return A random number
*/
double eh_log_normal( GRand* rand , double mu , double sigma )
{
   return exp( eh_rand_normal(rand,mu,sigma) );
}

/** Maximum of a series of log-normal random numbers

The maximum of a series of numbers drawn from a log-normal distribution

\param rand  A GRand
\param mu    The scale parameter
\param sigma The shape parameter
\param n     The number of numbers picked

\return A random number
*/
double eh_max_log_normal( GRand *rand , double mu , double sigma , double n )
{
   double eh_ran2( long *idum);
   double dum;

   do
   {
//      dum=eh_ran2(idum);
      dum = g_rand_double( rand );
//      dum = ((double)random())/((double)G_MAXINT);
   }
   while ( fabs(2*(pow(dum,1./n)-.5)) >= 1.-1e-12 );

   return exp( eh_erf_inv( 2.*(pow(dum,1./n)-.5) )*sqrt(2)*sigma + mu );
}

/** Random number from a Weibull distribution

Pick a random number from a Weibull distribution with shape parameter, \a beta and
scale parameter \a eta.

\f[
   f(x) = {\beta \over \eta } \left( {x \over \eta} \right)^{\beta-1}
          e^{-\left( {x \over \eta} \right)^\beta }
\f]

\param rand A GRand
\param eta  Scale parameter
\param beta Shape parameter

\return A random number
*/
double eh_rand_weibull( GRand *rand , double eta , double beta )
{
   double dum;

   do
   {
      dum = g_rand_double( rand );
   }
   while ( dum == 0 );

   return eta*pow( -log(dum) , 1./beta );
}

/** Maximum of a series of Weibull random numbers

The maximum of a series of numbers drawn from a Weibull distribution.

\param rand A GRand
\param beta Shape parameter
\param eta  Scale parameter
\param n    The number of numbers picked

\return A random number
*/
double eh_rand_max_weibull( GRand *rand , double eta , double beta , double n )
{
   double dum;
   double z;
   double ans;

   do
   {
      dum = g_rand_double( rand );
      z = pow( dum , 1./n );
   }
   while ( dum==1. );
//   while ( fabs(z) >= 1. );

   if ( n<1. )
      ans = eta*pow( z*( 1 + .5*z*( 1 + .6666667*z ) ) , 1./beta );
//      return b*pow( -( z*( 1. + z*( -.5 + z*(.33333  - .25*z))) ) , 1./a );
   else
      ans = eta*pow( -log( 1 - pow(dum,1./n) ) , 1./beta );

   return ans;
}

/** Random number from a user defined distribution

Pick a random number from a user defined distribution function.

\param rand A GRand
\param x    x-values of the user-defined CDF
\param F    F-values of the user-defined CDF
\param len  Length of \a x and \a y

\return A random number
*/
double eh_rand_user( GRand *rand , double *x , double *F , gssize len )
{
   double ans, u = g_rand_double( rand );
   interpolate( F , x , len ,  &u , &ans , 1 );
   return ans;
}

/** Random number from a normal distribution

Pick a random number from a normal distribution with mean \a mu, and standard deviation,
\a sigma.

\f[
   f(x) = {1 \over \sigma \sqrt{2 \pi} } e^{ \left( x-\mu \right)^2 \over 2 \sigma^2 }
\f]

\param rand  A GRand
\param mu    Mean of normal distribution
\param sigma Standard deviation of normal distribution

\return A random number
*/
double eh_rand_normal( GRand* rand , double mu , double sigma )
{
   static gboolean iset=FALSE;
   static double gset;
   double fac,rsq, v1, v2;

   eh_require( sigma>0 );
   
   if ( iset == 0 )
   {
      do
      {
         if ( rand )
         {
            v1 = 2.0*g_rand_double( rand )-1.;
            v2 = 2.0*g_rand_double( rand )-1.;
         }
         else
         {
            v1 = 2.0*g_random_double( )-1.;
            v2 = 2.0*g_random_double( )-1.;
         }
         rsq = v1*v1 +v2*v2;
      } while ( rsq >= 1.0 || rsq == 0.0 );
      fac = sqrt(-2.0*log(rsq)/rsq);
      gset=v1*fac;
      iset=TRUE;
      return v2*fac*sigma+mu;
   }
   else
   {
      iset = FALSE;
      return gset*sigma+mu;
   }
}

double eh_get_fuzzy_dbl( double min , double max )
{
   return g_random_double_range( min , max );
}

double eh_get_fuzzy_dbl_norm( double mean , double std )
{
   return eh_rand_normal( NULL , mean , std );
}

double eh_get_fuzzy_dbl_log_norm( double mean , double std )
{
   return eh_log_normal( NULL , mean , std );
}

gint32 eh_get_fuzzy_int( gint32 min , gint32 max )
{
   return g_random_int_range( min , max );
}

/* @} */
