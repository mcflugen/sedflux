#ifndef HYDROTREND_H_
#define HYDROTREND_H_

int hydrocommandline(int *argc, char **argv);
int hydrosetparams();
int hydrosecurityinputcheck();
int hydroreadinput();
int hydroreadhypsom();
int hydroreadclimate(gw_rainfall_etc* gw_rain);
int hydrosetglobalpar();
int hydrocheckinput();
int hydroopenfiles();
int hydrosetgeoparams( gw_rainfall_etc* gw_rain );
int hydrorandom();
int hydroshoulder();
int hydroclimate(gw_rainfall_etc* gw_rain);
int hydroweather(gw_rainfall_etc* gw_rain);
int hydrohypsom();
int hydroglacial();
int hydrosnow();
int hydrorain();
int hydrosumflow();
int hydromaxevents();
int hydrosedload ( gw_rainfall_etc* gw_rain );
int hydrooutput();
int hydroprinttable();
int hydroprintannual();
int hydrocalqsnew();
int hydroprintstat();
int hydroswap();
int hydrooutletfraction(int x);
int hydrosetnumberoutlet(int x);
int hydroqfractionshuffle( int k );
void hydroallocmemoutlet(int ep);
void hydroallocmemoutlet1(int ep);
void hydrofreememoutlet(int j);
void hydrofreememoutlet1(int ep);
int hydroshuffle(int dvals[31],int mnth);
int hydroexpdist(double pvals[31],int mnth);

#endif

