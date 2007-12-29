/*
 * HydroShuffle.c	randomly shuffles the days of the month for climate
 *			simulations
 *
 *  Author:    M.D. Morehead    (June 1998)
 *  Author2:   A.J. Kettner     (August 2002)
 *
 * Variable		Def.Location	Type	Units	Usage
 * --------		------------	----	-----	-----
 * dumflt		HydroShuffle.c	float	-		temporary float
 * dvals[31]	HydroShuffle.c	int		-		shuffled array of daily index values
 * err			HydroShuffle.c	int		-		error flag, halts program
 * ii			HydroShuffle.c	int		-		temporary loop counter
 * mnth			HydroShuffle.c	int		-		month of the year
 * yy			HydroShuffle.c	int		-		temporary integer
 * zz			HydroShuffle.c	int		-		temporary integer
 *
 */

#include "hydroclimate.h"
#include "hydroparams.h"
#include "hydrodaysmonths.h"
#include "hydrornseeds.h"


#ifdef DBG
#include "hydroinout.h"
#endif

#define ntot (93)

/*---------------------------
 *  Start of HydroShuffle.c
 *---------------------------*/
int hydroshuffle(int dvals[31],int mnth)
{

float hydroran3(long *idum);
float dumflt;
double dummy_double;
int yy, zz, ii, err;

/*------------------------
 *  Initialize variables
 *------------------------*/
err   = 0;
//for( ii=0; ii<daysim[mnth]; ii++ )
for( ii=0; ii<days_in_month(mnth); ii++ )
      dvals[ii] = ii+1;

/*---------------------------------
 *  shuffle the days of the month
 *---------------------------------*/
if ( yr == syear[ep] && mnth == 0)
   rnseed3 = -INIT_RAN_NUM_SEED; 

//for( ii=0; ii<daysim[mnth]; ii++ ) {
for( ii=0; ii<days_in_month(mnth) ; ii++ ) {
   dumflt = hydroran3(&rnseed3);				/* get a uniform random number [0:1] */
   if( 0 > dumflt || dumflt > 1 ){
      fprintf( stderr,"A function in HydroRan2 failed in HydroShuffle.c \n");
      fprintf( stderr," \t dumflt = %f: \t setting value to 0.5, ii = %d \n", dumflt, ii);
      dumflt=0.5;
   }

//   dummy_double = dumflt*(float)daysim[mnth];
   dummy_double = dumflt*(float)days_in_month(mnth);
   yy = (int)rnd(dummy_double);                 /* scale to a random day */
   if( yy == 0 ) yy += 1;
   zz = dvals[yy-1];				/* swap random day with day ii */
   dvals[yy-1] = dvals[ii];
   dvals[ii] = zz;
}

#ifdef DBG
  if( tblstart[ep] <= yr && yr <= tblend[ep] && mnth == 0 ) {
   fprintf( fidlog, "\n HydroShuffle.c:\n");
//   fprintf( fidlog, "    epoch = %d, year = %d, month = %d, daysim = %d \n",ep+1,yr,mnth+1,daysim[mnth]);
   fprintf( fidlog, "    epoch = %d, year = %d, month = %d, daysim = %d \n",ep+1,yr,mnth+1,days_in_month(mnth));
//   for( ii=0; ii<daysim[mnth]; ii++)
   for( ii=0; ii<days_in_month(mnth); ii++)
      fprintf( fidlog, "    ii = %d, \t dvals[ii] = %d \n", ii, dvals[ii] );
   fprintf( fidlog, "\n");
   fflush( fidlog );
  }
#endif

return(err);
} /* end of HydroShuffle.c */
