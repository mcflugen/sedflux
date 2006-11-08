/*
 *  HydroHypsom.c
 *
 *  Calculates/converts the hypsometric integral data onto a
 *  regular grid (in elevation).  Then calculates the daily
 *  temperature for each altitude/area bin based on the lapse
 *  rate and the hypsometric integral.  This is used later to
 *  determine snow/glacial melt and accumulation.
 *
 *	Author:    M.D. Morehead  (Oct 1998)
 *	Author2:   S.D. Peckham   (Jan 2002)
 *  Author3:   A.J. Kettner   (October 2002)
 *
 *  Variable	Def.Location	Type	Units	Usage
 *  --------	------------	----	-----	-----
 *  dumdbl		various			double	-		temporary double
 *  err			various			int		-		error flag, halts program
 *  ii			various			int		-		temporary loop counter
 *  kk			various			int		-		temporary loop counter
 *  noldelevbinsHydroHypsom.c	int		-		number of elev bins from previous epoch
 *  totarea		HydroHypsom.c	double	m^2		total basin area summed from areabins
 *  tst			HydroHypsom.c	int		-		error checking flag
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "hydroparams.h"
#include "hydroclimate.h"
#include "hydrotimeser.h"
#include "hydroalloc_mem.h"
#include "hydrofree_mem.h"

#ifdef DBG
#include "hydroinout.h"
#endif

/*------------------------
 *  Start of HydroHypsom
 *------------------------*/
int hydrohypsom()
{
#ifdef DBG
FILE *fid;
#endif

int err;
int kk, ii, noldelevbins, tst;
double totarea, dumdbl, *cumarea;

err = 0;
noldelevbins = 0;

cumarea = malloc1d ( nhypts[ep], double );

/*-----------------------------------------
 *  Check for FloodExceedence
 *  If exceeded just refill the Snowarray
 *  with last years leftover snow
 *-----------------------------------------*/
if (floodtry == 0) {

   /*-------------------------------
    *  Remember the old array size
    *-------------------------------*/
   if (yr > syear[0])
      noldelevbins = nelevbins;

   /*----------------------------------------
    *  Find the new number of altitude bins
    *----------------------------------------*/
   if (yr == syear[ep])
       nelevbins = (int)floor( (maxalt[ep]/elevbinsize) + 1 );
   if (yr == syear[0])
       noldelevbins = nelevbins;

   /*----------------------------------
    *  Free the snow carry over array
    *----------------------------------*/
   if( ep > 0 && yr == syear[ep] )
      free(Snowcarry);

   /*--------------------------------------
    *  Allocate the snow carry over array
    *--------------------------------------*/
   if( yr == syear[ep] )
       if( (Snowcarry = (double *) calloc(nelevbins,sizeof(double)))==NULL ) {
	       fprintf(stderr," PlumeArray ERROR: memory allocation failed \n");
	       fprintf(stderr,"    failed on Snowcarry \n");
	       exit(1);
       }

      /*
       *	Fill the snow carry over array
       *	(still need the old areas at this point)
       *
       *	old < new
       *		new	1 2 3 4 5 6
       *			  ^ ^ ^ ^ ^
       *		old	  1 2 3 4 5
       *
       *	old = new
       *		new	1 2 3 4 5
       *			^ ^ ^ ^ ^
       *		old	1 2 3 4 5
       *
       *	old > new
       *		new	1 1 2 3 4 5
       *			^ ^ ^ ^ ^ ^
       *		old	1 2 3 4 5 6
       *
       *	reasons for this method:
       *	1) keeps the snow at the high altitudes for first two cases
       *	2) does not pile up a bunch of snow at high altitude for last case
       *		therefore it is more likely to melt in the summer
       */
   if( yr == syear[0] )
       for(kk=0; kk<noldelevbins; kk++ )
	       Snowcarry[kk] = 0.0;
   else {
       if( noldelevbins <= nelevbins )
           for( kk=0; kk<noldelevbins; kk++ )
               Snowcarry[kk+(nelevbins-noldelevbins)] =	\
	   	             Snowelevday[kk][daysiy-1]*areabins[kk];
       else {
           for( kk=0; kk<nelevbins; kk++ )					/* zero the array */
               Snowcarry[kk] = 0.0;
           for( kk=0; kk<(noldelevbins-nelevbins); kk++ )	/* add the lowest bins together */
               Snowcarry[0] += Snowelevday[kk][daysiy-1]*areabins[kk];
           for( kk=0; kk<nelevbins; kk++ )					/* add the rest of the bins */
               Snowcarry[kk] +=	\
                     Snowelevday[kk+(noldelevbins-nelevbins)][daysiy-1]*areabins[kk];
       }
   }	/* endifelse filling snow carry over array */

   /*--------------------------------------------------
    *  Calculate the Hypsometric integral information
    *  allocate the elevation related arrays
    *--------------------------------------------------*/
   if( yr == syear[ep] ) {

      /*---------------------------------------------------
       *  Free up the old arrays before creating new ones
       *---------------------------------------------------*/
       if( ep > 0 ) {
           free(elevbins);
           free(distbins);
           free(areabins);
           free_dmatrix(Televday,0,noldelevbins,0,daysiy);
           free_dmatrix(Snowelevday,0,noldelevbins,0,daysiy);
       }

      /*------------------------------------------------------------------------
       *  Allocate memory for Altitude bins, area bins, snow bins, and T array
       *------------------------------------------------------------------------*/
       if( ( elevbins = (double *) calloc( nelevbins, sizeof(double))) == NULL ||
             ( distbins =    (int *) calloc( nelevbins, sizeof(int)))    == NULL ||
             ( areabins = (double *) calloc( nelevbins, sizeof(double))) == NULL ) {
	       fprintf(stderr," PlumeArray ERROR: memory allocation failed \n");
	       fprintf(stderr,"    failed on elevbins, distbins, or areabins \n");
	       exit(1);
       }
       Televday    = dmatrix(0,nelevbins,0,daysiy);
       Snowelevday = dmatrix(0,nelevbins,0,daysiy);

      /*-------------------------------
       *  Calculate the Altitude bins
       *-------------------------------*/
       for (kk = 0; kk<nelevbins; kk++){
	       elevbins[kk] = 0 + kk*elevbinsize;
       }
       
      /*-----------------------------------------------
       *  Create the area/elevation relationship
       *  Use digitized data and linear interpolation
       *-----------------------------------------------*/

         /*----------------------------
          *  Find the cumulative area
          *----------------------------*/
       cumarea[0] = hypsarea[ep][0];
	   for (kk=1; kk<nelevbins-1; kk++) {
	       tst = 0;
	       for (ii=1; ii<nhypts[ep]; ii++)
	           if (elevbins[kk]>hypselev[ep][ii-1] && elevbins[kk]<=hypselev[ep][ii]) {
 	               cumarea[kk] = hypsarea[ep][ii-1]				\
          	             + ( (elevbins[kk]-hypselev[ep][ii-1])	\
                         / (hypselev[ep][ii]-hypselev[ep][ii-1]) )	\
                         * (hypsarea[ep][ii]-hypsarea[ep][ii-1]);
	               tst = 1;
	           }
	       if( tst == 0 ) {
	           fprintf(stderr," HydroHypsom ERROR: \n" );
	           fprintf(stderr,"\t Hypsometric elevation not interpolated. \n");
	           fprintf(stderr,"\t kk = %d, elevbins[kk] = %f \n", kk, elevbins[kk] );
	           err++;
	       }
	   }
       cumarea[nelevbins-1] = totalarea[ep];
       areabins[0] = cumarea[0];
       totarea = areabins[0];
       for( kk=1; kk<nelevbins; kk++ ) {
           areabins[kk] = cumarea[kk] - cumarea[kk-1];
	       totarea += areabins[kk];
	   }

      /*------------------------
       *  Check the total area
       *------------------------*/
       if (fabs(totarea - totalarea[ep]) > 0.001) {
           fprintf(stderr, " ERROR in HydroHypsom, totarea != totalarea in ep=%d \n", ep+1);
           fprintf(stderr, "\t totarea    = %f \n", totarea   );
           fprintf(stderr, "\t totalarea  = %f \n", totalarea[ep] );
           fprintf(stderr, "\t totarea-totalarea  = %f \n", totarea-totalarea[ep] );
           err = 1;
       }

      /*-----------------------------------------------------------
       *  Create the distance relationship; distbins (days).
       *  This scales the total distance by the area relationship
       *  and allows an approximation of flow duration from each
       *  altitude bin to give a first approximation of routing.
       *-----------------------------------------------------------*/
       dumdbl = 0.0;
       for (kk=0; kk<nelevbins; kk++) {
           dumdbl += areabins[kk];
           distbins[kk] = (int)((basinlength[ep]/(avgvel[ep]*dTOs))*(dumdbl/totalarea[ep]));
       }

      /*-----------------------------------------------------------
       *  Are there enough overflow days (maxday) for the basin ?
       *-----------------------------------------------------------*/
       if (distbins[nelevbins-1]+daysiy > maxday) {
           fprintf(stderr, " ERROR in HydroHypsom: \n");
           fprintf(stderr, "\t The number of overflow days/year is too small, \n" );
           fprintf(stderr, "\t or the length relationship for the basin failed.\n" );
           fprintf(stderr, "\t\t distbins[nelevbins-1]+daysiy > maxday \n" );
           fprintf(stderr, "\t\t maxday = %d \n", maxday );
           fprintf(stderr, "\t\t daysiy = %d \n", daysiy );
           fprintf(stderr, "\t\t distbins[nelevbins-1] = %d \n", distbins[nelevbins-1] );
           err = 1;
       }

#ifdef DBG
    fprintf(fidlog, " HydroHypsom: \t totarea    = %f (km^2) \n", totarea/1e6   );
    fprintf(fidlog, " \t\t totalarea  = %f (km^2) \n\n", totalarea[ep]/1e6 );
#endif
    }	/* endif create Hypsometric Info */
}	/* end the flood exceedance check */

/*-----------------------------
 *  Initialize the Snow array
 *-----------------------------*/
for( kk=0; kk<nelevbins; kk++ )
    for( ii=0; ii<daysiy; ii++ )
        Snowelevday[kk][ii] = 0.0;

/*---------------------------------------------
 *  Fill in the snow left over from last year
 *---------------------------------------------*/
for( kk=0; kk<nelevbins; kk++ )
    Snowelevday[kk][0] = Snowcarry[kk]/areabins[kk];

/*---------------------------------------------------
 *  Set the FLAindex for each day to 9999
 *  This indicates no freezing in basin on that day
 *---------------------------------------------------*/
for (ii=0; ii<daysiy; ii++)
    FLAindex[ii] = FLAflag;

/*-----------------------------------------------------------------
 *  For each day of the year calculate the T in each altitude bin
 *  Televday(nelevbins,365days)
 *  Also Flag the FLA, the lowest bin with freezing temperatures
 *-----------------------------------------------------------------*/
for( kk = 0; kk<nelevbins; kk++ )
    for( ii = 0; ii<daysiy; ii++ ) {
        Televday[kk][ii] = Tdaily[ii] - lapserate[ep]*elevbins[kk];
       
        if( Televday[kk][ii] < 0.0 && FLAindex[ii] == FLAflag ){
            FLAindex[ii] = kk;
        }
    }

#ifdef DBG
  /*-----------------------------------
   *  Print out Televday for checking
   *-----------------------------------*/
    if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
        if( (fid = fopen("hydro.tt","a+")) == NULL) {
            printf("  HydroHypsom ERROR: Unable to open the temperature file hydro.tt \n");
            printf("     non-fatal error, continuing. \n\n");
        }
        else {
            for(kk=0; kk<nelevbins; kk++) {
                fprintf( fid,"%%\n%%\n%%HydroHypsom: Daily predicted temperatures for epoch %d, year %d, elevation %f \n%%\n", ep+1, yr, elevbins[kk] );
                fprintf( fid,"%%Elev \t Day \t Temperature \n", yr );
                fprintf( fid,"%%---- \t --- \t ----------- \n", yr );
                for(ii=0; ii<daysiy; ii++ )
                    fprintf( fid,"%7.1f \t %d \t %f \n", elevbins[kk], ii+1, Televday[kk][ii]);
            }
            fclose(fid);
        }
    }
#endif

#ifdef DBG
  /*--------------------------------------------
   *  Print out Hypsometric Areas for checking
   *--------------------------------------------*/
    if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
        if( (fid = fopen("hydro.hp","a+")) == NULL) {
            printf("  HydroHypsom ERROR: Unable to open the hypsometry file hydro.hp \n");
            printf("     non-fatal error, continuing. \n\n");
        }
        else {
            fprintf( fid,"\n\nHydroHypsom: Area bins for epoch %d, year %d \n\n", ep+1, yr );
            fprintf( fid,"   Elev \t    Area \t Time-Distance \n" );
            fprintf( fid,"   ---- \t  --------- \t ------------ \n" );
            for(kk=0; kk<nelevbins; kk++)
               fprintf( fid,"%7.1f \t %e \t %d \n", elevbins[kk], areabins[kk], distbins[kk] );
            fprintf( fid,"\nTotal Area = \t %e, Basin length = %e, AvgVel = %f \n", totalarea[ep], basinlength[ep], avgvel[ep] );
            fclose(fid);
        }
    }
#endif

freematrix1D( (void*)cumarea);
return(err);
}  /* end of HydroHypsom */
