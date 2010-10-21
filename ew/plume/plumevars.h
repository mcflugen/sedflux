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

#ifndef __PLUMEVARS_IS_INCLUDED__
#define __PLUMEVARS_IS_INCLUDED__
/*
 *	PlumeVars.h
 *
 *	Author:		M.D. Morehead
 *	Original:	April 1998
 *	Modified:	Sep 1998, MDM, Conversion for SEDFLUX3D
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	Useful values
 */

#define TMLEN 19		/* length of the time stamp */
//extern int	strt;			/* Straight plume flag, if straight (or Fjord) = 1, else = 0 */

/* 
 *	Commonly used units and constants
 */
#define sq(a)	((a)*(a))
#define mn(a,b) (((a)<(b))?(a):(b))
#define mx(a,b) (((a)>(b))?(a):(b))
#define rnd(a)  ((fmod(a,1.0)>(0.5)?((float)((int)(a)+1)):((float)((int)(a)))))
#define PI	3.1415926535
#define sqtwo	1.414213562	/* sqrt(2) */  
#define dTOs	86400.0		/* 60.*60.*24. days to seconds or (s/day) */
#define grv	9.806194	/* acceleration due to gravity (m/s^2)	*/
#define rTOdeg	57.29577951	/* 360.0/(2.0*PI) convert radians to degrees */
#define degTOr	1.745329252e-2	/* 2.0*PI/360.0 convert degrees to radians */
#define sqpi	1.772453851	/* sqrt(PI) commonly used values */
#define omega	7.292123517e-5	/* 2.0*PI/86164.0 angular rotation rate of the earth */
/*				86164s/sidereal day, 86400s/solar day */
/*
 *	Model Coefficients
 */
#define C1	0.109		/* Albertson Constant */
#define ucrit	0.0001		/* if(vo<ucrit*uo) assume vo==0 */
#define pcrit	0.01		/* if(ccnc<pcrit) assume ccnc=0 (kg/m^3) */
#define plg	5.17605		/* plg*bo = length of plug flow from river mouth */
#define npts	11		/* +/- number of points to search for plume centerline */
#define mberr	0.4		/* Maximum mass balance error:  abs((tdeps-triver)/triver) < mberr */
#define sprd	tan(14*degTOr)	/* spread angle of the plume = 14 degrees */
/*
 *	File id numbers
 */
extern FILE	*fidlog;		/* Log file */
/*
 *	Numerical Recipes utilities (nrutil) array types
 */
void nrerror(char []);
float     **matrix( long, long, long, long);
double   **new_dmatrix( long, long);
float  ***f3tensor( long, long, long, long, long, long);
double ***new_d3tensor( long, long, long);
void free_matrix(   float **, long, long, long, long);
void free_dmatrix(  double ** );
void free_f3tensor(  float ***, long, long, long, long, long, long);
void free_d3tensor( double *** );

/*
 *	Data arrays
 */
/*
extern double ***ccnc, ***ncnc, ***deps, ***dist;
extern double **ualb, **pcent, **sln;
extern double *xval, *yval;
extern int lc, lpc, lx, ly, zx, zy;
*/

/*
 *	Mass balance values
 *	(Qsw[4] size must match xi[4] size and assignments in PlumeMass.c)
 */
//extern double Qsr, Qsw[4], Tsr, Tsd[2], merr;

#ifdef __cplusplus
}
#endif

#endif /* plumevars.h is included*/
