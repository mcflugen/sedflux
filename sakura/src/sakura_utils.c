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

//LIST OF FUNCTIONS defined only for sakura

#include <stdio.h>
#include <math.h>
#include "sakura.h"
/*
double **dmatrix(int nrl, int nrh, int ncl, int nch)
{
	int i;
	double **m;

	m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*));
	if (!m) 
		fprintf(stderr, "allocation failure 1 in dmatrix()\n");
	m -= nrl;

	for(i=nrl;i<=nrh;i++) {
		m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) 
			fprintf(stderr, "allocation failure 2 in dmatrix()\n");
		m[i] -= ncl;
	}
	return m;
}
*/

/* ---bathyA--- *
  *  floor is flat in the box, then slope, and flat again to downstream end
  *  grid interval is constant dx
  */
/*void bathyA(double *Xi, double *Zi, double *Si, double dx)
{
	int i;
	Xi[0] = 0;
	Zi[0] = DEPTHATORIGIN;
	for (i =1; i < NNODES; i++){
		Xi[i] = (double)( i * dx);
		if (Xi[i] <= BOXLENGTH){
			Zi[i] = DEPTHATORIGIN;
			Si[i] = 0;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH){
			Zi[i] = Zi[i-1] - dx * SLOPEGRADIENT;
			Si[i] = SLOPEGRADIENT;
		}
		else {
			Zi[i] = Zi[i-1];
			Si[i] = 0;
			}
	}
}
*/
/* ---bathyB--- *
  *  floor is flat in the box, then slope, and three ups and downs to downstream end
  *  grid interval is constant dx
  */
/*
void bathyB(double *Xi, double *Zi, double *Si, double dx)
{
	int i;
	Xi[0] = 0;
	Zi[0] = DEPTHATORIGIN;
	for (i =1; i < NNODES; i++){
		Xi[i] = (double)( i * dx);
		if (Xi[i] <= BOXLENGTH){
			Zi[i] = DEPTHATORIGIN;
			Si[i] = 0;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH){
			Zi[i] = Zi[i-1] - dx * SLOPEGRADIENT;
			Si[i] = SLOPEGRADIENT;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + HUMPLENGTH){
			Zi[i] = Zi[i-1] + dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = -HUMPHEIGHT/HUMPLENGTH;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + 2 * HUMPLENGTH){
			Zi[i] = Zi[i-1] - dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = HUMPHEIGHT/HUMPLENGTH;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + 3 * HUMPLENGTH){
			Zi[i] = Zi[i-1] + dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = -HUMPHEIGHT/HUMPLENGTH;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + 4 * HUMPLENGTH){
			Zi[i] = Zi[i-1] - dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = HUMPHEIGHT/HUMPLENGTH;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + 5 * HUMPLENGTH){
			Zi[i] = Zi[i-1] + dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = -HUMPHEIGHT/HUMPLENGTH;
		}
		else if (Xi[i] <= BOXLENGTH + SLOPELENGTH + 6 * HUMPLENGTH){
			Zi[i] = Zi[i-1] - dx * (HUMPHEIGHT/HUMPLENGTH);
			Si[i] = HUMPHEIGHT/HUMPLENGTH;
		}
		else {
			Zi[i] = Zi[i-1];
			Si[i] = 0;
			}
	}
}
*/
/* ---bathyC--- *
  *  constant slope and then horizontal
  *  same with default inflow.bathy
  *  grid interval is constant dx
  */
void bathyC(double *Xi, double *Zi, double *Si, double dx)
{
	int i;
	Xi[0] = 0.0;
	Zi[0] = -3.0; /* river depth */
	for (i =1; i < 48; i++)
			Xi[i] = (double)( i * 300.0);
	for (i =1; i < 25; i++){
		 if (Xi[i] <= 13200){
			Zi[2*i -1] = Zi[2*i-2] - 300. * (13.12/300.0);
			Si[2*i -1] = 13.12/300.0;
			Zi[2*i] = Zi[2*i-1];
			Si[2*i] = 0;
		}
		else {
			Zi[i] = Zi[i-1];
			Si[i] = 0;
		}
	}
		Xi[47] = 250000.0;
		Zi[47] = Zi[47];
		Si[47] = 0;
}

int interpolate2(double *x,double *y,int len,double *xNew,double *yNew,int lenNew, double dx)
{
   int i,j;
	double m, b, x0;
	
	xNew[0]=x[0]; yNew[0] = y[0];
	for (j=1; j<lenNew; j++)
			xNew[j] = dx + xNew[j-1];

   // initialize yNew with NaN's.
   for (j=0;j<lenNew;yNew[j]=0,j++);

   // set j to the first index inside of the given data.
   for (j=0;xNew[j]<x[0];j++);

   // interpolate linearly between points.
   for (i=0;i<len-1;i++)
   {
      m = (y[i+1]-y[i])/(x[i+1]-x[i]);
      b = y[i];
      x0 = x[i];
      while ( j<lenNew && xNew[j] <= x[i+1] )
      {
         yNew[j] = m*(xNew[j]-x0)+b;
         j++;
      }
   }

   return 0;
}



/* ---dudt--- *
  * returns (du/dt)
*/
double dudt( double u, double ul, double ur, double ull, double urr, double hl, double hr, double hm, double cl, double cr, double cm, double ustar, double s, double Ew, double smallh, double dx, double Cd, double nu)
{
	double dudx, du, ugrav, upress, ufric, uvisco, uright, uleft;
	double minmod2(double, double);
	double tvdleft(double, double, double, double, double, double);
	double tvdright(double, double, double, double, double, double);
	
//	if (hm < HMIN )
//			fprintf(stderr, "error ...hm too small in dudt\n");
	if (cm < 0 )
			fprintf(stderr, "error ...negative C in dudt; cm=%f\n",cm); 
		
	uright = tvdright(u, u, ul, ur, ull, urr);
	uleft = tvdleft(u, u, ul, ur, ull, urr);
	
	dudx = (uright - uleft)/dx;
	
	/* acceleration due to gravity on slope */
	ugrav = R * G * s * cm;
	
	/* acceleration due to pressure gradient */
	if (hm < HMIN)
			upress = R * G * (cm * (hr-hl) + 0.5 * hm * (cr-cl))/dx;
	else
			upress = 0.5 * R * G / hm * ( cr * hr * hr - cl * hl * hl ) / dx;
		
	/* acceleration due to friction; uStar determined from the energy equation*/
	if (hm < HMIN)
			ufric = 0;
	else
			ufric= Cd * SQ(u)/hm;
	
	uvisco = nu * (1 + 2.5 * cm) * (ur - 2 * u + ul)/dx/dx;
	
	du = -u * dudx + ugrav - upress - ufric -uvisco;
	return du;
}


/* ---dfdt--- *
* returns (d/dx)(u*f)
* ext = Ew
*/
double dfdt(double ul, double ur, double wl, double wr, double fl, double fr, double fll, double frr, double fm, double dx, double ext)
{
	double fluxl, fluxr, dh, w;
	double minmod2(double, double);
	double tvdleft(double, double, double, double, double, double);
	double tvdright(double, double, double, double, double, double);
	
	fluxr = tvdright(ur, fm, fl, fr, fll, frr);
	fluxl = tvdleft(ul, fm, fl, fr, fll, frr);
        w = 0.5 * (wr + wl); 	
	dh = (ul * wl * fluxl - ur * wr * fluxr)/dx/w + ext;
	return dh;
}

/* MINMOD */
double minmod2(double x, double y)
{
	double val;
	if (x * y < 0)
			val = 0;
	else if (fabs(x) <= fabs(y))
			val = x;
	else
			val = y;
	return val;
}

/* TVD method to interpolate values at midpoint of the node */
double tvdright(double u, double f, double fl, double fr, double fll, double frr){
	double ret;
	
	if (fabs(u) <= HMIN)
			ret = 0.0;
	else if ( u > 0 && f - fl == 0 )
			ret = f ;
        else if ( u > 0 && f <= HMIN)
                        ret = f ; 
	else if ( u > 0)
			ret = f + 0.5 * (f - fl) * minmod2(1., (fr - f)/(f - fl));
	else if ( u < 0 && frr - fr ==0 )
			ret = fr;
        else if ( u < 0 && fr <= HMIN)
			ret = fr;
	else
			ret = fr - 0.5 * (frr - fr) * minmod2(1., (fr - f)/(frr - fr));
	
	return ret;
	}
	
double tvdleft(double u, double f, double fl, double fr, double fll, double frr){
	double ret;
	
	if (fabs(u) <= HMIN)
			ret = 0.0;
	else if ( u > 0 && fl - fll == 0 )
			ret = fl ;
	else if ( u > 0 && fl <= HMIN )
			ret = fl ;
	else if ( u > 0)
			ret = fl + 0.5 * (fl - fll) * minmod2(1., (f - fl)/(fl - fll));
	else if ( u < 0 && fr - f == 0 )
			ret = f;
	else if ( u < 0 && f <= HMIN )
			ret = f;
	else
			ret = f - 0.5 * (fr - f) * minmod2(1., (f - fl)/(fr - f));
	
	return ret;
}
double tvd(double u, double fl, double fr, double fll, double frr){
	double ret;
	
	if ( u >= 0 && fl - fll == 0)
			ret = fl;
	else if ( u >= 0)
			ret = fl + 0.5 * (fl - fll) * minmod2(1., (fr - fl)/(fl - fll));
	else if ( u < 0 && frr - fr ==0)
			ret = fr;
	else
			ret = fr - 0.5 * (frr - fr) * minmod2(1., (fr - fl)/(frr - fr));
	
	return ret;
}

/* ---outputData--- *
  * print the results on outputfile 
*/

void outputData(FILE *fp, int NNODES, double totTime, double *U, double *HH, double *CC, double *SED, double *Utemp, double *SEDRATE, double xhead, double uhead, int node)
{
	int	i, day, hr, min;
	double sec;
	void getTime(double, int *, int *, int *, double *);
	
	/* output the time */
	getTime(totTime, &day, &hr, &min, &sec);
	fprintf(fp, "%2d:%2d:%2d:%4.1f\n", day, hr, min, sec);
	fprintf(stdout, "%2d:%2d:%2d:%4.2f\n", day, hr, min, sec);
	
	/* output the flow velocity, thickness, concentration, deposit mass and sedimentation rate at each node */
	for (i = 0; i < NNODES-1; i++) {
		fprintf(fp, "%d\t%f\t%f\t%f\t%f\t%f\t%f\t\n", i, U[i], HH[i], CC[i], Utemp[i], SED[i], SEDRATE[i]);
	}
	
	/* ...and the last node of U and parameters at flow head */
	fprintf(fp, "%d\t%f\n", i, U[i]);
	fprintf(fp, "xhead = %f\tuhead = %f\n", xhead, uhead);
	fprintf(stdout, "xhead = %f\tuhead = %f\theadnode = %d\n\n", xhead, uhead, node);
			
	/* output end-of-record indicator
	*/
	fprintf(fp, "#\n");
	fflush(fp);
}

/* ---getTime---
  *	Gets the days, hours, minutes and seconds from a total number of seconds.
 */
void getTime(double totTime, int *day, int *hr, int *min, double *sec)
{
	*day	= (int)totTime/DAY;
	*hr	= (int)(totTime - ((double)(*day) * DAY))/3600;
	*min	= (int)(totTime - ((double)(*day) * DAY) - ((double)(*hr) * 3600.0))/60;
	*sec	= totTime - ((double)(*day) * DAY) - ((double)(*hr) * 3600.0) - ((double)(*min) * 60);
}



