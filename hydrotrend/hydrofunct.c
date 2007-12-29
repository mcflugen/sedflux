/*
 *  HydroFunct.c
 *
 *  Various functions used by HydroTrend
 *  From Kerningham and Ritchie
 *  and Numerical Recipes in C
 *
 *  Author:     M.D. Morehead   (1998)
 *
 */

#include <string.h>

/*--------------------------------
 *  Convert n to characters in s
 *  From: Kerningham and Ritchie
 *--------------------------------*/
int itoa(int n,char s[]){
   int c, i, j, sign;

   if((sign = n) < 0 )		/* record sign */
      n = -n;			/* make n positive */
   i = 0;
   do {				/* generate digits in reverse order */
      s[i++] = n % 10 + '0';	/* get next digit */
      } while ((n /= 10) > 0);	/* delete it */
   if ( sign < 0 )
      s[i++] = '-';
   s[i] = '\0';
				/* reverse the string */
   for( i=0, j=strlen(s)-1; i<j; i++, j--) {
      c    = s[i];
      s[i] = s[j];
      s[j] = c;
   }
   return 0;
}

/*----------------------------------------------------------------
 *  The following was take from nrutil.c, I only wanted a subset
 *  and I wanted to make a standalone version so it was easier
 *  to use on various platforms
 *----------------------------------------------------------------*/

/* CAUTION: This is the ANSI C (only) version of the Numerical Recipes
   utility file nrutil.c.  Do not confuse this file with the same-named
   file nrutil.c that is supplied in the same subdirectory or archive
   as the header file nrutil.h.  *That* file contains both ANSI and
   traditional K&R versions, along with #ifdef macros to select the
   correct version.  *This* file contains only ANSI C.               */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#define NR_END 1
#define FREE_ARG char*

/*--------------------------------------------
 *  Numerical Recipes standard error handler
 *--------------------------------------------*/
void nrerror(char error_text[]){
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

/*----------------------------------------------------------------------
 *  allocate a float matrix with subscript range m[nrl..nrh][ncl..nch]
 *----------------------------------------------------------------------*/
float **matrix(long nrl, long nrh, long ncl, long nch){
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

/*-----------------------------------------------------------------------
 *  allocate a double matrix with subscript range m[nrl..nrh][ncl..nch]
 *-----------------------------------------------------------------------*/
double **dmatrix(long nrl, long nrh, long ncl, long nch){
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

/*-----------------------------------------------------------------------
 *  allocate a float 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh]
 *-----------------------------------------------------------------------*/
float ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh){
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
	float ***t;

	/* allocate pointers to pointers to rows */
	t=(float ***) malloc((size_t)((nrow+NR_END)*sizeof(float**)));
	if (!t) nrerror("allocation failure 1 in f3tensor()");
	t += NR_END;
	t -= nrl;

	/* allocate pointers to rows and set pointers to them */
	t[nrl]=(float **) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float*)));
	if (!t[nrl]) nrerror("allocation failure 2 in f3tensor()");
	t[nrl] += NR_END;
	t[nrl] -= ncl;

	/* allocate rows and set pointers to them */
	t[nrl][ncl]=(float *) malloc((size_t)((nrow*ncol*ndep+NR_END)*sizeof(float)));
	if (!t[nrl][ncl]) nrerror("allocation failure 3 in f3tensor()");
	t[nrl][ncl] += NR_END;
	t[nrl][ncl] -= ndl;

	for(j=ncl+1;j<=nch;j++) t[nrl][j]=t[nrl][j-1]+ndep;
	for(i=nrl+1;i<=nrh;i++) {
		t[i]=t[i-1]+ncol;
		t[i][ncl]=t[i-1][ncl]+ncol*ndep;
		for(j=ncl+1;j<=nch;j++) t[i][j]=t[i][j-1]+ndep;
	}

	/* return pointer to array of pointers to rows */
	return t;
}

/*--------------------------------------------------------------------------
 *  allocate a double 3D tensor with range t[nrl..nrh][ncl..nch][ndl..ndh]
 *--------------------------------------------------------------------------*/
double ***d3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh){
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
	double ***t;

	/* allocate pointers to pointers to rows */
	t=(double ***) malloc((size_t)((nrow+NR_END)*sizeof(double**)));
	if (!t) nrerror("allocation failure 1 in d3tensor()");
	t += NR_END;
	t -= nrl;

	/* allocate pointers to rows and set pointers to them */
	t[nrl]=(double **) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double*)));
	if (!t[nrl]) nrerror("allocation failure 2 in d3tensor()");
	t[nrl] += NR_END;
	t[nrl] -= ncl;

	/* allocate rows and set pointers to them */
	t[nrl][ncl]=(double *) malloc((size_t)((nrow*ncol*ndep+NR_END)*sizeof(double)));
	if (!t[nrl][ncl]) nrerror("allocation failure 3 in d3tensor()");
	t[nrl][ncl] += NR_END;
	t[nrl][ncl] -= ndl;

	for(j=ncl+1;j<=nch;j++) t[nrl][j]=t[nrl][j-1]+ndep;
	for(i=nrl+1;i<=nrh;i++) {
		t[i]=t[i-1]+ncol;
		t[i][ncl]=t[i-1][ncl]+ncol*ndep;
		for(j=ncl+1;j<=nch;j++) t[i][j]=t[i][j-1]+ndep;
	}

	/* return pointer to array of pointers to rows */
	return t;
}

/*---------------------------------------------
 *  free a float matrix allocated by matrix()
 *---------------------------------------------*/
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch){
   free((FREE_ARG) (m[nrl]+ncl-NR_END));
   free((FREE_ARG) (m+nrl-NR_END));
}

/*-----------------------------------------------
 *  free a double matrix allocated by dmatrix()
 *-----------------------------------------------*/
void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch){
   free((FREE_ARG) (m[nrl]+ncl-NR_END));
   free((FREE_ARG) (m+nrl-NR_END));
}

/*-------------------------------------------------
 *  free a float f3tensor allocated by f3tensor()
 *-------------------------------------------------*/
void free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh){
   free((FREE_ARG) (t[nrl][ncl]+ndl-NR_END));
   free((FREE_ARG) (t[nrl]+ncl-NR_END));
   free((FREE_ARG) (t+nrl-NR_END));
}

/*-------------------------------------------------
 *  free a float d3tensor allocated by d3tensor()
 *-------------------------------------------------*/
void free_d3tensor(double ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh){
   free((FREE_ARG) (t[nrl][ncl]+ndl-NR_END));
   free((FREE_ARG) (t[nrl]+ncl-NR_END));
   free((FREE_ARG) (t+nrl-NR_END));
}
#include "hydrodaysmonths.h"

int days_in_month( Month m )
{
   static int daysim[12]  = { 31 , 28 , 31 , 30 ,
                              31 , 30 , 31 , 31 ,
                              30 , 31 , 30 , 31 };
   return daysim[m];
}

int start_of( Month m )
{
   static int daystrm[12] = {   1 ,  32 ,  60 ,  91 ,
                              121 , 152 , 182 , 213 ,
                              244 , 274 , 305 , 335 };
   return daystrm[m];
}

int end_of( Month m )
{
   static int dayendm[12] = {  31 ,  59 ,  90 , 120 ,
                              151 , 181 , 212 , 243 ,
                              273 , 304 , 334 , 365 };
   return dayendm[m];
}

