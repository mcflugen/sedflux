/*
 * HydroWeather.c	Calculates the daily values of T and P for each
 *			altitude bin of the basin.
 *
 *	Author:		M.D. Morehead   June 1998
 *  Author2:	A.J. Kettner    September-October 2002
 * 				A.J. Kettner	December 2002
 *
 * Variable		Def.Location	Type	Units	Usage
 * --------		------------	----	-----	-----
 * darray[]		HydroWeather.c	int		-		day of month array
 * dumdbl		various			double	-		temporary double
 * err			various			int		-		error flag, halts program
 * jj			various			int		-		temporary loop counter
 * ii			various			int		-		temporary loop counter
 * ndaysppt		HydroWeather.c  int		-		montly counter for number of days of preci.
 * parray[]		HydroWeather.c	double	m		daily precipitation array for a month
 * pind			HydroWeather.c	int		-		parray index
 * sump			various			double	m		total precipitation
 * sumt			various			double	degC	total temperature
 *
 */
 
#include <stdlib.h>
#include "hydroclimate.h"
#include "hydrotimeser.h"
#include "hydroparams.h"
#include "hydroinout.h"
#include "hydroreadclimate.h"
#include "hydroalloc_mem.h"
#include "hydrofree_mem.h"
#include "hydrodaysmonths.h"
#include "hydrotrend.h"
#include "hydrornseeds.h"
#define MAXIT (3000)
#define swap_dbl_vec( x , i , j ) { double temp; temp=x[i]; x[i]=x[j]; x[j]=temp; }
typedef int (Cost_fcn)( double*, int );

/*--------------------
 *  Global variables
 *--------------------*/
int jj;

/*--------------------
 *  Global functions
 *--------------------*/
double *anneal( double *x , int n , Cost_fcn *f , int cost_min, int jj );
int eh_get_fuzzy_int(int y , int z, int jj, int count);
int cost_fcn( double *x , int n );
float hydroran4(long *idum);

/*-------------------------
 *  Start of HydroWeather
 *-------------------------*/
int hydroweather(gw_rainfall_etc* gw_rain){

/*-------------------
 *  Local Variables
 *-------------------*/
#ifdef DBG
FILE *fid;
#endif

double dumdbl, parray[31], sumt, sump;
double Tstdcorr;
int darray[31], err, ii, pind, count;
int ndaysppt, daysinmnd;
Cost_fcn cost_fcn;
err = 0;

/*-----------------------------------------------------------
 *  Set a correction ratio for the Tstd values
 *  The Tstd's are derived from differnces between monthly
 *  values from different years.  We really want the STD of
 *  the daily values within a year.  This factor appears to
 *  correct this in the proper direction
 *-----------------------------------------------------------*/
Tstdcorr = 0.5;
/*--------------------------------------------------------------
 *  Get the daily Temperature at the river mouth (sealevel) by 
 *  value's (from file).
 *--------------------------------------------------------------*/
if (raindatafile == 1)
	for( ii=0; ii<daysiy; ii++ )
		Tdaily[ii] = gw_rain->T[yr-syear[ep]][ii];
		
/*-----------------------------------------------------------------
 *  Calculate the daily Temperature at the river mouth (sealevel)
 *-----------------------------------------------------------------*/
else
//	for( jj=0; jj<12; jj++ ) {
	for( jj=Jan; jj<=Dec; jj++ ) {
		sumt = 0;
//		for( ii=daystrm[jj]-1; ii<dayendm[jj]; ii++ ) {
		for( ii=start_of(jj)-1 ; ii<end_of(jj) ; ii++ ) {
			dumdbl = ranarray[nran]; nran++;
			Tdaily[ii] = Tmonth[jj] + Tnomstd[jj][ep] * dumdbl * Tstdcorr;
			sumt += Tdaily[ii];
		} /* end ii-day loop */
//		for( ii=daystrm[jj]-1; ii<dayendm[jj]; ii++ )
		for( ii=start_of(jj)-1 ; ii<end_of(jj) ; ii++ )
//			Tdaily[ii] = Tdaily[ii] - (sumt/daysim[jj]) + Tmonth[jj];
			Tdaily[ii] = Tdaily[ii]
                                   - ( sumt/days_in_month(jj) )
                                   + Tmonth[jj];
	} /* end jj-month loop */

/*----------------------------------------------------------------
 *  Get the daily Precipitation at the river mouth (sealevel) by 
 *  value's (from file).
 *----------------------------------------------------------------*/
if (raindatafile == 1)
	for( ii=0; ii<daysiy; ii++ )
		Pdaily[ii] = gw_rain->R[yr-syear[ep]][ii];

/*-------------------------------------------------------------------
 *  Calculate the daily Precipitation at the river mouth (sealevel)
 *  the skewed distribution is the new and more realistic method
 *  Either by file or by climate generator.
 *-------------------------------------------------------------------*/
else{
   for( jj=0; jj<12; jj++ ) {           /* skewed distribution */

      /*---------------------------------------------------------
       *  Generate 31 days worth of skewed distribution numbers
       *---------------------------------------------------------*/
      err = hydroexpdist(parray,jj);
      if(err) fprintf(stderr,"expdist failed in HydroWeather, epoch = %d, year = %d \n", ep+1, yr);

      /*------------------------------------------
       *  Randomly shuffle the days of the month
       *------------------------------------------*/
      err = hydroshuffle(darray,jj);
      if(err) fprintf(stderr,"shuffle failed in HydroWeather, epoch = %d, year = %d \n", ep+1, yr);

      /*---------------------------------------------------------------------
       *  Assign enough rain days to be just under the monthly rainfall.
       *  Assign one less than the # of days in the month, then (if needed)
       *  the last day can make up any slack.
       *---------------------------------------------------------------------*/
      sump = 0.0;
      pind = 0;
      count = 0;
      ndaysppt = 0;
//      while( (sump + parray[pind]) < Pmonth[jj] && pind < daysim[jj]-1 ) {
      while( (sump + parray[pind]) < Pmonth[jj] && pind < days_in_month(jj)-1 )
      {
//         if( daystrm[jj]+darray[pind]-2 >= daysiy ) {
         if( start_of(jj)+darray[pind]-2 >= daysiy ) {
            fprintf(stderr,"ERROR in HydroWeather \n");
            fprintf(stderr,"   # days exceeded 365, case 1 \n");
            exit(1);
         }
//         Pdaily[daystrm[jj]+darray[pind]-2] = parray[pind];            
         Pdaily[start_of(jj)+darray[pind]-2] = parray[pind];            
//         if (Pdaily[daystrm[jj]+darray[pind]-2] != 0.0)
         if (Pdaily[start_of(jj)+darray[pind]-2] != 0.0)
            ndaysppt++;
         sump += parray[pind];
         pind++;
         count++;
      } /* end while */

      /*--------------------------------------------------------------------
       *  add enough rain to another random day (or the last shuffled day)
       *  to achieve Pmonth[jj]
       *--------------------------------------------------------------------*/
//      if( daystrm[jj]+darray[pind]-2 >= daysiy ) {
      if( start_of(jj)+darray[pind]-2 >= daysiy ) {
         fprintf(stderr,"ERROR in HydroWeather \n");
         fprintf(stderr,"   # days exceeded 365, case 2 \n");
         fprintf(stderr,"   # days = daystrm[jj]-2+darray[pind] \n");
//         fprintf(stderr,"   daystrm[jj] = %d \n", daystrm[jj] );
         fprintf(stderr,"   daystrm[jj] = %d \n", start_of(jj) );
         fprintf(stderr,"   darray[pind] = %d \n", darray[pind] );
         exit(1);
      }
//      Pdaily[daystrm[jj]+darray[pind]-2] = Pmonth[jj] - sump;
      Pdaily[start_of(jj)+darray[pind]-2] = Pmonth[jj] - sump;
//      if (Pdaily[daystrm[jj]+darray[pind]-2] != 0.0)
      if (Pdaily[start_of(jj)+darray[pind]-2] != 0.0)
          ndaysppt++;

      /*----------------------------------------------------
       *  Check to make sure all the rain days stay within
       *  reasonable limits.
       *----------------------------------------------------*/
//      for( ii=0; ii<daysim[jj]; ii++ ) {
      for( ii=0; ii<days_in_month(jj); ii++ ) {
//         if( 0.0 > Pdaily[daystrm[jj]-1+ii] ||
//                   Pdaily[daystrm[jj]-1+ii] > Pmonth[jj]+Pnomstd[jj][ep]*Prange[ep] ) {
         if (    Pdaily[start_of(jj)-1+ii] < 0
              || Pdaily[start_of(jj)-1+ii] > Pmonth[jj]+Pnomstd[jj][ep]*Prange[ep] ) {
            fprintf(stderr," HydroWeather ERROR: A daily rainfall value exceeds limits.");
            fprintf(stderr,"    epoch = %d, year = %d, month = %d, day = %d \n",ep+1,yr,jj,ii);
            fprintf(stderr,"    Criteria: 0 < P < monthlyP*STD*Prange \n");
//            fprintf(stderr,"    P = %g (from HydroWeather.c)\n", Pdaily[daystrm[jj]-1+ii] );
            fprintf(stderr,"    P = %g (from HydroWeather.c)\n", Pdaily[start_of(jj)-1+ii] );
            fprintf(stderr,"    monthlyP = %g (from setclimate.c)\n",  Pmonth[jj]);
            fprintf(stderr,"    STD = %g (from input file)\n",  Pnomstd[jj][ep]);
            fprintf(stderr,"    Prange = %g (from input file)\n",  Prange[ep]);
            err = 1;
         }
      }

     /*----------------------------------------------------------
      *  Make the distribution look more natural by grouping
      *  the precipitation days (so random distribution becomes
      *  more a grouped distribution). This is only done when
      *  number of precipitation days is bigger than 2.
      *----------------------------------------------------------*/
//      daysinmnd = (daysim[jj] + daystrm[jj])-1;
      daysinmnd = (days_in_month(jj) + start_of(jj))-1;
      anneal (Pdaily, daysinmnd, &cost_fcn, ndaysppt, jj);
   }    /* end the month for loop of skewed distribution*/
} /* end else */   

#ifdef DBG
  if( tblstart[ep] <= yr && yr <= tblend[ep] ) {
    if( (fid = fopen("hydro.t","a+")) == NULL) {
      printf("  HydroWeather ERROR: Unable to open the temperature file hydro.t \n");
      printf("     non-fatal error, continueing. \n\n");
    }
    else {
      fprintf( fid,"%%HydroWeather: Daily predicted temperatures for year %d \n%%\n", yr );
      fprintf( fid,"%%Day \t Temperature \n", yr );
      fprintf( fid,"%%--- \t ----------- \n", yr );
      for(ii=0; ii<daysiy; ii++ )
         fprintf( fid,"%d \t %f \n",ii+1,Tdaily[ii]);
      fprintf( fid,"%%\n%%\n" );
    }
    if( (fid = fopen("hydro.pr","a+")) == NULL) {
      printf("  HydroWeather ERROR: Unable to open the precipitation file hydro.pr \n");
      printf("     non-fatal error, continueing. \n\n");
    }
    else {
      fprintf( fid,"%%HydroWeather: Daily predicted precipitation for year %d \n%%\n", yr );
      fprintf( fid,"%%Day \t Precipitation \n", yr );
      fprintf( fid,"%%--- \t ------------- \n", yr );
      for(ii=0; ii<daysiy; ii++ )
         fprintf( fid,"%d \t %f \n",ii+1,Pdaily[ii]);
      fprintf( fid,"%%\n%%\n" );
    }
    fclose(fid);
  }
#endif
return(err);
}	/* end of HydroWeather */

/*-------------------------------------
 * Defining area for global functions
 *-------------------------------------*/

/*  FUNCTION TO DISTRIBUTE RAINDAYS IN A MORE "NATURAL WAY" */
double *anneal(double *x, int n, Cost_fcn *f, int cost_min, int jj) {

/*----------------------------------------------------------
 *  This function is ordering the precipitation array in a
 *  random way and compair the results of the number of
 *  switches with the results from before. The smaller the
 *  number, the more the array is grouped. It does it till
 *  the boundery cost_min is reached.
 *----------------------------------------------------------*/
   double cost_before, cost_after;
   int i, j, count;
   int itr=0, max_itr=MAXIT;
   cost_after = 31;
   count=0;
   do {
      cost_before = (*f)(x, n);
//      i = eh_get_fuzzy_int(daystrm[jj], n-1, jj, count);
      i = eh_get_fuzzy_int(start_of(jj), n-1, jj, count);
      count++;
      do
//         j = eh_get_fuzzy_int(daystrm[jj], n-1, jj, count);
         j = eh_get_fuzzy_int(start_of(jj), n-1, jj, count);
      while (j == i);
      swap_dbl_vec(x, i, j);
      cost_after = (*f)(x, n);
      if (cost_after>cost_before)
         swap_dbl_vec(x, i, j);
   }
   while (cost_after > (double)cost_min && ++itr<max_itr);
   return x;
}


/* FUNCTION EH_GET_FUZZY_INT */
int eh_get_fuzzy_int(int y , int z, int jj, int count){
	double x,dumflt;
	
/*---------------------------------------
 *  Just getting a array number between
 *  the startday of the month and the
 *  endday of the month.
 *---------------------------------------*/
	if ( yr == syear[ep] && jj == 0 && count == 0)
		rnseed4 = -INIT_RAN_NUM_SEED;

	dumflt = hydroran4(&rnseed4);				/* get a uniform random number [0:1] */
	if( 0 > dumflt || dumflt > 1 ){
		fprintf( stderr,"A function in HydroRan4 failed in HydroWeather.c \n");
		fprintf( stderr," \t dumflt = %f: \t setting value to 0.5, jj = %d \n", dumflt, jj);
		dumflt=0.5;
	}
	x = dumflt;
	x = x*(z-y)+y;
	return (int)(x+.5);
}


/* FUNCTION Cost_fcn */
int cost_fcn( double *x , int n ){

/*-------------------------------------------------
 *  This routine is flagging the precipitation
 *  days by giving them a value 1, other days
 *  gets a flag value 0. Then it counts how often
 *  it switch from a precipitation day to a non-
 *  precipitation day and returns that value.
 *-------------------------------------------------*/
   int i, *j, k;
   k = 0;
   j = malloc1d(n, int);

//   for (i=daystrm[jj] ; i<n ; i++){
   for (i=start_of(jj) ; i<n ; i++){
       if (x[i] > 0.0)
           j[i] = 1;
       else j[i] = 0;
       if (i>start_of(jj))
           k += fabs(j[i-1] - j[i]);
   }
   freematrix1D( (void*) j );
   return k;
}
