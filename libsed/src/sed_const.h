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

#if !defined( SED_CONST_H )
# define SED_CONST_H

// Name : sedflux constants - some commonly used constants in the sedflux
//        environment.
//
// Synopsis :
//
// #define           PROGRAM_NAME
// #define           PROGRAM_MAJOR_VERSION
// #define           PROGRAM_MINOR_VERSION
// #define           S_MAJOR_VERSION
// #define           S_MINOR_VERSION
// #define           S_MICRO_VERSION
// #define           S_CHECK_VERSION
// #define           S_SECONDS_PER_HOUR
// #define           S_SECONDS_PER_DAY
// #define           S_SECONDS_PER_YEAR
// #define           S_DAYS_PER_YEAR
// #define           S_RADS_PER_DEGREE
// #define           S_GRAVITY
// #define           S_RHO_WATER
// #define           S_RHO_GRAIN
// #define           S_MU_WATER
// #define           S_ETA_WATER
// #define           S_GAMMA_WATER
// #define           S_GAMMA_GRAIN
// #define           S_VELOCITY_IN_WATER
// #define           S_VELOCITY_IN_ROCK
// #define           FRAC                             ( a )
// #define           SWAP                             ( a,
//                                                      b,
//                                                      a_type )
// #define           S_ADDBINS
// #define           S_PROPERTY_BULK_DENSITY
// #define           S_PROPERTY_GRAIN_SIZE
// #define           S_PROPERTY_FACIES
// #define           S_PROPERTY_AGE
// #define           S_PROPERTY_VELOCITY

#define PROGRAM_NAME "sedflux"
#define PROGRAM_MAJOR_VERSION "2"
#define PROGRAM_MINOR_VERSION "0"
#define PROGRAM_MICRO_VERSION "9"

#define S_MAJOR_VERSION (2)
#define S_MINOR_VERSION (0)
#define S_MICRO_VERSION (9)

#define S_CHECK_VERSION(major,minor,micro)    \
    (S_MAJOR_VERSION > (major) || \
     (S_MAJOR_VERSION == (major) && S_MINOR_VERSION > (minor)) || \
     (S_MAJOR_VERSION == (major) && S_MINOR_VERSION == (minor) && \
      S_MICRO_VERSION >= (micro)))

#define S_MPS_PER_KNOT      (.51444)
#define S_SECONDS_PER_HOUR  (3600.)
#define S_SECONDS_PER_DAY   (86400.)
#define S_DAYS_PER_SECOND   (1.157407407407407e-5)
#define S_SECONDS_PER_YEAR  (31536000.)
#define S_DAYS_PER_YEAR     (365.)
#define S_YEARS_PER_DAY     (0.00273972602740)
#define S_RADS_PER_DEGREE   (3.14159/180.)

#define S_GRAVITY           (9.81)          // Acceleration due to gravity (m/s/s)
#define S_RHO_SEA_WATER     (1030.)         // Density of sea water (kg/m^3)
#define S_RHO_FRESH_WATER   (1000.)         // Density of fresh water (kg/m^3)
#define S_RHO_WATER         S_RHO_SEA_WATER
#define S_ETA_WATER         (0.0014e-3)     // Kinematic viscosity of clear water (m^2/s)
#define S_MU_WATER          (0.0014)        // Dynamic viscosity of water (kg/m/s)
#define S_SALINITY_SEA      (35)            // Salinity of the ocean (psu)
#define S_GAMMA_WATER       (10000.)        // Unit weight of water (N/m^3)
#define S_GAMMA_GRAIN       (26500.)        // Unit weight of closely compacted sediment (N/m^3)
#define S_VELOCITY_IN_WATER (1500.)         // Speed of sound in water (m/s)
#define S_VELOCITY_IN_ROCK  (5230.)         // Speed of sound in rock (m/s)
#define S_RHO_QUARTZ        (2650.)         // Density of quartz (kg/m^3)
#define S_RHO_GRAIN         (S_RHO_QUARTZ)  // Maximum sediment density (kg/m^3)
#define S_RHO_MANTLE        (3300.)         // Density of the mantle (kg/m^3)

typedef enum
{
   UNITS_MKS,
   UNITS_CGS,
   UNITS_IMPERIAL
}
Sed_units;

typedef struct
{
   double gravity;
   double rho_sea_h2o;
   double rho_h2o;
   double eta_h2o;
   double mu_h2o;
   double salinity;
   double rho_quartz;
   double rho_mantle;
}
Sed_constants;

#define SED_INIT_CONSTANTS { S_GRAVITY , S_RHO_SEA_WATER , S_RHO_FRESH_WATER , S_MU_WATER , S_ETA_WATER , S_SALINITY_SEA , S_RHO_MANTLE }

#define years_to_secs( a ) ( (a)*S_SECONDS_PER_YEAR )
#define secs_to_years( a ) ( (a)/S_SECONDS_PER_YEAR )

#ifndef FRAC
# define FRAC(a) ( (a) - (int)(a) )
#endif
#ifndef SWAP
# define SWAP(a,b,a_type) { a_type temp=(a); (a)=(b); (b)=temp; }
#endif

#ifndef S_ADDBINS
# define S_ADDBINS (16)
#endif

// Define the facies.
#define S_FACIES_NOTHING     (0)
#define S_FACIES_BEDLOAD     (1<<0)
#define S_FACIES_PLUME       (1<<1)
#define S_FACIES_DEBRIS_FLOW (1<<2)
#define S_FACIES_TURBIDITE   (1<<3)
#define S_FACIES_DIFFUSED    (1<<4)
#define S_FACIES_RIVER       (1<<5)
#define S_FACIES_WAVE        (1<<6)
#define S_FACIES_ALONG_SHORE (1<<7)

#define S_NORTH_EDGE ( 1<<0 )
#define S_WEST_EDGE  ( 1<<1 )
#define S_SOUTH_EDGE ( 1<<2 )
#define S_EAST_EDGE  ( 1<<3 )

#define DEFAULT_SEDIMENT_FILE { \
"--- 'Grain 1 (bedload)' ---                                                 ",\
"grain size (microns):       200                                             ",\
"grain density (kg/m^3):     2625                                            ",\
"saturated density (kg/m^3): 1850                                            ",\
"minimum void ratio (-):     .30                                             ",\
"plastic index (-):          .1                                              ",\
"diffusion coefficient (-):  .25                                             ",\
"removal rate (1/day):       50                                              ",\
"consolidation coefficient (m^2/yr): 100000                                  ",\
"compaction coefficient (-): 0.0000000368                                    ",\
"--- 'Grain 2' ---                                                           ",\
"grain size (microns):       100                                             ",\
"grain density (kg/m^3):     2600                                            ",\
"saturated density (kg/m^3): 1800                                            ",\
"minimum void ratio (-):     .2                                              ",\
"plastic index (-):          .2                                              ",\
"diffusion coefficient (-):  .25                                             ",\
"removal rate (1/day):       16.8                                            ",\
"consolidation coefficient (m^2/yr): 10000                                   ",\
"compaction coefficient (-): 0.00000005                                      ",\
"--- 'Grain 3' ---                                                           ",\
"grain size (microns):       40                                              ",\
"grain density (kg/m^3):     2550                                            ",\
"saturated density (kg/m^3): 1750                                            ",\
"minimum void ratio (-):     .15                                             ",\
"plastic index (-):          .3                                              ",\
"diffusion coefficient (-):  .5                                              ",\
"removal rate (1/day):       9                                               ",\
"consolidation coefficient (m^2/yr): 1000                                    ",\
"compaction coefficient (-): 0.00000007                                      ",\
"--- 'Grain 4' ---                                                           ",\
"grain size (microns):       10                                              ",\
"grain density (kg/m^3):     2500                                            ",\
"saturated density (kg/m^3): 1700                                            ",\
"minimum void ratio (-):     .1                                              ",\
"plastic index (-):          .4                                              ",\
"diffusion coefficient (-):  .75                                             ",\
"removal rate (1/day):       3.2                                             ",\
"consolidation coefficient (m^2/yr): 100                                     ",\
"compaction coefficient (-): 0.00000008                                      ",\
"--- 'Grain 5' ---                                                           ",\
"grain size (microns):       1                                               ",\
"grain density (kg/m^3):     2450                                            ",\
"saturated density (kg/m^3): 1650                                            ",\
"minimum void ratio (-):     .05                                             ",\
"plastic index (-):          .5                                              ",\
"diffusion coefficient (-):  1.                                              ",\
"removal rate (1/day):       2.4                                             ",\
"consolidation coefficient (m^2/yr): 10                                      ",\
"compaction coefficient (-): 0.000000368                                     ",\
NULL }

#define DEFAULT_HYDRO_INLINE_FILE { \
"Number of grain sizes:                    5                                 ",\
"Event duration:                           .25y                              ",\
"Bedload (kg/s):                           100.0                             ",\
"Suspended load concentrations (kg/m^3):   1.0,      1.0,      1.0,      1.0 ",\
"River velocity (m/s):                     1                                 ",\
"River width (m):                          100.0                             ",\
"River depth (m):                          2.0                               ",\
NULL }


#endif

