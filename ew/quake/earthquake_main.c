//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <utils/utils.h>

/*** Self Documentation ***/
static char *help_msg[] =
{
"                                                                             ",
" earthquake [options] [parameters]                                           ",
"  generate a random series of earthquakes.                                   ",
"                                                                             ",
" Options                                                                     ",
"  v=yes          : toggle verbose mode. [off]                                ",
"  help=yes       : print this help message.                                  ",
"  info=mercalli  : print info on the Modified Mercalli Intensity Scale.      ",
"  info=omori     : print info on the Omori Seismic Scale.                    ",
"                                                                             ",
" Parameters                                                                  ",
"  n=value        : generate value earthquakes. [100]                         ",
"  dt=value       : find the biggest quake of over value days. [1]            ",
"  mag=value      : magnitude of the median quake is value. [1]               ",
"  units=mercalli : output will be from the Modified Mercalli Intensity Scale.",
"  units=omori    : output will be from the Omori Seismic Scale.              ",
"  units=si       : output will be in units of m/s^2. [default]               ",
"  units=g        : output will be in units of g (here on earth).             ",
"                                                                             ",
" Generate a random series of earthquakes based on the probability density    ",
" function:                                                                   ",
"    p(x) = -log(a) a^x , with a^2 < 1                                        ",
" a is determined by the median quake over the given time step.  a is found   ",
" by:                                                                         ",
"    a = 2^(-1/median_quake)                                                  ",
" or if the mean quake is given,                                              ",
"    a = e^(-1/mean_quake)                                                    ",
" If a time step other than 1 is given, the maximum quake over this number of ",
" time steps is returned.  This is done using the distribution function:      ",
"    P(x) = (1-a^x)^n                                                         ",
" where n is the number of time steps.                                        ",
NULL
};

static char *omori_msg[] =
{
"                                                                             ",
" The Omori Seismic Scale                                                     ",
"                                                                             ",
" The Omori seismic scale is a seven-point scale that relates various         ",
" phenomena to maximum ground acceleration. It is based on the behavior of    ",
" typical Japanese structures and is still widely used in Japan. This         ",
" description is from Building Structures in Earthquake Countries by Alfredo  ",
" Montel (Lippincott, Philadelphia, 1912). Notice that its level I is         ",
" equivalent to a Mercalli intensity of VI.                                   ",
"                                                                             ",
" I.   Maximum Acceleration = 300 mm per sec. per sec.                        ",
"      The shock is rather strong, so much so that it generally induces people",
"      to escape from their houses into the open. The walls of badly          ",
"      constructed brick houses crack slightly and some parquet falls down;   ",
"      ordinary wooden houses are shaken in such a degree that they loudly    ",
"      creak; furniture is overturned; trees are visibly shaken; the water in ",
"      ponds and pools gets turbid, owing to the disturbance of the mud;      ",
"      pendulum clocks stop; some very badly built factory chimneys are       ",
"      damaged.                                                               ",
" II.  Maximum Acceleration = 900 mm per sec. per sec.                        ",
"      The walls in the wooden houses of Japan crack; old wooden houses get   ",
"      slightly out of plumb; the Japanese tombstones and the badly           ",
"      constructed stone lanterns are overturned; in a few cases the flow of  ",
"      the thermal and mineral springs is changed; ordinary factory chimneys  ",
"      are not damaged.                                                       ",
" III. Maximum Acceleration = 1200 mm per sec. per sec.                       ",
"      About one-fourth of the factory chimneys are damaged; badly constructed",
"      brick houses are partially or totally destroyed; some old wooden houses",
"      are destroyed; wooden bridges are slightly damaged; some tombstones and",
"      stone lanterns are overturned; Japanese sliding doors (covered with    ",
"      paper) are broken; the tiles of wooden houses are displaced; some      ",
"      fragments of rocks are detached from the sides of the mountains.       ",
" IV.  Maximum Acceleration = 2000 mm per sec. per sec.                       ",
"      All factory chimneys are ruined; the majority of the ordinary brick    ",
"      houses are partially or totally destroyed; some wooden houses are      ",
"      totally destroyed; the wooden sliding doors are mostly thrust out of   ",
"      their channels; crevices from 2 to 3 inches (5 to 7-1/2 cm) wide appear",
"      in low and soft grounds; here and there the embankments are slightly   ",
"      damaged; wooden bridges are partially destroyed; ordinarily constructed",
"      stone lanterns are overturned.                                         ",
" V.   Maximum Acceleration = 2500 mm per sec. per sec.                       ",
"      All ordinary brick houses are very seriously damaged; about 3 percent  ",
"      of the wooden houses are totally destroyed; some Buddhist temples are  ",
"      ruined; the embankments are badly damaged; the railways are slightly   ",
"      contorted; ordinary tombstones are overturned; brick walls are damaged;",
"      here and there, large fissures from 1 to 2 feet (30 to 60 cm) wide     ",
"      appear along the banks of the watercourses. The water of rivers and    ",
"      ditches is thrown on the banks; the contents of the wells are          ",
"      disturbed; landslides occur.                                           ",
" VI.  Maximum Acceleration = 4000 mm per sec. per sec.                       ",
"      The greater part of the Buddhist temples are ruined; from 50 to 80     ",
"      percent of the wooden houses are totally destroyed; the embankments are",
"      almost destroyed; the roads through paddy fields are ruined and        ",
"      interrupted by fissures in such a degree that traffic by animals or    ",
"      vehicles is impeded; the railways are very much contorted; great iron  ",
"      bridges are destroyed; wooden bridges are partially or totally damaged;",
"      tombstones of solid construction are overturned; fissures some feet    ",
"      wide appear in the soil, and are sometimes accompanied by jets of water",
"      and sand; iron or terra cotta tanks embedded in the ground are mostly  ",
"      destroyed; all lowlying grounds are completely convulsed horizontally  ",
"      as well as vertically in such a degree that sometimes the trees and all",
"      the vegetation on them die off; numerous landslides take place.        ",
" VII. Maximum Acceleration = much more than 4000 mm per sec. per sec.        ",
"      All buildings are completely destroyed except a few wooden             ",
"      constructions; some doors or wooden houses are thrown over distances   ",
"      from 1 to 3 feet; enormous landslides with faults and shears of the    ",
"      ground occur.                                                          ",
"                                                                             ",
NULL
};

static char *mercalli_msg[] =
{
"                                                                             ",
" The Modified Mercalli Intensity Scale                                       ",
"                                                                             ",
" I.    Not felt except by a very few under especially favorable              ",
"       circumstances.                                                        ",
" II.   Felt only by a few persons at rest, especially on upper floors of     ",
"       buildings. Delicately suspended objects may swing.                    ",
" III.  Felt quite noticeably indoors, especially on upper floors of          ",
"       buildings but many people do not recognize it as an earthquake.       ",
"       Standing motor cars may rock slightly. Vibration like passing truck.  ",
"       Duration estimated.                                                   ",
" IV.   During the day felt indoors by many, outdoors by few. At night some   ",
"       awakened. Dishes, windows, and doors disturbed; walls make creaking   ",
"       sound. Sensation like heavy truck striking building. Standing         ",
"       motorcars rock noticeably.                                            ",
" V.    Felt by nearly everyone; many awakened. Some dishes, windows, etc.,   ",
"       broken; a few instances of cracked plaster; unstable objects          ",
"       overturned. Disturbance of trees, poles, and other tall objects       ",
"       sometimes noticed.  Pendulum clocks may stop.                         ",
" VI.   Felt by all; many frightened and run outdoors. Some heavy furniture   ",
"       moved; a few instances of fallen plaster or damaged chimneys. Damage  ",
"       slight.                                                               ",
" VII.  Everybody runs outdoors. Damage negligible in buildings of good design",
"       and construction slight to moderate in well built ordinary structures;",
"       considerable in poorly built or badly designed structures. Some       ",
"       chimneys broken. Noticed by persons driving motor cars.               ",
" VIII. Damage slight in specially designed structures; considerable in       ",
"       ordinary substantial buildings, with partial collapse; great in poorly",
"       built structures. Panel walls thrown out of frame structures. Fall of ",
"       chimneys, factory stacks, columns, monuments, walls. Heavy furniture  ",
"       overturned. Sand and mud ejected in small amounts. Changes in well    ",
"       water. Persons driving motor cars disturbed.                          ",
" IX.   Damage considerable in specially designed structures; well-designed   ",
"       frame structures thrown out of plumb; great in substantial buildings, ",
"       with partial collapse. Buildings shifted off foundations. Ground      ",
"       cracked conspicuously. Underground pipes broken.                      ",
" X.    Some well-built wooden structures destroyed; most masonry and frame   ",
"       structures destroyed with foundations; ground badly cracked. Rails    ",
"       bent. Landslides considerable from river banks and steep slopes.      ",
"       Shifted sand and mud. Water splashed over banks.                      ",
" XI.   Few, if any (masonry), structures remain standing. Bridges destroyed. ",
"       Broad fissures in ground.  Underground pipelines completely out of    ",
"       service. Earth slumps and land slips in soft ground. Rails bent       ",
"       greatly.                                                              ",
" XII.  Damage total. Waves seen on ground surfaces. Lines of sight and level ",
"       distorted. Objects thrown upward into the air.                        ",
"                                                                             ",
NULL
};

#define DEFAULT_N                   (100)
#define DEFAULT_DT                  (1.)
#define DEFAULT_MAG                 (1.)
#define DEFAULT_VERBOSE             0
#define DEFAULT_UNITS               SI_UNITS

#define SI_UNITS       0
#define MERCALLI_UNITS 1
#define OMORI_UNITS    2
#define G_UNITS        3

#define INFO_MERCALLI  0
#define INFO_OMORI     1

double earthquake(double a,double dt);
double convert_accel_to_si(double acceleration);
double convert_accel_to_g(double acceleration);
double convert_accel_to_omori(double acceleration);
double convert_accel_to_mercalli(double acceleration);
double convert_mercalli_to_accel(double mercalli);
double convert_omori_to_accel(double omori);
double convert_g_to_accel(double g);
double convert_si_to_accel(double si);

int main(int argc, char *argv[])
{
   char *unit_vals[] = { "si" , "mercalli" , "omori" , "g" , NULL };
   char *info_vals[] = { "mercalli" , "omori" , NULL };
   int i;
   int n, units, info;
   double dt, mag;
   gboolean verbose;
   double average_quake;
   double acceleration;
   double (*convert_func)(double);
   double (*convert_inv_func)(double);
   Eh_args *args;

   args = eh_opts_init( argc , argv );
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
     eh_exit(-1);

   n       = eh_get_opt_int ( args , "n"     , DEFAULT_N   );
   dt      = eh_get_opt_dbl ( args , "dt"    , DEFAULT_DT  );
   mag     = eh_get_opt_dbl ( args , "mag"   , DEFAULT_MAG );
   verbose = eh_get_opt_bool( args , "v"     , FALSE       );
   units   = eh_get_opt_key ( args , "units" , 0  ,  unit_vals );
   info    = eh_get_opt_key ( args , "info"  , -1 ,  info_vals );
 
   switch ( info )
   {
      case INFO_MERCALLI:
         eh_print_message( stderr , mercalli_msg );
         eh_exit(0);
      case INFO_OMORI:
         eh_print_message( stderr , omori_msg );
         eh_exit(0);
   }

   if ( verbose )
   {
      fprintf(stderr,"Magnitude : %f\n",mag);
      fprintf(stderr,"Dt        : %f\n",dt);
      fprintf(stderr,"n         : %d\n",n);
   }

   switch ( units )
   {
      case SI_UNITS:
         convert_func = &convert_accel_to_si;
         convert_inv_func = &convert_si_to_accel;
         break;
      case MERCALLI_UNITS:
         convert_func = &convert_accel_to_mercalli;
         convert_inv_func = &convert_mercalli_to_accel;
         break;
      case OMORI_UNITS:
         convert_func = &convert_accel_to_omori;
         convert_inv_func = &convert_omori_to_accel;
         break;
      case G_UNITS:
         convert_func = &convert_accel_to_g;
         convert_inv_func = &convert_g_to_accel;
         break;
   }

   if ( strcmp(g_basename(argv[0]),"quakeconvert_to")==0 )
   {
      while ( fscanf(stdin,"%lf",&acceleration)==1 )
         fprintf(stdout,"%f\n",convert_func(acceleration));
      return 0;
   }
   else if ( strcmp(g_basename(argv[0]),"quakeconvert_from")==0 )
   {
      while ( fscanf(stdin,"%lf",&acceleration)==1 )
         fprintf(stdout,"%f\n",convert_inv_func(acceleration));
      return 0;
   }

   average_quake = convert_inv_func(mag);

   for ( i=0 ; i<n ; i++ )
   {
      acceleration = earthquake(exp(-1./average_quake),dt);
      fprintf(stdout,"%f\n",convert_func(acceleration));
   }

   return 0;
}

double convert_accel_to_si(double acceleration)
{
   return acceleration;
}

double convert_si_to_accel(double si)
{
   return si;
}

double convert_accel_to_g(double acceleration)
{
   return acceleration/9.81;
}

double convert_g_to_accel(double g)
{
   return g*9.81;
}

double convert_accel_to_omori(double acceleration)
{
   if ( acceleration <= .3 )
      return 1;
   else if ( acceleration <= .9 )
      return 2;
   else if ( acceleration <= 1.2 )
      return 3;
   else if ( acceleration <= 2. )
      return 4;
   else if ( acceleration <= 2.5 )
      return 5;
   else if ( acceleration <= 4 )
      return 6;
   else
      return 7;
}

double convert_omori_to_accel(double omori)
{
   if ( omori == 1 )
      return .3;
   else if ( omori == 2 )
      return .9;
   else if ( omori == 3 )
      return 1.2;
   else if ( omori == 4 )
      return 2.;
   else if ( omori == 5 )
      return 2.5;
   else
      return 4.;
}

double convert_accel_to_mercalli(double acceleration)
{
   return (log10(acceleration) + 2.5 ) * 3;
}

double convert_mercalli_to_accel(double mercalli)
{
   return pow(10,mercalli/3 - 2.5);
}

int do_help(char *message[])
{
   int i;
   char **p;

   if ( message )
      for ( p=message ; *p ; fprintf(stderr,"%s\n",*p), p++ );
   else
      for (p=message,i=0 ; i<4 ; i++)
         fprintf(stderr,"%s\n",p[i]);

   return 0;
}

