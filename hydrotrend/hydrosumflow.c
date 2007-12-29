/*
 *  HydroSumFlow.c
 *
 *  Sums the discharges from the snowmelt, icemelt
 *  and precipitation routines, calculates the
 *  annual peak flood.
 *
 *  Stores the lagged overflow and groundwater pool size
 *  for the following year.
 *
 *  Author:    M.D. Morehead  (June 1998)
 *  Author2:   S.D. Peckham   (December 2001)
 *  Author3:   A.J. Kettner   (August-October 2002)(February-April 2003)
 *
 *  Variable	Def.Location	Type	Units	Usage
 *  --------	------------	----	-----	-----
 *  err			various			int		-		error flag, halts program
 *  ii			various			int		-		temporary loop counter
 *  mtotal		HydroSumFlow.c	dbl		%		mass balance total
 *  p			various			int		-		temporary loop counter for events or outlets
 *  Qeventcounter HydroSumFlow.c int	-		
 *
 */

#include <stdlib.h>
#include <math.h>
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"

/*-------------------------
 *  Start of HydroSumFlow
 *-------------------------*/
int hydrosumflow()
{
/*-------------------
 *  Local Variables
 *-------------------*/
int	err, ii, p, Qeventcounter, eventsoccure, x,y;
double	mtotal, Qpeaktemp;

/*------------------------
 *  Initialize Variables
 *------------------------*/
err		= 0;
Ewetannual	= 0.0;
mtotal	= 0.0;
Minput	= 0.0;
Moutput	= 0.0;
MEtotal	= 0.0;
MQprevious	= 0.0;
MQnext	= 0.0;
Qtotal = 0.0;
Qeventcounter = 0;
eventsperyear = 0;
eventsoccure = 0;

if (yr == syear[ep]){
	Qgrandtotal[ep] = 0.0;
	for (x=0; x<eventsnr[ep]; x++){
		if ( setstartmeanQandQs > 0 ){
			daysievent[x] = 0;
			for (p=0; p<maxnoutlet; p++)
				Qgrandtotaloutlet[ep][p][x] = 0.0;
		}
	}	
}
for (p=0; p<maxnoutlet; p++){
	Qpeakperoutlet[p] = 1.0;
	Qtotaloutletannual[p] = 0.0;
	if (setstartmeanQandQs > 0 )
		for (x=0; x<eventsnr[ep]; x++)
			Qtotaloutlet[p][x] = 0.0;
}
Qpeak	= 1.0;
if ( setstartmeanQandQs > 0 )
	for (p=0; p<eventsnr[ep]; p++){
		Qpeakevents[p] = 1.0;
		numberday[p] = 0;
	}

for (ii=0; ii<daysiy; ii++) {

/*------------------------------------------------------------------
 *  Sum the daily flow (m^3/s) and find the annual peak.
 *  Sum the annual total annual flow (m^3/s) and evaporation (m^3)
 *------------------------------------------------------------------*/
	if ( setstartmeanQandQs == 0 ){
		Qsumtot[ii] = Qice[ii] + Qnival[ii] + Qrain[ii] + Qss[ii] + Qexceedgw[ii];
		if ( steadyoutletpctflag == 1 )
			if ( floodvalue[ep] > 0.0 && Qsumtot[ii] > floodvalue[ep] ){
				floodcounter++;
				Qpeakfloodtemp[yr-syear[ep]][ii] = Qsumtot[ii]; 
			}
	}

    if ( setstartmeanQandQs > 0 ){
    /*------------------------------------------------
     *  Check is baseflow isn't > than Qbartotal[ep]
     *------------------------------------------------*/
		if ( baseflowtot[ep] > Qbartotal[ep] ){
			fprintf( stderr, "ERROR: in HydroSumFlow.c; setstartmeanQandQs=%d \n", setstartmeanQandQs );
			fprintf( stderr, "   baseflow is higher than your average discharge in year %d:\n",yr );
			fprintf( stderr, "   baseflow = %e, Qbar= %e. \n",baseflowtot[ep],Qbartotal[ep] );
			fprintf( stderr, "Qgrandtotal[ep]=%e. \n",Qgrandtotal[ep]);
			exit(-1);
		}
		Qsumtot[ii] = (( Qbartotal[ep] - baseflowtot[ep])/Qbartotal[ep])* (Qice[ii] + Qnival[ii] + Qrain[ii]+ Qss[ii] + Qexceedgw[ii]) + baseflowtot[ep];
		if ( steadyoutletpctflag == 1 ){
			for ( p=0; p<eventsnr[ep]; p++ )
				if ( Qsumtot[ii] == Qpeakallevents[ep][p] && eventcounter < (eventsnr[ep]-1) ){				
					eventcounter++;
					numberday[eventsperyear] = ii;
					eventsperyear++;
					p = eventsnr[ep];
				}
		}

    /*--------------------------------------
     *  calculate Qsum daily with baseflow
     *--------------------------------------*/				
		if ( outletmodelflag == 1 )
			for ( p=0; p<maxnoutlet; p++ ){
				Qsum[ii][p] = Qsumtot[ii] * outletpct[p][ep][eventcounter];
				if (Qsum[ii][p] > 10000){
					printf("setstartmeanQandQs=%d\n",setstartmeanQandQs);
					printf("outletpct[p][ep][eventcounter]=%f\n\n",outletpct[p][ep][eventcounter]);
					printf("Qsum=%f, outlet=%d, Qsumtot=%f\n",Qsum[ii][p],p, Qsumtot[ii]);
				}
			}
	daysievent[eventcounter]++;
	} // ( end setstartmeanQandQs > 0 )	
	Qpeak = mx( Qpeak, Qsumtot[ii] );
		
	/*-------------------------------------------------
	 *  Find the largest Qpeak events of this year
	 *  if the number of outlets is unknown or the
	 *  Qfraction shifts per event over the outlets.
	 *-------------------------------------------------*/ 
	if ( steadyoutletpctflag == 1 && setstartmeanQandQs > 0){
		for ( p=0; p<eventsnr[ep]; p++ ) {
			eventsoccure = 0;
			if (Qpeakevents[p] < Qsumtot[ii] && Qeventcounter == (eventsnr[ep]-1)){
				Qpeakevents[p] = mx(Qpeakevents[p], Qsumtot[ii]);
				eventsoccure = 1;
				p = eventsnr[ep];
			}
			if (Qpeakevents[p] < Qsumtot[ii] && Qeventcounter != (eventsnr[ep]-1) && Qpeakevents[p] == 1.0){
				Qpeakevents[p] = mx(Qpeakevents[p], Qsumtot[ii]);
				Qeventcounter++;
				eventsoccure = 1;
				p = eventsnr[ep];
			}		
		}

		/*-------------------------------------
		 *  Rearrange the events of this year
		 * ------------------------------------*/
		if (eventsoccure == 1)
			for (p=0; p<eventsnr[ep]-1; p++)
				while (Qpeakevents[p] > Qpeakevents[p+1]){
					Qpeaktemp = Qpeakevents[p+1];
					Qpeakevents[p+1] = Qpeakevents[p];
					Qpeakevents[p] = Qpeaktemp;
					p = 0;
				}
	} /* end if ( steadyoutletpctflag == 1) */
	
	/*---------------------------------------------------
	 *  Calculate the total discharge of the year,
	 *  per outlet and for the total river (all outlets)
	 *---------------------------------------------------*/
	Qtotal +=Qsumtot[ii];
	if (outletmodelflag == 1 && setstartmeanQandQs > 0){
		for (p=0; p<maxnoutlet; p++){	 
			Qpeakperoutlet[p] = mx( Qpeakperoutlet[p], Qsum[ii][p] );
			Qtotaloutlet[p][eventcounter] += Qsum[ii][p];
		}
	}	
	Ewetannual += ( Egw[ii] * totalarea[ep] ) + ( Ecanopy[ii] * rainarea[ii] );	
} /* end for daysiy */
Qtotal *= dTOs;
Qgrandtotal[ep] += Qtotal;
if (outletmodelflag == 1 && setstartmeanQandQs > 0)
	for (p=0; p<maxnoutlet; p++)
		for (y=eventcounter-eventsperyear; y <eventcounter+1; y++){
				Qtotaloutlet[p][y] *= dTOs;
				Qgrandtotaloutlet[ep][p][y] += Qtotaloutlet[p][y];
				Qtotaloutletannual[p] += Qtotaloutlet[p][y];
		}	
MEtotal += Enivalannual*totalarea[ep] + Eiceannual*glacierarea + Ewetannual;

/*-----------------------------------------------------
 *  Sum the carryover from/to the previous/next years
 *-----------------------------------------------------*/
for (ii=0; ii<maxday-daysiy; ii++)
    MQprevious += (Qrainwrap[ii] + Qicewrap[ii] + Qnivalwrap[ii] + Qsswrap[ii])*dTOs;
for (ii=daysiy; ii<maxday; ii++)
    MQnext += (Qrain[ii] + Qice[ii] + Qnival[ii] + Qss[ii])*dTOs;

/*---------------------------
 *  Set the minimum Qsum[i]
 *  This is done after the
 *  precipitation balance!!
 *---------------------------*/
for (ii=0; ii<daysiy; ii++)
	if (Qsumtot[ii] < 0.1)
		Qsumtot[ii] = 0.1;

/*-----------------------------------------------
 *  Check the precipitation input balance (m^3)
 *-----------------------------------------------
 *  MPglacial  = Mass of precip as glacial/year
 *  MPnival    = Mass of precip as nival/year
 *  MPrain     = Mass of precip as rain/year
 *-----------------------------------------------*/
if ( setstartmeanQandQs == 0 ){
    Moutput = MPnival + MPglacial + MPrain;
    Minput  = (Pannual) * totalarea[ep];
    mtotal  = (Moutput - Minput) / Moutput;

    if (fabs(mtotal) > masscheck ) {
        fprintf( stderr, "ERROR: in HydroSumFlow.c: \n" );
		fprintf( stderr, "   Precipitation Balance Error: fabs(mtotal) > masscheck \n" );
		fprintf( stderr, "   note: masscheck set in HydroParams.h \n" );
		fprintf( stderr, "   masscheck  = %e  \n",		masscheck );
		fprintf( stderr, "   mtotal     = %e (%%) \n",		mtotal );
		fprintf( stderr, "   Minput     = %e (m^3/a) \n",	Minput );
		fprintf( stderr, "   Moutput    = %e (m^3/a) \n",	Moutput );
		fprintf( stderr, "   out-in     = %e (m^3/a) \n",	Moutput-Minput );
		
		fprintf( stderr, "   Year       = %d \n",	yr );
		fprintf( stderr, "   Epoch      = %d \n\n",	ep+1 );

		fprintf( stderr,"   Minput = (Pannual)*totalarea[ep]  \n" );
		fprintf( stderr," \t Minput    \t = \t %e (m^3) \n",	Minput );
		fprintf( stderr," \t Pannual   \t = \t %e (m^3) \n",	Pannual*totalarea[ep] );
		fprintf( stderr," \t Pannual   \t = \t %e (m)   \n",	Pannual );
		fprintf( stderr," \t Pbaseflow \t = \t %e (m) \n\n",	baseflowtot[ep]*totalarea[ep] );

		fprintf( stderr,"   Moutput = MPnival + MPglacial + MPrain \n" );
		fprintf( stderr," \t Moutput   \t = \t %e (m^3) \n",	Moutput );
		fprintf( stderr," \t MPnival   \t = \t %e (m^3) \n",	MPnival );
		fprintf( stderr," \t MPglacial \t = \t %e (m^3) \n",	MPglacial );
		fprintf( stderr," \t MPrain    \t = \t %e (m^3) \n\n",	MPrain );
	
		err = 1;
    }

/*--------------------------------
 *  Check the mass balance (m^3)
 *-----------------------------------------------
 *  Msnowstart = Snow in basin at start of year
 *  Msnowend   = Snow in basin at end of year
 *-----------------------------------------------*/
    Minput  = Pannual*totalarea[ep] + gwlast + Gmass + Msnowstart + MQprevious;
    Moutput = MEtotal + Qtotal + gwstore[daysiy-1] + Msnowend + MQnext;
    mtotal = (Minput - Moutput) / Moutput;

    if (fabs(mtotal) > masscheck) {
		fprintf( stderr, "ERROR: in HydroSumFlow.c: \n" );
		fprintf( stderr, "   Mass Balance Error: fabs(mtotal) > masscheck \n" );
		fprintf( stderr, "   note: masscheck set in HydroParams.h \n" );
		fprintf( stderr, "   masscheck  = %e  \n",	masscheck );
		fprintf( stderr, "   mtotal     = %e (%%) \n",		mtotal );
		fprintf( stderr, "   Minput     = %e (m^3/a) \n",	Minput );
		fprintf( stderr, "   Moutput    = %e (m^3/a) \n",	Moutput );
		fprintf( stderr, "   out-in     = %e (m^3/a) \n",	Moutput-Minput );
	
		fprintf( stderr, "   Year       = %d \n",	yr );
		fprintf( stderr, "   Epoch      = %d \n",	ep+1 );
		fprintf( stderr, "   counterQandQs = %d \n\n",	setstartmeanQandQs );

		fprintf( stderr,"   Minput =  Pannual + gwlast + previousice + previousnival + MQprevious \n" );
		fprintf( stderr," \t Minput \t = \t\t\t %e (m^3) \n",			Minput );
		fprintf( stderr," \t Pannual \t = %e (m) \t %e (m^3)\n",		Pannual, Pannual*totalarea[ep] );
		fprintf( stderr," \t gwlast \t = \t\t\t %e (m^3) \n",			gwlast );
		fprintf( stderr," \t Gmass \t\t = \t\t\t %e (m^3) \n",			Gmass );
		fprintf( stderr," \t Msnowstart \t = \t\t\t %e (m^3) \n",		Msnowstart );
		fprintf( stderr," \t MQprevious \t = \t\t\t %e (m^3) \n\n",		MQprevious );

		fprintf( stderr,"   Moutput =  MEtotal + Qtotal + gwstore + nextice + nextnival + MQnext \n" );
		fprintf( stderr," \t Moutput \t = \t %e (m^3) \n",		Moutput );
		fprintf( stderr," \t MEtotal \t = \t %e (m^3)\n",		MEtotal );
		fprintf( stderr," \t Qtotal  \t = \t %e (m^3) \n",		Qtotal );
		fprintf( stderr," \t gwstore[daysiy-1] = \t %e (m^3) \n",	gwstore[daysiy-1] );
		fprintf( stderr," \t Msnowend \t = \t %e (m^3) \n",		Msnowend );
		fprintf( stderr," \t MQnext \t = \t %e (m^3) \n",		MQnext );
	
	err = 1;
    }   
}
return(err);
}  /* end of HydroSumFlow.c */

