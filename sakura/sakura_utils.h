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

#if !defined( SAKURA_UTILS_H )
#define SAKURA_UTILS_H

//double **dmatrix(int nrl, int nrh, int ncl, int nch);
void bathyC(double *Xi, double *Zi, double *Si, double dx);
int interpolate2(double *x,double *y,int len,double *xNew,double *yNew,int lenNew, double dx);
double dudt( double u, double ul, double ur, double ull, double urr, double hl, double hr, double hm, double cl, double cr, double cm, double ustar, double s, double Ew, double smallh, double dx, double Cd, double nu);
double dfdt(double ul, double ur, double wl, double wr, double fl, double fr, double fll, double frr, double fm, double dx, double ext);
double tvdright(double u, double f, double fl, double fr, double fll, double frr);
double tvdleft(double u, double f, double fl, double fr, double fll, double frr);
double minmod2(double x, double y);
void outputData(FILE *fp, int NNODES, double totTime, double *U, double *HH, double *CC, double *SED, double *Utemp, double *SEDRATE, double xhead, double uhead, int node);
void getTime(double totTime, int *day, int *hr, int *min, double *sec);

#endif // sakura_utils.h is included */
