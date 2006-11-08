/*
 *  HydroSnow.c
 *
 *  Calculates the daily snow fall or melt for each altitude bin.
 *  Also calculates the groundwater flow and time lag due to the snow.
 *
 *  This needs to be updated with a thermodynamically based routine
 *  like SNTHERM or ...
 *
 *  Author:    M.D. Morehead  (Dec 1998)
 *  Author2:   S.D. Peckham   (Dec 2001)
 *
 * Variable		Def.Location	Type	Units	Usage
 * --------		------------	----	-----	-----
 * err			various			int		-		error flag halts program
 * ii			various			int		-		temporary loop counter
 * jj			various			int		-		temporary loop counter
 * kk			various			int		-		temporary loop counter
 * shldday[]	HydroSnow.c		double	m^3/s	shoulder discharge array
 * Tcorrection	HydroSnow.c		double	degC	melt modifier for rain fall
 * melt			HydroSnow.c		double	m		snow melt on a given day
 * Minput		HydroSnow.c		double	m^3/a	snow input in a year
 * Mout			HydroSnow.c		double	m^3/a	snow melt output in a year
 * Mwrapin		HydroSnow.c		double	m^3/a	nival discharge from previous year
 * Mwrapout		HydroSnow.c		double	m^3/a	nival discharge to next year
 * Mgw			HydroSnow.c		double	m^3/a	total nival discharge put into GW
 * Mnival		HydroSnow.c		double	m^3/a	total nival discharge
 *
 *
 *  The ELA index for each year (ELAindex) was found in HydroGlacial.c
 *  The FLA index for each day (FLAindex[]) was found in HydroHypsom.c
 *  The FLAflag (=9999) indicates the snow line was above the maximum
 *  altitude of the basin on that day
 *
 */

#include <stdlib.h>
#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrotimeser.h"
#include "hydroinout.h"
#include "hydrodaysmonths.h"

/*----------------------
 *  Start of HydroSnow
 *----------------------*/
int hydrosnow()
{

/*-------------------
 *  Local Variables
 *-------------------*/
#ifdef DBG
FILE *fid;
#endif

int err, ii, jj, kk;
double shldday[maxday], Tcorrection, melt;
double Minput, Mout, Mwrapin, Mwrapout, Mgw, Mnival;

/*-----------------
 *  Set Variables
 *-----------------*/
err = 0;
Minput		= 0.0;
Mgw			= 0.0;
Mnival		= 0.0;
Mout		= 0.0;
MPnival		= 0.0;
Mwrapin		= 0.0;
Mwrapout	= 0.0;
Msnowend	= 0.0;
Msnowstart	= 0.0;
Snowremains	= 0.0;

/*-----------------------------------------------
 *  Make sure the shoulder day arrays are clean
 *-----------------------------------------------*/
for( ii=0; ii<maxday; ii++ )
   shldday[ii] = 0.0;

/*----------------------------------------------------
 *  Calculate the carryover snow for mass balance.
 *  Storing the carryover of snow from previous year
 *  is done in HydroHypsom.c
 *----------------------------------------------------*/
for( kk=0; kk<nelevbins; kk++ )
      Msnowstart += Snowelevday[kk][0] * areabins[kk];

/*--------------------------------------
 *  print out Snow values for checking
 *--------------------------------------*/
#ifdef DBG
  if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
    if( (fid = fopen("hydro.snow1","a+")) == NULL) {
      printf("  HydroHypsim ERROR: Unable to open the snow file hydro.snow1 \n");
      printf("     non-fatal error, continueing. \n\n");
    }
    else {
      fprintf( fid,"%%\n%%\n%%HydroSnow: Starting Snow array for epoch %d, year %d \n%%\n%%", ep+1, yr );
      fprintf( fid,"%%   Elev \t    day \t Snow (m) \n" );
      fprintf( fid,"%%   ---- \t  --------- \t ------------ \n" );
      for( ii=0; ii<daysiy; ii++ )
         for(kk=0; kk<nelevbins; kk++)
            fprintf( fid,"%7.1f \t %d \t %f \n", elevbins[kk], ii, Snowelevday[kk][ii] );
      fclose(fid);
    }
  }
#endif

/*-----------------------------------------------------------
 *  Loop through the days adding snowfall
 *  Loop through the Temperature Elevation bins looking for
 *  potential snowfall.
 *  "Snow" only occurs above the FLA and below the ELA
 *  everything above the ELA is considered "Ice"
 *  Snow is stored in "m" of water equivalent
 *-----------------------------------------------------------*/
for( ii=0; ii<daysiy; ii++ )
   for( kk=FLAindex[ii]; kk<nelevbins && kk<ELAindex; kk++ )
      if( Pdaily[ii] > 0.0 ) {
         Snowelevday[kk][ii] += Pdaily[ii];
         MPnival += Pdaily[ii]*areabins[kk];
      }

/*--------------------------------------
 *  Print out Snow values for checking
 *--------------------------------------*/
#ifdef DBG
  if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
    if( (fid = fopen("hydro.snow2","a+")) == NULL) {
      printf("  HydroHypsim ERROR: Unable to open the snow file hydro.snow2 \n");
      printf("     non-fatal error, continueing. \n\n");
    }
    else {
      fprintf( fid,"%%\n%%\nHydroSnow: Filled Snow array for epoch %d, year %d \n%%\n%%", ep+1, yr );
      fprintf( fid,"%%   Elev \t    day \t Snow (m) \n" );
      fprintf( fid,"%%   ---- \t  --------- \t ------------ \n" );
      for( ii=0; ii<daysiy; ii++ )
         for(kk=0; kk<nelevbins; kk++)
            fprintf( fid,"%7.1f \t %d \t %f \n", elevbins[kk], ii, Snowelevday[kk][ii] );
      fclose(fid);
    }
  }
#endif

/*----------------------------------------------------
 *  Loop through the days/elev bins melting snowfall
 *----------------------------------------------------*/
for( ii=0; ii<daysiy; ii++ ) {
   for( kk=0; kk<ELAindex && kk<nelevbins; kk++ ) {

      /*--------------------------------------------------------
       *  Melt snow if T is warm enough (and if there is snow)
       *--------------------------------------------------------*/
      if( Televday[kk][ii] > 1.0 && Snowelevday[kk][ii] > 0.0 ) {

         /*---------------------------------------------------
          *  Snow melt = 1cm/degC  ( -1cm if it is raining )
	  *  Meltrate  = x m/degC
	  *  Snow melt = Meltrate * T = m/degC * degC = m
          *---------------------------------------------------*/
         Tcorrection = 0.0;
         if( Pdaily[ii] > 0.0 )
            Tcorrection = 1.0;
	 melt = mn( Meltrate[ep]*(Televday[kk][ii]-Tcorrection), Snowelevday[kk][ii] );

	 /*---------------------------------------------------------------------
	  *  (Mark's version of routing)
	  *  Add the time lag for the distance up the basin (distbins[elabin])
	  *---------------------------------------------------------------------*/
         Qnival[ii+distbins[kk]]	+= melt*areabins[kk]/dTOs;
         Snowelevday[kk][ii]		-= melt;
      }
   }

   /*-------------------------------------------------
    *  Check how much snow is remaining on August 31
    *-------------------------------------------------*/
//	if( ii == dayendm[7] ) {
	if ( ii == end_of(Aug) ) {
		for( kk=0; kk<nelevbins; kk++ )
			Snowremains += Snowelevday[kk][ii]*areabins[kk];

#ifdef DBG
fprintf( stderr, "\n HydroSnow: \t Snow on August 31 = %e (m^3) \n", Snowremains );
fprintf( fidlog, "\n HydroSnow: \n\n" );
fprintf( fidlog, "\t Snow on August 31 \t\t = %e (m^3) \n", Snowremains );
#endif
	if (setstartmeanQandQs == 4)
		if( Snowremains > 1 ) {
			fprintf( stderr, "\n \t HydroSnow Warning: There is Snow remaining on August 31 \n" );
			fprintf( stderr, " \t Snowremains \t = %e (m^3) \n", Snowremains );
			fprintf( stderr, " \t in year: %d \n",yr );
			fprintf( fidlog, "\n \t HydroSnow Warning: There is Snow remaining on August 31 \n" );
			fprintf( fidlog, " \t Snowremains \t = %e (m^3) \n", Snowremains );
			fprintf( fidlog, " \t in year: %d \n",yr );
		}
	}

  /*------------------------------------------
   *  Add any remaining snow to the next day
   *------------------------------------------*/
   for( kk=0; kk<nelevbins; kk++ )
      if( ii < daysiy-1 ) {
         Snowelevday[kk][ii+1] += Snowelevday[kk][ii];
         Snowelevday[kk][ii] = 0.0;
      }
} /* end daily loop */

/*--------------------------------------
 *  Print out Snow values for checking
 *--------------------------------------*/
#ifdef DBG
  if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
    if( (fid = fopen("hydro.snow3","a+")) == NULL) {
      printf("  HydroHypsim ERROR: Unable to open the snow file hydro.snow3 \n");
      printf("     non-fatal error, continueing. \n\n");
    }
    else {
      fprintf( fid,"%%\n%%\n%%HydroSnow: Emptied Snow array for epoch %d, year %d \n%%\n%%", ep+1, yr );
      fprintf( fid,"%%   Elev \t    day \t Snow (m) \n" );
      fprintf( fid,"%%   ---- \t  --------- \t ------------ \n" );
      for( ii=0; ii<daysiy; ii++ )
         for(kk=0; kk<nelevbins; kk++)
            fprintf( fid,"%7.1f \t %d \t %f \n", elevbins[kk], ii, Snowelevday[kk][ii] );
      fclose(fid);
    }
  }
#endif

/*-------------------------------------------------------------------
 *  Distribute the melted snow to E, GW, and shoulders for each day
 *  account for evaporation (m/day)
 *  I have distributed the E by the area of the basin
 *  which is not technically correct, but creates the
 *  correct units
 *-------------------------------------------------------------------*/
for( ii=0; ii<daysiy; ii++ ) {
   Enivalannual += Qnival[ii]*dryevap[ep]*dTOs/totalarea[ep];
   Qnival[ii]  -= Qnival[ii]*dryevap[ep];
}

/*------------------------------------------------------------
 *  Create the shoulder events (Murray's version of routing)
 *  there is one left (preceeding) day scaled as:
 *  shoulderleft*event
 *  the main event is scaled down to:
 *  shouldermain*event
 *  there are 1 or more right (following days) scaled to:
 *  shoulderright[]*event
 *  1.0 = Sum(shoulderleft+shouldermain+shoulderright[])
 *------------------------------------------------------------*/
ii = 0;
if( Qnival[ii] > 0.0 ) {
   shldday[ii] += shoulderleft*Qnival[ii];
   for( jj=0; jj<shouldern-2; jj++ )
      shldday[ii+jj+1] += shoulderright[jj]*Qnival[ii];
   Qnival[ii] = shouldermain*Qnival[ii];
}
for( ii=1; ii<daysiy; ii++ )
   if( Qnival[ii] > 0.0 ) {
      shldday[ii-1] += shoulderleft*Qnival[ii];
      for( jj=0; jj<shouldern-2; jj++ )
         shldday[ii+jj+1] += shoulderright[jj]*Qnival[ii];
      Qnival[ii] = shouldermain*Qnival[ii];
   }

/*----------------------------------------------------------------
 *  Add the shoulder events and the main events to get the total
 *----------------------------------------------------------------*/
for( ii=0; ii<maxday; ii++ )
   Qnival[ii]	+= shldday[ii];

/*--------------------------------------------
 *  Add the carryover from the previous year
 *  and track it's mass
 *--------------------------------------------*/
for( ii=0; ii<maxday-daysiy; ii++ ) {
   Qnival[ii]	+= Qnivalwrap[ii];
   Mwrapin	+= Qnivalwrap[ii]*dTOs;
}

#ifdef DBG
fprintf( fidlog, "\t Snow Mwrapin \t = %e (m^3) \n", Mwrapin );
#endif

/*-------------------------------------------------------------
 *  Add to the flux to the Groundwater pool
 *  The actual addition to the GW pool is done in HydroRain.c
 *-------------------------------------------------------------*/
for( ii=0; ii<daysiy; ii++ ) {
   Qnivaltogw[ii]	= percentgw[ep]*Qnival[ii];
   Qnival[ii]		-= Qnivaltogw[ii];
   Mgw			+= Qnivaltogw[ii]*dTOs;
}

/*--------------------------------
 *  Check the mass balance (m^3)
 *--------------------------------*/
for( ii=0; ii<daysiy; ii++ )
   Mnival	+= Qnival[ii]*dTOs;
for( ii=daysiy; ii<maxday; ii++ )
   Mwrapout	+= Qnival[ii]*dTOs;
for( kk=0; kk<nelevbins; kk++ )		/* the remaining snow */
   Msnowend	+= Snowelevday[kk][daysiy-1]*areabins[kk];

#ifdef DBG
 if( yr >= 4690 ) {
    fprintf(stderr, " Msnowend = %f \n", Msnowend );
    for( kk=0; kk<nelevbins; kk++ )
       fprintf(stderr, " snow = %f, area = %f \n", Snowelevday[kk][daysiy-1], areabins[kk] );
 }
#endif

Mout = Mgw + Mnival + Enivalannual*totalarea[ep] + Msnowend + Mwrapout;
Minput = MPnival + Mwrapin + Msnowstart;

if( (fabs(Mout-Minput)/Minput) > masscheck ) {
   fprintf( stderr, "\nERROR in HydroSnow: \n");
   fprintf( stderr, "  Mass Balance error: Mout != Minput \n\n");

   fprintf( stderr, "\t fabs(Mout-Minput)/Minput > masscheck \n" );
   fprintf( stderr, "\t note: masscheck set in HydroParams.h \n" );
   fprintf( stderr, "\t masscheck     \t = %f (%%) \n", masscheck );
   fprintf( stderr, "\t abs(out-in)/in\t = %f (%%) \n", fabs(Mout-Minput)/Minput );
   fprintf( stderr, "\t out-in        \t = %e (m^3) \n\n", Mout-Minput );

   fprintf( stderr, " \t Minput = MPnival + Mwrapin + Msnowstart (m^3) \n" );
   fprintf( stderr, " \t Minput       \t = %e \n", Minput );
   fprintf( stderr, " \t MPnival      \t = %e \n", MPnival );
   fprintf( stderr, " \t Mwrapin      \t = %e \n", Mwrapin );
   fprintf( stderr, " \t Msnowstart   \t = %e \n\n", Msnowstart );

   fprintf( stderr, " \t Mout = Mnival + Mgw + Enivalannual + Msnowend + Mwrapout (m^3) \n" );
   fprintf( stderr, " \t Mout         \t = %e \n", Mout );
   fprintf( stderr, " \t Mnival       \t = %e \n", Mnival );
   fprintf( stderr, " \t Mgw          \t = %e \n", Mgw );
   fprintf( stderr, " \t Enivalannual \t = %e \n", Enivalannual*totalarea[ep] );
   fprintf( stderr, " \t Mwrapout     \t = %e \n", Mwrapout );
   fprintf( stderr, " \t Msnowend     \t = %e \n\n", Msnowend );
   exit(-1);
}

return(err);
}  /* end of HydroSnow.c */

