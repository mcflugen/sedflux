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

#define SED_RIVER_PROC_NAME "river"
#define EH_LOG_DOMAIN SED_RIVER_PROC_NAME

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "sed_sedflux.h"
#include "river.h"
#include "utils.h"
#include "processes.h"

Sed_process_info run_river(gpointer ptr,Sed_cube prof)
{
   River_t *data=(River_t*)ptr;
   int i;
   double dt, volume, volume_to_remove, mass_removed;
   double susp_mass, bedload_mass, init_susp_mass, init_bedload_mass;
   Sed_river *new_river;
   Sed_hydro river_data;
   Sed_process_info info = SED_EMPTY_INFO;
   
   if ( prof == NULL )
   {
      if ( data->initialized )
      {
         sed_hydro_file_destroy( data->fp_river );
         data->initialized = FALSE;
      }
      return SED_EMPTY_INFO;
   }

   if ( !data->initialized )
   {
      data->fp_river = sed_hydro_file_new( data->filename , data->type , TRUE );

      new_river = sed_create_river( sed_sediment_env_size() , NULL );
      new_river->river_name = g_strdup( data->river_name );

      // the hinge point is defined in the avulsion process.
      new_river->hinge->x     = 0;
      new_river->hinge->y     = 0;
      new_river->hinge->angle = M_PI_2;

      sed_cube_add_river( prof , new_river );
      data->this_river = sed_cube_river_list( prof );

      data->total_mass = 0;
      data->total_mass_from_river = 0;
      data->initialized = TRUE;
   }

   river_data = sed_hydro_file_read_record( data->fp_river );

   if ( river_data )
   {
      Sed_hydro hydro_data;

      sed_cube_set_river_data( prof , data->this_river , river_data );

      hydro_data = sed_cube_river_data( prof , data->this_river );

      //---
      // Set the cube time step to be that of the river.
      //---
      if ( data->type & (HYDRO_USE_BUFFER|HYDRO_INLINE) )
         sed_cube_set_time_step( prof , 
                                 sed_hydro_duration_in_seconds(hydro_data)/S_SECONDS_PER_YEAR );
      dt = sed_cube_time_step_in_seconds( prof );

      //---
      // Keep a running total of the sediment mass added from the river.
      // This exludes erosion/deposition within the river.
      //---
      data->total_mass_from_river += sed_hydro_total_load( hydro_data );

      init_susp_mass         = sed_hydro_suspended_flux( hydro_data )*dt;
      init_bedload_mass      = sed_hydro_bedload       ( hydro_data )*dt;

      //---
      // Add any eroded sediment to the river.
      //---
      volume = sed_cube_x_res( prof )
             * sed_cube_y_res( prof )
             * sed_cell_thickness( sed_cube_to_add(prof) );
      sed_hydro_add_cell( hydro_data , sed_cube_to_add(prof) , volume );
      sed_cell_clear( sed_cube_to_add(prof) );

      //---
      // Remove any sediment that was deposited within the river.
      //---
      volume_to_remove = sed_cube_x_res( prof )
                       * sed_cube_y_res( prof )
                       * sed_cell_thickness( sed_cube_to_remove(prof) );
      sed_hydro_subtract_cell( hydro_data , sed_cube_to_remove(prof) , volume_to_remove );
      sed_cell_clear( sed_cube_to_remove(prof) );

      susp_mass         = sed_hydro_suspended_flux( hydro_data )*dt;
      bedload_mass      = sed_hydro_bedload       ( hydro_data )*dt;
      data->total_mass += susp_mass + bedload_mass;

      mass_removed     = susp_mass+bedload_mass - (init_susp_mass+init_bedload_mass);

      eh_message( "time         : %f" , sed_cube_age_in_years(prof) );
      eh_message( "duration     : %f" , sed_cube_time_step_in_years(prof) );
      eh_message( "velocity     : %f" , sed_hydro_velocity(hydro_data) );
      eh_message( "width        : %f" , sed_hydro_width   (hydro_data) );
      eh_message( "depth        : %f" , sed_hydro_depth   (hydro_data) );
      eh_message( "bedload      : %f" , sed_hydro_bedload (hydro_data) );
      for ( i=0 ; i<sed_hydro_size(hydro_data) ; i++ )
         eh_message( "conc[%d]      : %f" , i , sed_hydro_nth_concentration(hydro_data,i) );
      eh_message( "eroded sediment added (m^3): %g"        , volume );
      eh_message( "sediment removed (kg): %g"              , mass_removed );
      eh_message( "suspended mass (kg): %g"                , susp_mass );
      eh_message( "bedload mass (kg): %g"                  , bedload_mass );
      eh_message( "total sediment added to basin (kg): %g" , data->total_mass );
      eh_message( "total sediment added to river (kg): %g" , data->total_mass_from_river );

// NOTE: this will be freed when the river file is closed.
//      hydro_destroy_hydro_record( river_data );
//      river_data = NULL;

   }

   sed_hydro_destroy( river_data );

   return info;
}

#define S_KEY_FILE_TYPE  "river values"
#define S_KEY_RIVER_FILE "river file"
#define S_KEY_RIVER_NAME "river name"

gboolean init_river( Eh_symbol_table symbol_table , gpointer ptr )
{
   River_t *data=(River_t*)ptr;
   char *str;

   if ( symbol_table == NULL )
   {
      eh_free( data->filename );
      eh_free( data->river_name );
      data->initialized = FALSE;
      return TRUE;
   }

   str              = eh_symbol_table_lookup( symbol_table , S_KEY_FILE_TYPE  );
   data->filename   = eh_symbol_table_value ( symbol_table , S_KEY_RIVER_FILE );

   data->river_name = eh_symbol_table_value ( symbol_table , S_KEY_RIVER_NAME );

   data->type = 0;
   if      ( strcasecmp( str , "SEASON"  )==0 )
      data->type |= HYDRO_INLINE;
   else if ( strcasecmp( str , "HYDROTREND" )==0 )
      data->type |= (HYDRO_HYDROTREND);
   else if ( strcasecmp( str , "EVENT" )==0 )
      data->type |= (HYDRO_HYDROTREND|HYDRO_USE_BUFFER);
   else if ( strcasecmp( str , "INLINE"  )==0 )
   {
      eh_warning( "keyword INLINE no longer used.  instead use SEASON." );
      data->type |= HYDRO_INLINE;
   }
   else if ( strcasecmp( str , "REFRESH" )==0 )
   {
      eh_warning( "keyword REFRESH no longer used.  instead use HYDROTREND." );
      data->type |= (HYDRO_HYDROTREND);
   }
   else if ( strcasecmp( str , "EVENT" )==0 )
   {
      eh_warning( "keyword BUFFER no longer used.  instead use EVENT." );
      data->type |= (HYDRO_HYDROTREND|HYDRO_USE_BUFFER);
   }
   else
   {
      eh_warning("unknown option -- %s\n",str);
      eh_warning("valid options are: 'season', 'hydrotrend', or 'event'.\n");
      eh_exit(-1);
   }

   if ( !eh_try_open(data->filename) )
      eh_exit(-1);

// River location.
   data->location = 0;

   return TRUE;
}

gboolean dump_river_data( gpointer ptr , FILE *fp )
{
   River_t *data = (River_t*)ptr;
   guint len;

   fwrite( data , sizeof(River_t) , 1 , fp );

   len = strlen(data->filename)+1;
   fwrite( &len , sizeof(guint) , 1 , fp );
   fwrite( data->filename , sizeof(char) , len , fp );

   return TRUE;
}

gboolean load_river_data( gpointer ptr , FILE *fp )
{
   River_t *data = (River_t*)ptr;
   guint len;

   fread( data , sizeof(River_t) , 1 , fp );

   fread( &len , sizeof(guint)   , 1 , fp );
   fread( data->filename , sizeof(char) , len , fp );
   
   return TRUE;
}


/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  READRIVER                                                           *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Read the river data for the next time step.  The data file in this  *
*  case is the binary output file from HYDROTREND.                     *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  runPrefix - The file name prefix for the river data file.           *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  NONE                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  hydroData   - Structure containing the river data for this time     *
*                step.                                                 *
*  hydroHeader - Structure containing the header for the data file.    *
*  first_time  - Flag indicating if this is the first time through the *
*                function.                                             *
*  prefix      - The file name prefix for the data file.               *
*  extension   - The file name extension for the data file.            *
*  fpRiver     - Pointer to the data file.                             *
*                                                                      *
***********************************************************************/

#if defined(IGNORE)

#include <stdio.h>
#include <string.h>
//#include <values.h>
#include "hydro.h"

Hydro_record *read_river(FILE *fpRiver)
{
   Hydro_record *hydro_data;
   static Hydro_header hydro_header;
   
   if ( ftell(fpRiver) == 0 )
      hydro_header = readHydroHeader(fpRiver);

   hydro_data = hydro_create_hydro_record(hydro_header.nGrain);

   if ( readHydroRecord(fpRiver,hydro_data,hydro_header.nGrain) )
      return hydro_data;
   else
   {
      hydro_destroy_hydro_record(hydro_data);
      return NULL;
   }
}

/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  READRIVERINLINE                                                     *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Read the river data for the next time step.  The data file in this  *
*  case is an ascii file.  An example file follows.                    *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  fpRiver - Pointer to the input file.                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  NONE                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  line      - The next line read from the input file.                 *
*  riverData - Structure of the river data for the next time step.     *
*  val       - The next number value read from the input file.         *
*  nGrains   - The number of grain types provided by the river (both   *
*              suspended load and bedload.                             *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Example input file:                                                  *
*                                                                      *
*  Number of Grains: 5                                                 *
*  -- Grain #1 (bedload) ---                                           *
*  Bulk Density (kg/m^3):      1850                                    *
*  Grain Size (microns):       200                                     *
*  Removal Rate (1/day):       50                                      *
*  Diffusion coefficient (-):  .25                                     *
*  -- Grain #2 ---*                                                    *
*  Bulk Density (kg/m^3):      1750                                    *
*  Grain Size (microns):       75                                      *
*  Removal Rate (1/day):       16.8                                    *
*  Diffusion coefficient (-):  .25                                     *
*                                                                      *
***********************************************************************/

#include <stdio.h>
#include <string.h>
//#include <values.h>
#include "hydro.h"

Hydro_record *read_river_inline(FILE *fpRiver)
{
   char line[S_LINEMAX];
   Hydro_record *river_data;
   double val;
   int nGrains;
   
   if ( read_int_vector(fpRiver,&nGrains,1)!=1 )
      return NULL;
   
   river_data = hydro_create_hydro_record(nGrains-1);
   
   read_double_vector(fpRiver,&river_data->bedload,1);
   read_double_vector(fpRiver,river_data->conc,(int)(nGrains-1));
   read_double_vector(fpRiver,&river_data->velocity,1);
   read_double_vector(fpRiver,&river_data->width,1);
   read_double_vector(fpRiver,&river_data->depth,1);

   return river_data;
   
}

#endif
