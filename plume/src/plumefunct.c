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

/*
 *	PLUMEFUNCT.C		Various functions used by Plume2
 */

#include <string.h>
#include <glib.h>
#include <utils.h>

/*
 *	convert n to characters in s
 *	From: Kerningham and Ritchie
 */
void itoa(int n,char s[])
{
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
}

/*
 *	The following was take from nrutil.c, I only wanted a subset
 *	and I wanted to make a standalone version so it was easier
 *	to use on various platforms
 */
 
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

#ifdef NOTDEFINED

void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	eh_exit(1);
}

#endif

float **matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
//	m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
        m = (float**)eh_malloc( ((size_t)(nrow+NR_END)*sizeof(float*)) ,
                                NULL , __FILE__ , __LINE__ );
	if (!m) eh_error("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
//	m[nrl] = (float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
        m[nrl] = (float*)eh_malloc( ((size_t)(nrow*ncol+NR_END)*sizeof(float)) ,
                                    NULL , __FILE__ , __LINE__ );
	if (!m[nrl]) eh_error("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

double **new_dmatrix( long n_rows , long n_cols )
{
   int i;
   double **m;

   m = eh_new( double* , n_rows );

   m[0] = eh_new( double , n_rows*n_cols );

   for ( i=1 ; i<n_rows ; i++ )
      m[i] = m[i-1]+n_cols;

   return m;
}

float ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
/* allocate a float 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh] */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
	float ***t;

	/* allocate pointers to pointers to rows */
//	t=(float ***) malloc((size_t)((nrow+NR_END)*sizeof(float**)));
        t = (float***)eh_malloc( ((size_t)(nrow+NR_END)*sizeof(float***)) ,
                                 NULL , __FILE__ , __LINE__ );
	if (!t) eh_error("allocation failure 1 in f3tensor()");
	t += NR_END;
	t -= nrl;

	/* allocate pointers to rows and set pointers to them */
//	t[nrl]=(float **) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float*)));
        t[nrl]=(float**)eh_malloc(((size_t)(nrow*ncol+NR_END)*sizeof(float*)) ,
                                  NULL , __FILE__ , __LINE__ );
	if (!t[nrl]) eh_error("allocation failure 2 in f3tensor()");
	t[nrl] += NR_END;
	t[nrl] -= ncl;

	/* allocate rows and set pointers to them */
//	t[nrl][ncl]=(float *) malloc((size_t)((nrow*ncol*ndep+NR_END)*sizeof(float)));
        t[nrl][ncl]=(float*)eh_malloc(((size_t)(nrow*ncol*ndep+NR_END)*sizeof(float)) ,
                                  NULL , __FILE__ , __LINE__ );
	if (!t[nrl][ncl]) eh_error("allocation failure 3 in f3tensor()");
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

double ***new_d3tensor( long n_rows , long n_cols , long n_dep )
{
   int i, j;
   double ***t;

   t = eh_new( double** , n_rows );

   t[0] = eh_new( double* , n_rows*n_cols );

   t[0][0] = eh_new( double , n_rows*n_cols*n_dep );

   for ( j=1 ; j<n_cols ; j++ )
      t[0][j] = t[0][j-1]+n_dep;

   for ( i=1 ; i<n_rows ; i++ )
   {
      t[i]    = t[i-1]+n_cols;
      t[i][0] = t[i-1][0]+n_cols*n_dep;
      for ( j=1 ; j<n_cols ; j++ )
         t[i][j]=t[i][j-1]+n_dep;
   }

   return t;
}

void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
   eh_free_mem((FREE_ARG) (m[nrl]+ncl-NR_END));
   eh_free_mem((FREE_ARG) (m+nrl-NR_END));
}

void free_dmatrix( double **m )
{
   if ( m )
   {
      eh_free( m[0] );
      eh_free( m    );
   }
}

void free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
/* free a float f3tensor allocated by f3tensor() */
{
   eh_free_mem((FREE_ARG) (t[nrl][ncl]+ndl-NR_END));
   eh_free_mem((FREE_ARG) (t[nrl]+ncl-NR_END));
   eh_free_mem((FREE_ARG) (t+nrl-NR_END));
}

void free_d3tensor( double ***t )
{
   if ( t )
   {
      if ( t[0] )
      {
         eh_free( t[0][0] );
         eh_free( t[0] );
      }
      eh_free( t );
   }
}

