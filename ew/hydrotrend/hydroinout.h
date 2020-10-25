/*
 *  HydroInOut.h
 *
 *  Contains fid's, filenames and the header string.
 *  See variable descriptions at the end.
 *
 *  Author:    M.D. Morehead  (June-July 1998)
 *  Author2:   S.D. Peckham   (January 2002)
 *  Author3:   A.J. Kettner   (September-October 2002) (February 2003)
 *
 */

#define dbg (1)
#define maxepochd           (110)      /* Also defined in hydroclimate.h and in hydroparams.h*/
#define fnameinput          "HYDRO_INPUT/HYDRO.IN"
#define fnameinputext       ".IN"
#define fnameq              ".Q"
#define fnameqs             ".QS"
#define fnametrend1         ".TRN1"
#define fnametrend2         ".TRN2"
#define fnametrend3         ".TRN3"
#define fnamestat           ".STAT"
#define fnamedis            ".DIS"
#define fnameconvdis        ".CONVDIS"
#define fnamehyps           "HYDRO_INPUT/HYDRO"
#define fnamehypsext        ".HYPS"
#define fnamelog            ".LOG"
#define fnamelapserate      "HYDRO_PROGRAM_FILES/HYDRO_LAPSERATE.LUT"
#define fnameinputgw_r      "HYDRO_INPUT/HYDRO.CLIMATE"
#define fnameclimateext     ".CLIMATE"
#define fidasc              "ASCII.VWD"
#define fidasc1             "ASCII.Q"
#define fidasc2             "ASCII.CSQS"
#define fidasc3             "ASCII.QBAVG"
#define fidasc4             "ASCII.CS"
#define fidasc5             "ASCII.QSAVG"
#define fidqnivalqice       "ASCII.QNIVAL_ICE"
#define MAXCH               (300)
#define OUTPUT_DIR          "/home/ftp/pub/forHydrousers/"
#define INPUTSTRING         "/home/ftp/incoming/toHydrotrend/"

FILE*    fidinput;
FILE*    fidq;
FILE*    fidqs;
FILE*    fidtrend1;
FILE*    fidtrend2;
FILE*    fidtrend3;
FILE*    fidstat;
FILE*    fiddistot;
FILE**    fiddis;
FILE*    fidconvdistot;
FILE**    fidconvdis;
FILE*    fidinputgw_r;
FILE**    fidhyps;
FILE*    fidlog;
FILE*    fidlapserate;
FILE*    outp, *outp1, *outp2, *outp3, *outp4, *outp5;
FILE*    outpnival_ice;

char title[maxepochd][121];
char moname[12][4];
char ffnameinput[MAXCH];
char ffnameq[MAXCH];
char ffnameqs[MAXCH];
char ffnametrend1[MAXCH];
char ffnametrend2[MAXCH];
char ffnametrend3[MAXCH];
char ffnamestat[MAXCH];
char ffnamedis[MAXCH];
char ffnamedistot[MAXCH];
char ffnameconvdis[MAXCH];
char ffnameconvdistot[MAXCH];
char ffnamehyps[MAXCH];
char ffnameinputgw_r[MAXCH];
char ffnamelog[MAXCH];
char ffidasc[MAXCH];
char ffidasc1[MAXCH];
char ffidasc2[MAXCH];
char ffidasc3[MAXCH];
char ffidasc4[MAXCH];
char ffidasc5[MAXCH];
char ffidnival_ice[MAXCH];

/*
 * Variable     Def.Location    Type    Units   Usage
 * --------     ------------    ----    -----   -----
 *
 * dbg          hydroinout.h    define  -   debug and log file flag
 * ffnameq[]            hydroinout.h    char    -       char. array to set filename + filepath
 * ffnameqs[]           hydroinout.h    char    -       char. array to set filename + filepath
 * ffnametrend1[]       hydroinout.h    char    -       char. array to set filename + filepath
 * ffnametrend2[]       hydroinout.h    char    -       char. array to set filename + filepath
 * ffnametrend3[]       hydroinout.h    char    -       char. array to set filename + filepath
 * ffnamedis[]          hydroinout.h    char    -       char. array to set filename + filepath
 * ffnamelog[]          hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc[]            hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc1[]           hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc2[]           hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc3[]           hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc4[]           hydroinout.h    char    -       char. array to set filename + filepath
 * ffidasc5[]           hydroinout.h    char    -       char. array to set filename + filepath
 * fiddis       hydroinout.h    FILE    -   binary discharge and sedload file id
 * fidhyps      hydroinout.h    FILE    -   input hypsometric integral file id
 * fidinput     hydroinout.h    FILE    -   input data file id
 * fidlog       hydroinout.h    FILE    -   log file id
 * fidq         hydroinout.h    FILE    -   output Q's table file id
 * fidqs        hydroinout.h    FILE    -   output Qs's table file id
 * fidtrend1        hydroinout.h    FILE    -   annual trend file #1 id
 * fidtrend2        hydroinout.h    FILE    -   annual trend file #2 id
 * fidtrend3            hydroinout.h    FILE    -   annual trend file #3 id
 * outp                 hydroinout.h    FILE    -       output daily vel, wid en dep file id
 * outp1                hydroinout.h    FILE    -       output daily Q file id
 * outp2                hydroinout.h    FILE    -       output daily Cs and Qs file id
 * outp3                hydroinout.h    FILE    -       output daily Qb average file id
 * outp4                hydroinout.h    FILE    -       output daily Cs file id
 * outp5                hydroinout.h    FILE    -       output daily Qs average file id
 * fnamedis     hydroinout.h    define  -   binary discharge and sedload file name
 * fnamehyps            hydroinout.h    define  -   input hypsometric integral file name
 * fnameinput           hydroinout.h    define  -   input data file name
 * fnamelog     hydroinout.h    define  -   log file name
 * fnameq       hydroinout.h    define  -   output Q's table file name
 * fnameqs      hydroinout.h    define  -   output Qs's table file name
 * fnametrend1          hydroinout.h    define  -   annual trend file #1 name
 * fnametrend2          hydroinout.h    define  -   annual trend file #2 name
 * fnametrend3          hydroinout.h    define  -   annual trend file #3 name
 * fidasc               hydroinout.h    define  -       ascii file   name for daily vel, wid en dep
 * fidasc1              hydroinout.h    define  -       ascii file 1 name for daily Q
 * fidasc2              hydroinout.h    define  -       ascii file 2 name for daily Cs and Qs
 * fidasc3              hydroinout.h    define  -       ascii file 3 name for daily Qb average
 * fidasc4              hydroinout.h    define  -       ascii file 4 name for daily Cs
 * fidasc5              hydroinout.h    define  -       ascii file 5 name for daily Qs average
 * MAXCH                hydroinout.h    define  -       maximum # of char. to set file name + path
 * maxepoche        hydroinout.h    define  -   maximum # of epochs to run
 * moname[][]           hydroinout.h    char    -   month name
 * title[][]            hydroinout.h    char    -   user specified text identifier
 *
 */

