/*
 *  HydroSwap.c
 *
 *  swab - Convert between big-endian and little-endian
 *  file formats. It creates a second binary file which
 *  simulair to the created output file of HYDROTREND but
 *  then in a swabbed order. So if you are running hydrotrend on a PC
 *  you can read the swabbed binary file on an Unix platform and
 *  visa versa.
 *
 *  Author:     A.J. Kettner    (October 2002) (February-April 2003)
 *
 *
 * Variable		Def.Location	Type	Units	Usage
 * --------		------------	----	-----	-----
 * comLen		HydroSwap.c	int	-	length of the title string
 * err  		various 	int	-	error flag, halts program
 * i			various		int	-	temporary loop counter
 * nYears		HydroSwap.c	int	-	total # of output years
 * recperyear   	HydroSwap.c	int	-	# of output records per year
 * start                HydroSwap.c     int     -       Start for filepointer after reading part of Header
 * word                 HydroSwap.c     int     -       Number of bytes of the input file
 * dataWord             HydroSwap.c     char    -       Holding byte info of each byte in a lope
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hydroinout.h"
#include "hydroparams.h"
#include "hydroclimate.h"
#include "hydrotimeser.h"
#include "hydroalloc_mem.h"

#define DEFAULT_WORD 4

/*----------------------
 *  Start main program
 *----------------------*/
int hydroswap()
{

/*-------------------
 *  Local Variables
 *-------------------*/
int i, err, verbose, comLen, nYears,p;
int word;
int start;
static int recperyear;
char *dataWord;
char dummystring[300];
word = DEFAULT_WORD;
err = 0;
verbose = 0;

/*------------------------
 *  Opening binary files
 *------------------------*/
if (verbose) printf("Opening %s... \n",ffnamedistot);
if ( (fiddistot = fopen(ffnamedistot,"rb")) == NULL) {
   fprintf(stderr, "  HydroSwap ERROR: Unable to open the discharge file %s \n",ffnamedistot);
   err = 1;
}
strcpy(ffnameconvdistot,startname);
strcat(ffnameconvdistot,fnameconvdis);
if (verbose) printf("Opening %s... \n",ffnameconvdistot);
if ( (fidconvdistot = fopen(ffnameconvdistot,"wb")) == NULL) {
   fprintf(stderr, "  HydroSwap ERROR: Unable to open the convdischarge file %s \n",ffnameconvdistot);
   err = 1;
}

if ( outletmodelflag == 1 ){
	fidconvdis = allocate_1d_F( maxnoutlet );	
	for (p=0; p<maxnoutlet; p++){
		strcpy(ffnamedis,startname);
		sprintf(dummystring,"%sOUTLET%d",ffnamedis,p+1);
		strcpy(ffnamedis,dummystring);
		strcat(ffnamedis,fnamedis);
		if (verbose) printf("Opening %s... \n",ffnamedis);
		if ( (fiddis[p] = fopen(ffnamedis,"rb")) == NULL) {
			fprintf(stderr, "  HydroSwap ERROR: Unable to open the discharge file %s \n",ffnamedis);
			err = 1;
		}
		strcpy(ffnameconvdis,startname);
		sprintf(dummystring,"%sOUTLET%d",ffnameconvdis,p+1);
		strcpy(ffnameconvdis,dummystring);
		strcat(ffnameconvdis,fnameconvdis);
		if (verbose) printf("Opening %s... \n",ffnameconvdis);
		if ( (fidconvdis[p] = fopen(ffnameconvdis,"wb")) == NULL) {
		fprintf(stderr, "  HydroSwap ERROR: Unable to open the convdischarge file %s \n",ffnameconvdis);
		err = 1;
		}
	}
}

/*-------------------
 *  Allocate memory
 *-------------------*/
   dataWord = (char*)malloc(sizeof(char)*word);

/*------------------------------------
 *  Setting variables for the header
 *------------------------------------*/
   if(      timestep[0] == 'd' ) recperyear = 365;
   else if( timestep[0] == 'm' ) recperyear = 12;
   else if( timestep[0] == 's' ) recperyear = 4;
   else recperyear = 1;

   nYears	= syear[nepochs-1]+nyears[nepochs-1]-syear[0];
   comLen	= strlen(title[0])-1;
   start = (word) +comLen;

/*--------------------------------------------
 *  Swapping parts of header + rest datafile
 *--------------------------------------------*/

		fread(dataWord,1,word,fiddistot);
		for (i=word-1;i>=0;i--)
			fwrite(&dataWord[i],1,1,fidconvdistot);
		fwrite(    title[0], sizeof(char), comLen, fidconvdistot);
		fseek(fiddistot,start,SEEK_SET);
		while ( fread(dataWord,1,word,fiddistot)==word )
			for (i=word-1;i>=0;i--)
				fwrite(&dataWord[i],1,1,fidconvdistot);

	if (outletmodelflag == 1)
		for (p=0; p<maxnoutlet; p++){
			fread(dataWord,1,word,fiddis[p]);
			for (i=word-1;i>=0;i--)
				fwrite(&dataWord[i],1,1,fidconvdis[p]);
			fwrite(    title[0], sizeof(char), comLen, fidconvdis[p]);
			fseek(fiddis[p],start,SEEK_SET);
			while ( fread(dataWord,1,word,fiddis[p])==word )
				for (i=word-1;i>=0;i--)
					fwrite(&dataWord[i],1,1,fidconvdis[p]);
		}
	
/*-------------------
 *  Close files
 *------------------*/
	fclose(fiddistot);
	fclose(fidconvdistot);
 	if (outletmodelflag == 1)
		for (p=0; p<maxnoutlet; p++){
			fclose(fiddis[p]);
			fclose(fidconvdis[p]);
		}
	return(err);
}  /* end of HydroSwap */

