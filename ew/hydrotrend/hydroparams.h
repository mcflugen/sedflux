
/*
 *  HydroParams.h
 *
 *  Define input parameters and common constants
 *
 *  Author:    M.D. Morehead (June 1998)
 *  Author2:   S.D. Peckham  (Sep 2001)
 *  Author3:   A.J. Kettner  (August - October 2002)
 *
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

/*----------------------
 *  Physical Constants
 *----------------------*/
#define rho_water (1000.0)		/* density of water (kg/m^3) */
#define rho_sed	(2670.0)		/* density of sediment (quartz) (kg/m^3) */

/*--------------------------
 *  Mathematical Constants
 *--------------------------*/
#define PI	(3.1415926535)

/*------------------------
 *  Conversion Constants
 *------------------------*/
#define dTOs	(86400.0)		/* days to seconds (# s/day) = 60.*60.*24. */
#define degTOr	(1.745329252e-2)	/* 2.0*PI/360.0 convert degrees to radians */

/*--------------------
 *  Global Constants
 *--------------------*/
#define	FLAflag	(9999)		/* used in FLAindex, indicates no freezing T's on a given day */
#define TMLEN (100)		/* length of the time stamp */
#define DAYSINYEAR (365)

/*--------------------
 *  Global Functions
 *--------------------*/
#define sq(a)	((a)*(a))
#define mn(a,b) ((a)<(b)?(a):(b))
#define mx(a,b) ((a)>(b)?(a):(b))
#define rnd(a)  ((fmod(a,1.0)>(0.5)?((double)((int)(a)+1)):((double)((int)(a)))))

/*-------------------
 *  Time Parameters
 *-------------------*/
#define 	maxepoch (110)		/* max number of epochs to run */
int		nepochs, ep;
int		yr, total_yr;
int		nyears[maxepoch], syear[maxepoch];
int		tblstart[maxepoch], tblend[maxepoch];
char		timestep[2];

/*---------------------------------
 *  Sediment Transport Parameters
 *---------------------------------*/
#define	maxgrn	(10)				/* maximum number of grain sizes */
int		ngrain;						/* number of grain sizes */
double		grainpct[maxgrn][maxepoch];	/* % of each grain size, HARDWIRED */

/*--------------------
 *  Event Parameters
 *--------------------*/ 
int		eventcounter;								/* keeps track of the events per epoch */
long		*numberday;									/* parameter to store temperatly which day of the year the event occured */
int		eventsperyear;								/* number of events that occur per year */
int		eventsnr[maxepoch];
double		floodvalue[maxepoch];
int		floodcounter;
int		eventnrflag;								/* indicates is number of events is given or is Qpeak which triggers events is given */

/*--------------------
 *  Delta Parameters
 *--------------------*/
int		noutlet;									/* number of outlets */
int		minnoutlet;									/* min number of outlets in a range */
int		maxnoutlet;									/* max number of outlets in a range */
double 	***outletpct;								/* % of each outlet, HARDWIRED */
double 	sedfilter[maxepoch];						/* filter variable for the delta outlets */
double 	totpercentageQ[maxepoch];
double		**outletpcttotevents;						/* total average percentages per outlet */
int		*nroutlets;
int		outletmodelflag;							/* 1 if delta, 0 for no delta (no multiple outlets) */
int		nooutletpctflag;							/* indicator if Q fractions are given by user or not */
int		noutletflag;
int		steadyoutletpctflag;						/* indicator if Q fractions has to be kept the same or change per event */
double		**outletpctdummy;

/*----------------------------
 *  Sediment Load Parameters
 *----------------------------*/
double threshbed[maxepoch];
#define alphabed  (0.9)
#define trneff (0.1)
#define anglerep (32.21)
double *C;

/*------------------------
 *  Hydraulic Parameters
 *------------------------*/
double	depcof[maxepoch],       deppow[maxepoch];   /* d = (c * Q^f) */
double	velcof[maxepoch],	velpow[maxepoch];   /* v = (k * Q^m)  */
double	widcof[maxepoch],       widpow[maxepoch];   /* w = (a * Q^b) */
double	avgvel[maxepoch];                     /* Avg. river vel. (m/s) */
double	rslope[maxepoch];                     /* Riverbed avg. slope (deg)*/

/*----------------------
 *  Terrain Parameters
 *----------------------*/
double elevbinsize;
int	nhypts[maxepoch];			/* size of hypsometry array per epoch */
double	*areabins;
double	*elevbins;
double	hypspow[maxepoch];
double	**hypsarea;
double  **hypselev;
double	basinlength[maxepoch];   /* River basin length (meters) */
double	maxalt[maxepoch];		 /* Now computed in hydroreadhypsom.c  */
double	totalarea[maxepoch];     /* Now computed in hydroreadhypsom.c  */
double	basin_relief[maxepoch];  /****  REPLACE maxalt ??  ****/
double	basin_area[maxepoch];    /****  REPLACE totalarea ?? ****/

/*---------------------------------
 *  General Hydrologic Parameters
 *---------------------------------*/
int	*distbins;				/* days to discharge a bin */
int	exceedflood;			/* flag for whether maxflood exceeded */
int	floodtry;
double	maxflood;				/* theoretical max flood size */
double	Rvol[maxepoch];			/* storage capacity of lake/reservoir */
double Ralt[maxepoch];			/* alitude of the lake/reservoir */
double	Rarea[maxepoch];		/* drainage area above the reservoir */
double TE[maxepoch];			/* Trapping efficiency */
double TEsubbasin[maxepoch];
char	Rparamcheck[maxepoch];	/* indicator if alt. or drainage area is used as input */	
double	alphac[maxepoch];    /*  OBSOLETE ?? */
double	betac[maxepoch];     /*  OBSOLETE ?? */
double	rhosed[maxepoch];    /***  HARDWIRE  ***/
double	rhowater[maxepoch];  /***  HARDWIRE  ***/

/*------------------------
 *  Rainfall Parameters
 *------------------------*/
double	alphag[maxepoch];    	/***  HARDWIRE  ***/
double	betag[maxepoch];     	/***  HARDWIRE  ***/
double	pcr[maxepoch];		/***  HARDWIRE  ***/
double	pmax[maxepoch];		/***  HARDWIRE  ***/
double	MPrain;

/*--------------------------
 *  Groundwater Parameters
 *--------------------------*/
double	Ko[maxepoch];                         /* sat. hydr. cond. (mm/day) */
double	alphass[maxepoch];
double	betass[maxepoch];
double	gwinitial;					  /* initial GW storage (m^3) */
double	gwlast;
double	gwmin[maxepoch], gwmax[maxepoch];     /* min/max storage (m^3) */
double	percentgw[maxepoch];			  /* % of snow/ice melt to GW */
double	alphagwe[maxepoch];			  /* evaporation coeff */
double	betagwe[maxepoch];			  /* evaporation exponent */

/*-----------------------------
 *  Snowmelt/Nival Parameters
 *-----------------------------*/
double	dryevap[maxepoch];
double	Meltrate[maxepoch];			/***  HARDWIRE ***/
double lapserate[maxepoch];
double	Msnowstart, Msnowend;
double	MPnival;

/*---------------------------
 *  Glacier Melt Parameters
 *---------------------------*/
int	ELAindex;
double	bigg, smallg, lastarea, initiallastarea;
double	ela, lastela, initiallastela;
double	Gmass;
double	MPglacial;
double bethaexpo, bethaglacier;

/*----------------------------
 *  Random Number Parameters
 *----------------------------*/
#define         maxran	2200
#define         INIT_RAN_NUM_SEED (850)
int     	nran;
double	        *ranarray;
double rmin, rmax;

/*--------------------------
 *  Mass Check Parameters
 *--------------------------*/
#define	masscheck (1e-5)		/* mass balance check (%) */
double	maxerr;
double	totalmass;

/*-------------------------
 *  Geographic Parameters
 *-------------------------*/
double lat, lon;
double alpha3, alpha4, alpha5;
double alpha6, alpha7, alpha8;
double k1,k2;
int Qsbarformulaflag[maxepoch];

/*-----------------------
 *  Rainfile parameters
 *-----------------------*/
int raindatafile;
  
/*---------------------------------------------
 *  Parameters to set Qsbarnew for each epoch
 *---------------------------------------------*/
int setstartmeanQandQs; /* loop counter for each epoch (count to 3) */

/*-------------------------------------------------
 *  Variables to set ASCII write option ON or OFF
 *-------------------------------------------------*/
#define MAXCHAR (5)
#define ON "ON"
#define OFF "OFF"
char asciioutput[MAXCHAR];

/*---------------------------------------------------------
 *  Set the file name and directory parameters + security
 *---------------------------------------------------------*/
#define DUMMY "HYDRO"
char startname[80];
char directory[100];
char chrdump[80];
char commandlinearg[2][20];
int webflag;
int globalparflag;
int lapserateflag, velpowflag, widpowflag;

double  *Qgrandtotaltotoutlet, Qgrandtotaltot, **Qdummy, **Qgrandtotalperepoch;
double  Qsgrandtotaltot, *Qsgrandtotaltotoutlet, Qpeakmax, TEtot, yeartot;
double Qsoutletdummy;
double **Qsbartotoutlet;

