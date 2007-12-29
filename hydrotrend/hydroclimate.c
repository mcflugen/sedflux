/*
 *  HydroClimate.c
 *
 *  Sets the annual climate values of P and T using
 *  the starting values, the trends and the Std. Dev.
 *  The climate values are assumed to be at the river
 *  mouth. The lapse rate is used later to calculate
 *  the temperature for each altitude bin.
 *
 *  Author:	M.D. Morehead   (June 1998)
 *  Author2:    S.D. Peckham    (December 2001)
 *  Author3:    A.J. Kettner    (November 2002 - January 2003)
 *
 *  Variable	Def.Location	Type	Units	Usage
 *  --------	------------	----	-----	-----
 *  err			various			int		-		error flag, halts program
 *  jj			various			int		-		temporary loop counter
 *  dumdbl		various			double	-		temporary double
 *  sumt		HydroClimate.c	double	-		total temperature
 *  sump		HydroClimate.c	double	-		total precipitation
 *
 */

#include "hydroclimate.h"
#include "hydrotimeser.h"
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydroreadclimate.h"
#include "hydrodaysmonths.h"

/*-------------------------
 *  Start of HydroClimate
 *-------------------------*/
int hydroclimate(gw_rainfall_etc* gw_rain)
{
/*-------------------
 *  Local Variables
 *-------------------*/
int err, jj, ii;
double dumdbl, sumt, sump;
err = 0;

/*----------------------------------------
 *  Calculate Average Annual Temperature
 *  Either by file or by climate generator.
 *----------------------------------------*/
Tannual = 0.0;
if ( raindatafile == 1)
	    Tannual =gw_rain->Tperyear[yr];
else{
	dumdbl = ranarray[nran];
	nran++;
	if (dumdbl >  Tmaxstd)
		dumdbl =  Tmaxstd;
	if (dumdbl < -Tmaxstd)
		dumdbl = -Tmaxstd;
	Tannual = Tstart[ep] + Tchange[ep]*(yr-syear[ep]) + dumdbl*Tstd[ep];
}

/*------------------------------------------
 *  Calculate Average Monthly Temperatures
 *  and scale to actual annual temperature.
 *  Either by file or by climate generator.
 *------------------------------------------*/
if ( raindatafile == 1)
//	for (jj=0; jj<12; jj++){
	for (jj=Jan; jj<=Dec; jj++){
		Tmonth[jj] = 0.0;
//		for( ii=daystrm[jj]-1; ii<dayendm[jj]; ii++ )
		for( ii=start_of(jj)-1; ii<end_of(jj); ii++ )
		    Tmonth[jj] += gw_rain->T[yr-syear[ep]][ii];
//		Tmonth[jj] = (Tmonth[jj]/daysim[jj]);
		Tmonth[jj] = (Tmonth[jj]/days_in_month(jj));
	}
else{
	sumt = 0.0;
//	for (jj=0; jj<12; jj++)
	for (jj=Jan; jj<=Dec; jj++)
		sumt += Tnominal[jj][ep] * days_in_month(jj);
//	for (jj=0; jj<12; jj++)
	for (jj=Jan; jj<=Dec; jj++)
		Tmonth[jj] = Tnominal[jj][ep] - (sumt/daysiy) + Tannual;
}

/*--------------------------------------------
 *  Calculate Total Annual Precipitation (m)
 *  Either by file or by climate generator. 
 *--------------------------------------------*/
Pannual = 0.0;
if ( raindatafile == 1)
	for (ii=0; ii < daysiy; ii++)
	    Pannual +=gw_rain->R[yr-syear[ep]][ii];
else {
	dumdbl = ranarray[nran];    /* Get one random value. */
    nran++;
    if (dumdbl >  Pmaxstd)
    	dumdbl =  Pmaxstd;
    if (dumdbl < -Pmaxstd)
    	dumdbl = -Pmaxstd;
    Pannual = Pmassbal[ep]*Pstart[ep] + Pchange[ep]*(yr-syear[ep]) + dumdbl*Pstd[ep];
    if (Pannual < 0)
    	Pannual = 0.;
}

/*----------------------------------------------------
 *  Calculate Total Monthly Precipitation and
 *  scale to actual annual precipitation
 *  Pmonth (m/mnth)= (m/mth) * ((m/y-m/y) / (m/y))
 *  Either by file or by climate generator.
 *----------------------------------------------------*/
if ( raindatafile == 1)
//	for (jj=0; jj<12; jj++){
	for (jj=Jan; jj<=Dec; jj++){
		Pmonth[jj] = 0.0;
//		for( ii=daystrm[jj]-1; ii<dayendm[jj]; ii++ )
		for( ii=start_of(jj)-1; ii<end_of(jj); ii++ )
		    Pmonth[jj] += gw_rain->R[yr-syear[ep]][ii];  
	}		
else{
    sump = 0.0;
	for (jj=0; jj<12; jj++)
    	sump += Pnominal[jj][ep];
	for (jj=0; jj<12; jj++)
    	Pmonth[jj] = Pnominal[jj][ep] * (Pannual/sump);
}

/*------------------------------------------
 *  Write out debug information if desired
 *------------------------------------------*/
#ifdef DBG
   fprintf( fidlog," HydroClimate: \t year = %d \t Pannual = %7f \t Tannual = %7f \n",yr,Pannual,Tannual);
   if (tblstart[ep] <= yr && yr <= tblend[ep]) {
       for (jj=0; jj<12; jj++ )
           fprintf( fidlog," \t month = %d \t P = %7f \t T = %7f \n",jj+1,Pmonth[jj],Tmonth[jj]);
   }
   fprintf( fidlog,"\n" );
#endif

return(err);
}  /* end of HydroClimate */
