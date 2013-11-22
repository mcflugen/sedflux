#include <stdio.h>
#include <glib.h>

const char* _default_init_file[] = {
"[ global ]",
"margin name:            Earth",
"vertical resolution:    .5",
"x resolution:           150",
"y resolution:           150",
"bathymetry file:        mars_bathy_150m.csv",
"sediment file:          earth_sediment.kvf",
"",
"[ epoch ]",
"number:           1",
"duration:         200y",
"time step:        1y",
"process file:     earth_epoch.kvf",
NULL
};

const char* _default_2d_bathy_file[] = {
"# Elevations are in meters. ",
"# Rows are constant x",
"# Columns are constant y",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
"0; -20; -40; -60; -80; -100; -120; -140; -160; -180",
NULL
};

const char* _default_1d_bathy_file[] = {
"# First column is cross-shore distance (m)",
"# Second column is elevation (m)",
"0; 0",
"100000; -180",
NULL
};

const char* _default_hydro_inline_file[] = {
"[ 'Season 1' ]",
"Duration (y): .25y",
"Bedload (kg/s): 100.0",
"Suspended load concentration (kg/m^3): 1.0, 2.5, 2.5, 2.5",
"Velocity (m/s): 2.",
"Width (m): 125.0",
"Depth (m): 2.0",
"[ 'Season 2' ]",
"Duration (y): 3m",
"Bedload (kg/s): 200.0",
"Suspended load concentration (kg/m^3): 2.5, 2.5, 2.5, 2.5",
"Velocity (m/s): 2.",
"Width (m): 125.0",
"Depth (m): 2.0",
"[ 'Season 3' ]",
"Duration (y): .25y",
"Bedload (kg/s): 300.0",
"Suspended load concentration (kg/m^3): 3.0, 2.5, 2.5, 2.5",
"Velocity (m/s): 2.",
"Width (m): 125.0",
"Depth (m): 2.0",
"[ 'Season 4' ]",
"Duration (y): 3m",
"Bedload (kg/s): 400.0",
"Suspended load concentration (kg/m^3): 4.0, 2.5, 2.5, 2.5",
"Velocity (m/s): 2.",
"Width (m): 125.0",
"Depth (m): 2.0",
NULL
};

const char* _default_sediment_file[] = {
"--- 'Grain 1 (bedload)' ---",
"grain size (microns):       200",
"grain density (kg/m^3):     2625",
"saturated density (kg/m^3): 1850",
"minimum void ratio (-):     .30",
"plastic index (-):          .1",
"diffusion coefficient (-):  .25",
"removal rate (1/day):       50",
"consolidation coefficient (m^2/yr): 100000",
"compaction coefficient (-): 0.0000000368",
"--- 'Grain 2' ---",
"grain size (microns):       100",
"grain density (kg/m^3):     2600",
"saturated density (kg/m^3): 1800",
"minimum void ratio (-):     .2",
"plastic index (-):          .2",
"diffusion coefficient (-):  .25",
"removal rate (1/day):       16.8",
"consolidation coefficient (m^2/yr): 10000",
"compaction coefficient (-): 0.00000005",
"--- 'Grain 3' ---",
"grain size (microns):       40",
"grain density (kg/m^3):     2550",
"saturated density (kg/m^3): 1750",
"minimum void ratio (-):     .15",
"plastic index (-):          .3",
"diffusion coefficient (-):  .5",
"removal rate (1/day):       9",
"consolidation coefficient (m^2/yr): 1000",
"compaction coefficient (-): 0.00000007",
"--- 'Grain 4' ---",
"grain size (microns):       10",
"grain density (kg/m^3):     2500",
"saturated density (kg/m^3): 1700",
"minimum void ratio (-):     .1",
"plastic index (-):          .4",
"diffusion coefficient (-):  .75",
"removal rate (1/day):       3.2",
"consolidation coefficient (m^2/yr): 100",
"compaction coefficient (-): 0.00000008",
"--- 'Grain 5' ---",
"grain size (microns):       1",
"grain density (kg/m^3):     2450",
"saturated density (kg/m^3): 1650",
"minimum void ratio (-):     .05",
"plastic index (-):          .5",
"diffusion coefficient (-):  1.",
"removal rate (1/day):       2.4",
"consolidation coefficient (m^2/yr): 10",
"compaction coefficient (-): 0.000000368",
NULL
};

const char* _default_process_file[] = {
"active: yes",
"logging: no",
"repeat interval: always",
NULL
};

/*
gchar*
get_config_text (const gchar* file)
{
  if (g_ascii_strcasecmp (file, "config")==0)
    return g_strjoinv ("\n", (gchar**)_default_init_file);
  if (g_ascii_strcasecmp (file, "bathy-1d")==0)
    return g_strjoinv ("\n", (gchar**)_default_1d_bathy_file);
  if (g_ascii_strcasecmp (file, "bathy-2d")==0)
    return g_strjoinv ("\n", (gchar**)_default_2d_bathy_file);
  if (g_ascii_strcasecmp (file, "hydro")==0)
    return g_strjoinv ("\n", (gchar**)_default_hydro_inline_file);
  if (g_ascii_strcasecmp (file, "sediment")==0)
    return g_strjoinv ("\n", (gchar**)_default_sediment_file);
  if (g_ascii_strcasecmp (file, "process")==0)
    return g_strjoinv ("\n", (gchar**)_default_sediment_file);
  else
    return NULL;
}
*/
