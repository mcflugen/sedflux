#if !defined( MY_SEDFLUX_H )
#define MY_SEDFLUX_H

#include "sed_sedflux.h"
#include "my_processes.h"
#include "bio.h"

/** Global variable that lists all of the process that sedflux will run 

sedflux will cycle through the process in the order that they are listed here.
To add a new process to sedflux, you will need to add an entry to this list.
sedflux will will then automatically cycle through your process.
*/
static Sed_process_init_t my_proc_defs[] =
{
   { "constants"         , init_constants     , run_constants   , destroy_constants } , 
   { "earthquake"        , init_quake         , run_quake       , destroy_quake     } , 
   { "tide"              , init_tide          , run_tide        , destroy_tide      } , 
   { "sea level"         , init_sea_level     , run_sea_level   , destroy_sea_level } ,
   { "storms"            , init_storm         , run_storm       , destroy_storm     } ,
   { "river"             , init_river         , run_river       , destroy_river     } ,
   { "erosion"           , init_erosion       , run_erosion     , destroy_erosion   } ,
   { "avulsion"          , init_avulsion      , run_avulsion    , destroy_avulsion  } ,

   /* A new process */
   { "new process"       , init_new_process   , run_new_process , destroy_new_process } ,

   /* The rest of the processes */
   { "bedload dumping"   , init_bedload       , run_bedload     , destroy_bedload     } ,
   { "plume"             , init_plume         , run_plume       , destroy_plume       } ,
   { "diffusion"         , init_diffusion     , run_diffusion   , destroy_diffusion   } ,
   { "xshore"            , init_xshore        , run_xshore      , destroy_xshore      } ,
   { "squall"            , init_squall        , run_squall      , destroy_squall      } ,
   { "bioturbation"      , bio_init           , bio_run         , bio_destroy } ,
   { "compaction"        , NULL               , run_compaction  , NULL                } ,
   { "flow"              , init_flow          , run_flow        , destroy_flow        } ,
   { "isostasy"          , init_isostasy      , run_isostasy    , destroy_isostasy    } ,
   { "subsidence"        , init_subsidence    , run_subsidence  , destroy_subsidence  } ,
   { "data dump"         , init_data_dump     , run_data_dump   , destroy_data_dump   } ,
   { "failure"           , init_failure       , run_failure     , destroy_failure     } ,
   { "measuring station" , init_met_station   , run_met_station , destroy_met_station } ,
   { "bbl"               , init_bbl           , run_bbl         , destroy_bbl         } ,
   { "cpr"               , init_cpr           , run_cpr         , destroy_cpr         } ,

   { "hypopycnal plume"  , init_plume_hypo    , run_plume_hypo  , destroy_plume_hypo  } ,
   { "inflow"            , init_inflow        , run_plume_hyper_inflow , destroy_inflow     } ,
   { "sakura"            , init_inflow        , run_plume_hyper_sakura , destroy_inflow     } ,

   { "debris flow"       , init_debris_flow   , run_debris_flow , destroy_debris_flow } ,
   { "slump"             , NULL               , run_slump       , NULL                } ,

   { NULL }
};

static Sed_process_family my_proc_family[] =
{
   { "failure" , "turbidity current" } ,
   { "failure" , "debris flow"       } ,
   { "failure" , "slump"             } ,
   { "plume"   , "hypopycnal plume"  } ,
   { "plume"   , "inflow"            } ,
   { "plume"   , "sakura"            } ,
   { NULL }
};

static Sed_process_check my_proc_checks[] =
{
   {"plume"           , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"bedload dumping" , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"bbl"             , NULL         , SED_PROC_UNIQUE|SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"river"           , NULL         ,                 SED_PROC_ACTIVE|SED_PROC_ALWAYS } ,
   {"earthquake"      , NULL         , SED_PROC_UNIQUE } ,
   {"storms"          , NULL         , SED_PROC_UNIQUE } ,
   {"failure"         , "earthquake" , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"squall"          , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"xshore"          , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   {"diffusion"       , "storms"     , SED_PROC_ACTIVE_PARENT|SED_PROC_SAME_INTERVAL } ,
   { NULL }
};

#endif /* MY_SEDFLUX_H */


