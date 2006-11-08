/*
 *  HydroOpenFiles.c
 *
 *  Opens the input/output files for HydroTrend.
 *
 *  Author:     M.D. Morehead   (June 1998)
 *  Author2:    S.D. Peckham    (January  2002)
 *  Author3:    A.J. Kettner    (September 2002) (February 2003)
 *
 */

#include <stdio.h>
#include <string.h>
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydroalloc_mem.h"

/*---------------------------
 *  Start of HydroOpenFiles
 *---------------------------*/
int hydroopenfiles()
{
int err, verbose,p;
char dummystring[300];
err = 0;
verbose = 0;

strcpy(ffnameq,startname);
strcat(ffnameq,fnameq);
if (verbose) printf("Opening %s... \n",ffnameq);
if ( (fidq = fopen(ffnameq,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the Q table file %s \n",ffnameq);
   err = 1;
}

strcpy(ffnameqs,startname);
strcat(ffnameqs,fnameqs);
if (verbose) printf("Opening %s... \n",ffnameqs);
if ( (fidqs = fopen(ffnameqs,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the Qs table file %s \n",ffnameqs);
   err = 1;
}

strcpy(ffnametrend1,startname);
strcat(ffnametrend1,fnametrend1);
if (verbose) printf("Opening %s... \n",ffnametrend1);
if ( (fidtrend1 = fopen(ffnametrend1,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the trend file %s \n",ffnametrend1);
   err = 1;
}

strcpy(ffnametrend2,startname);
strcat(ffnametrend2,fnametrend2);
if (verbose) printf("Opening %s... \n",ffnametrend2);
if ( (fidtrend2 = fopen(ffnametrend2,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the trend file %s \n",ffnametrend2);
   err = 1;
}

strcpy(ffnametrend3,startname);
strcat(ffnametrend3,fnametrend3);
if (verbose) printf("Opening %s... \n",ffnametrend3);
if ( (fidtrend3 = fopen(ffnametrend3,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the trend file %s \n",ffnametrend3);
   err = 1;
}

strcpy(ffnamestat,startname);
strcat(ffnamestat,fnamestat);
if (verbose) printf("Opening %s... \n",ffnamestat);
if ( (fidstat = fopen(ffnamestat,"w")) == NULL) {
   fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the trend file %s \n",ffnamestat);
   err = 1;
}

strcpy(ffnamedistot,startname);
sprintf(dummystring,"%s",ffnamedistot);
strcpy(ffnamedistot,dummystring);
strcat(ffnamedistot,fnamedis);
if (verbose) printf("Opening %s... \n",ffnamedistot);
if ( (fiddistot = fopen(ffnamedistot,"wb")) == NULL) {
	fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffnamedistot);
	err = 1;
}

if ( outletmodelflag == 1 )
	fiddis = allocate_1d_F( maxnoutlet );

if ( outletmodelflag == 1  )
	for (p=0; p<maxnoutlet; p++){
		strcpy(ffnamedis,startname);
		sprintf(dummystring,"%sOUTLET%d",ffnamedis,p+1);
		strcpy(ffnamedis,dummystring);
		strcat(ffnamedis,fnamedis);
		if (verbose) printf("Opening %s... \n",ffnamedis);
		if ( (fiddis[p] = fopen(ffnamedis,"wb")) == NULL) {
			fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffnamedis);
			err = 1;
		}
	}

/*-----------------------
 *  Opening ascii files
 *-----------------------*/
if( strncmp(asciioutput,ON,2) == 0){
    strcpy(ffidasc,startname);
    strcat(ffidasc,fidasc);
    if (verbose) printf("Opening %s... \n",ffidasc);
    if ( (outp = fopen(ffidasc,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc);
        err = 1;
    }

    strcpy(ffidasc1,startname);
    strcat(ffidasc1,fidasc1);
    if (verbose) printf("Opening %s... \n",ffidasc1);
    if ( (outp1 = fopen(ffidasc1,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc1);
        err = 1;
    }

    strcpy(ffidasc2,startname);
    strcat(ffidasc2,fidasc2);
    if (verbose) printf("Opening %s... \n",ffidasc2);
    if ( (outp2 = fopen(ffidasc2,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc2);
        err = 1;
    }

    strcpy(ffidasc3,startname);
    strcat(ffidasc3,fidasc3);
    if (verbose) printf("Opening %s... \n",ffidasc3);
    if ( (outp3 = fopen(ffidasc3,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc3);
        err = 1;
    }

    strcpy(ffidasc4,startname);
    strcat(ffidasc4,fidasc4);
    if (verbose) printf("Opening %s... \n",ffidasc4);
    if ( (outp4 = fopen(ffidasc4,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc4);
        err = 1;
    }

    strcpy(ffidasc5,startname);
    strcat(ffidasc5,fidasc5);
    if (verbose) printf("Opening %s... \n",ffidasc5);
    if ( (outp5 = fopen(ffidasc5,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidasc5);
        err = 1;
    }

    strcpy(ffidnival_ice,startname);
    strcat(ffidnival_ice,fidqnivalqice);
    if (verbose) printf("Opening %s... \n",ffidnival_ice);
    if ( (outpnival_ice = fopen(ffidnival_ice,"w")) == NULL) {
        fprintf(stderr, "  HydroOpenFiles ERROR: Unable to open the discharge file %s \n",ffidnival_ice);
        err = 1;
    }
    
}
return(err);
}  /* end of HydroOpenFiles */
