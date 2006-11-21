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
#include <limits.h>
#include <math.h>
#include <string.h>

#include "sed_sedflux.h"
#include "processes.h"
#include "plume_types.h"
#include "utils.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#define G_KEY_MARGIN_NAME    "margin name"
#define G_KEY_V_RES          "vertical resolution"
#define G_KEY_X_RES          "x resolution"
#define G_KEY_Y_RES          "y resolution"
#define G_KEY_H_RES          "horizontal resolution"
#define G_KEY_LENGTH         "basin length"
#define G_KEY_WIDTH          "basin width"
#define G_KEY_BATHY_FILE     "bathymetry file"
#define G_KEY_SEDIMENT_FILE  "sediment file"

void sed_free_process( Sed_process p , gpointer free_seg );
void destroy_epoch( Epoch *epoch );

Sed_cube init_cube( Eh_key_file in_file , const char *img_file )
{
   Sed_cube prof;

   //---
   // initialize sediment profile.  if an image file was supplied, then
   // initialize the profile with that profile.  otherwise initialize an
   // empty profile using the data supplied in the init file.  in the case
   // of initializing from a file, any data supplied in the init file 
   // about the profile is ignored.
   //---
   if ( img_file )
   {
      FILE *fp = fopen( img_file , "rb" );
      if ( !fp )
         perror( img_file ), eh_exit(-1);
      prof = sed_cube_read( fp );
      fclose( fp );
   }
   else
   {
      Eh_dbl_grid grid;
      char *name, *bathy_file, *sediment_file;
      Sed_sediment sediment_type;
      gssize i, j;
      double z_res, x_res, y_res;

      // Read the init file and read the global vars.
      name          = eh_key_file_get_value    ( in_file , "global" , G_KEY_MARGIN_NAME );
      z_res         = eh_key_file_get_dbl_value( in_file , "global" , G_KEY_V_RES );
      x_res         = eh_key_file_get_dbl_value( in_file , "global" , G_KEY_X_RES );
      y_res         = eh_key_file_get_dbl_value( in_file , "global" , G_KEY_Y_RES );
      bathy_file    = eh_key_file_get_value    ( in_file , "global" , G_KEY_BATHY_FILE );
      sediment_file = eh_key_file_get_value    ( in_file , "global" , G_KEY_SEDIMENT_FILE );

      if ( is_sedflux_3d() )
         grid = sed_get_floor_2d_grid( bathy_file , x_res , y_res );
      else
         grid = sed_get_floor_1d_grid( bathy_file , x_res , y_res );

      sediment_type = sed_sediment_scan( sediment_file );
      sed_sediment_set_env( sediment_type );

      prof = sed_cube_new( eh_grid_n_x(grid) , eh_grid_n_y(grid) );
      for ( i=0 ; i<sed_cube_n_x(prof) ; i++ )
         for ( j=0 ; j<sed_cube_n_y(prof) ; j++ )
         {
            sed_column_set_x_position ( sed_cube_col_ij(prof,i,j) , eh_grid_x(grid)[i]        );
            sed_column_set_y_position ( sed_cube_col_ij(prof,i,j) , eh_grid_y(grid)[j]        );
            sed_column_set_base_height( sed_cube_col_ij(prof,i,j) , eh_dbl_grid_val(grid,i,j) );
         }

      sed_cube_set_x_res( prof , x_res );
      sed_cube_set_y_res( prof , y_res );
      sed_cube_set_z_res( prof , z_res );

      sed_cube_set_name( prof , name );

      sed_sediment_destroy( sediment_type );
      eh_free( name );
      eh_free( bathy_file );
      eh_free( sediment_file );
      eh_grid_destroy( grid , TRUE );
   }
   
   return prof;
}

Epoch *init_epoch(Eh_symbol_table);

int sed_cmp_epoch_no(gconstpointer,gconstpointer);

// Return a singly linked list of epochs in the order that they are to
// be run.
//GSList *init_epoch_list(GSList *symbol_table_list )
GSList *init_epoch_list( Eh_key_file in_file )
{
   GSList *epoch_list=NULL;

   eh_debug( "Initialize each epoch" );
   {
      Eh_symbol_table* epoch_tab = eh_key_file_get_symbol_tables( in_file , "epoch" );
      Eh_symbol_table* tab;

      for ( tab=epoch_tab ; *tab ; tab++ )
      {
eh_symbol_table_print_aligned( *tab , stderr );
         epoch_list = g_slist_append( epoch_list , init_epoch( *tab ) );
         eh_symbol_table_destroy( *tab );
      }
      eh_free( epoch_tab );
   }
/*
   for ( i=0 ; i<g_slist_length(symbol_table_list) ; i++ )
      epoch_list = g_slist_append(
                      epoch_list ,
                      init_epoch(
                         (GHashTable*)g_slist_nth_data(symbol_table_list,i) ) );
*/

   // sort the epochs by epoch number.
   epoch_list = g_slist_sort( epoch_list , &sed_cmp_epoch_no );

   return epoch_list;
}

void destroy_epoch_list( GSList *epoch_list )
{
   void _destroy_epoch( gpointer data , gpointer user_data );

   g_slist_foreach( epoch_list , &_destroy_epoch , NULL );
   g_slist_free( epoch_list );
}

void destroy_epoch( Epoch *epoch )
{
   eh_free( epoch->filename );
   eh_free( epoch );
}

void _destroy_epoch( gpointer data , gpointer user_data )
{
   destroy_epoch( (Epoch*)data );
}

int sed_cmp_epoch_no(gconstpointer a,gconstpointer b)
{
   Epoch *e1 = (Epoch*)a;
   Epoch *e2 = (Epoch*)b;
   if ( e1->number > e2->number )
      return 1;
   else if ( e1->number < e2->number )
      return -1;
   else
      return 0;
}

/**@name Epoch_labels
*/
//@{
/// Label for epoch number.
#define G_KEY_EPOCH_NUMBER "epoch number"
/// Label for the duration of an epoch.
#define G_KEY_EPOCH_DURATION     "epoch duration"
/// Label for the time step of an epoch.
#define G_KEY_EPOCH_TIME_STEP    "epoch time step"
/// Label for the file name of the file holding the process information.
#define G_KEY_EPOCH_FILE   "epoch process file"
//@}

/** Initialize an epoch from a symbol table.

An Epoch type is initialized with data from a label/value hash table.  These
label/values are usually read in from a file.

\param t    A pointer to a label/value hash table.

\return     A pointer to the new epoch.

\see Epoch_labels .
*/
Epoch *init_epoch( Eh_symbol_table t )
{
   Epoch *new_epoch = eh_new(Epoch,1);

   new_epoch->number    = eh_symbol_table_dbl_value ( t , G_KEY_EPOCH_NUMBER    );
   new_epoch->duration  = eh_symbol_table_time_value( t , G_KEY_EPOCH_DURATION  );
   new_epoch->time_step = eh_symbol_table_time_value( t , G_KEY_EPOCH_TIME_STEP );
   new_epoch->filename  = eh_symbol_table_value     ( t , G_KEY_EPOCH_FILE      );

eh_watch_dbl( new_epoch->number );

   if ( !eh_try_open( new_epoch->filename ) )
      eh_exit(-1);

   return new_epoch;
}

/** Create the processes that are used in sedflux.

@return A pointer to a newly created Sed_process_list.

@see init_process_list , destroy_process_list .
*/
Sed_process_list *create_process_list( void )
{
   Sed_process_list *pl = eh_new( Sed_process_list , 1 );

   pl->constants   = sed_process_new( "constants"         , Constants_t ,
                                      &init_constants     , &run_constants );
   pl->quake       = sed_process_new( "earthquake"        , Quake_t ,
                                      &init_quake         , &run_quake );
   pl->tide        = sed_process_new( "tide"              , Tide_t ,
                                      &init_tide          , &run_tide );
   pl->sea_level   = sed_process_new( "sea level"         , Sea_level_t ,
                                      &init_sea_level     , &run_sea_level );
   pl->storm       = sed_process_new( "storms"            , Storm_t ,
                                      &init_storm         , &run_storm );
   pl->avulsion    = sed_process_new( "avulsion"          , Avulsion_t ,
                                      &init_avulsion      , &run_avulsion );
   pl->erosion     = sed_process_new( "erosion"           , Erosion_t ,
                                      &init_erosion       , &run_erosion );
   pl->river       = sed_process_new( "river"             , River_t ,
                                      &init_river         , &run_river );
   pl->bedload     = sed_process_new( "bedload dumping"   , Bedload_dump_t ,
                                      &init_bedload       , &run_bedload );
   pl->plume       = sed_process_new( "plume"             , Plume_t ,
                                      &init_plume         , &run_plume );
   pl->turbidity   = sed_process_new( "turbidity current" , Turbidity_t ,
                                      &init_turbidity     , &run_turbidity );
   pl->debris_flow = sed_process_new( "debris flow"       , Debris_flow_t ,
                                      &init_debris_flow   , &run_debris_flow );
   pl->slump       = sed_process_new( "slump"             , Slump_t ,
                                      &init_slump         , &run_slump );
   pl->diffusion   = sed_process_new( "diffusion"         , Diffusion_t ,
                                      &init_diffusion     , &run_diffusion );
   pl->xshore      = sed_process_new( "xshore"            , Xshore_t ,
                                      &init_xshore        , &run_xshore );
   pl->squall      = sed_process_new( "squall"            , Squall_t ,
                                      &init_squall        , &run_squall );
   pl->compaction  = sed_process_new( "compaction"        , Compaction_t ,
                                      &init_compaction    , &run_compaction );
   pl->flow        = sed_process_new( "flow"              , Flow_t ,
                                      &init_flow          , &run_flow );
   pl->isostasy    = sed_process_new( "isostasy"          , Isostasy_t ,
                                      &init_isostasy      , &run_isostasy );
   pl->subsidence  = sed_process_new( "subsidence"        , Subsidence_t ,
                                      &init_subsidence    , &run_subsidence );
   pl->data_dump   = sed_process_new( "data dump"         , Data_dump_t ,
                                      &init_data_dump     , &run_data_dump );
   pl->final_dump  = sed_process_new( "final dump"        , Data_dump_t ,
                                      &init_data_dump     , &run_data_dump );
   pl->failure     = sed_process_new( "failure"           , Failure_proc_t ,
                                      &init_failure       , &run_failure );
   pl->met_station = sed_process_new( "measuring station" , Met_station_t ,
                                      &init_met_station   , &run_met_station );
   pl->bbl         = sed_process_new( "bbl"               , Bbl_t ,
                                      &init_bbl           , &run_bbl );
   pl->cpr         = sed_process_new( "cpr"               , Cpr_t ,
                                      &init_cpr           , &run_cpr );
eh_debug( "DONE INIT ALL PROCESSES" );

   return pl;
}

/** Initialize the processes used in sedflux.

Scan a hash table for each of the processes.  The hash table is formed by
scanning the input file given in the Epoch, cur_epoch.

\param pl          A pointer to a Sed_process_list.
\param cur_epoch   A pointer to an Epoch.

\return A pointer to the input Sed_process_list .

\see create_process_list , destroy_process_list .
*/
Sed_process_list *init_process_list( Sed_process_list *pl ,
                                     Epoch *cur_epoch )
{
   int i;
   Eh_key_file epoch_tab;

   epoch_tab = eh_key_file_scan( epoch_get_epoch_filename(cur_epoch) );

   pl->constants_l    = sed_process_scan( epoch_tab , pl->constants    );
   pl->quake_l        = sed_process_scan( epoch_tab , pl->quake        );
   pl->tide_l         = sed_process_scan( epoch_tab , pl->tide         );
   pl->sea_level_l    = sed_process_scan( epoch_tab , pl->sea_level    );
   pl->storm_l        = sed_process_scan( epoch_tab , pl->storm        );
   pl->avulsion_l     = sed_process_scan( epoch_tab , pl->avulsion     );
   pl->erosion_l      = sed_process_scan( epoch_tab , pl->erosion      );
   pl->river_l        = sed_process_scan( epoch_tab , pl->river        );
   pl->bedload_l      = sed_process_scan( epoch_tab , pl->bedload      );
   pl->plume_l        = sed_process_scan( epoch_tab , pl->plume        );
   pl->turbidity_l    = sed_process_scan( epoch_tab , pl->turbidity    );
   pl->debris_flow_l  = sed_process_scan( epoch_tab , pl->debris_flow  );
   pl->slump_l        = sed_process_scan( epoch_tab , pl->slump        );
   pl->diffusion_l    = sed_process_scan( epoch_tab , pl->diffusion    );
   pl->xshore_l       = sed_process_scan( epoch_tab , pl->xshore       );
   pl->squall_l       = sed_process_scan( epoch_tab , pl->squall       );
   pl->compaction_l   = sed_process_scan( epoch_tab , pl->compaction   );
   pl->flow_l         = sed_process_scan( epoch_tab , pl->flow         );
   pl->isostasy_l     = sed_process_scan( epoch_tab , pl->isostasy     );
   pl->subsidence_l   = sed_process_scan( epoch_tab , pl->subsidence   );
   pl->met_station_l  = sed_process_scan( epoch_tab , pl->met_station  );
   pl->data_dump_l    = sed_process_scan( epoch_tab , pl->data_dump    );
   pl->final_dump_l   = sed_process_scan( epoch_tab , pl->final_dump   );
   pl->failure_l      = sed_process_scan( epoch_tab , pl->failure      );
   pl->bbl_l          = sed_process_scan( epoch_tab , pl->bbl          );
   pl->cpr_l          = sed_process_scan( epoch_tab , pl->cpr          );

   //---
   // Set the turbidity_current, debris_flow, and slump members of the data
   // portion of the failure process.
   //---
   for ( i=0 ; i<g_slist_length(pl->failure_l) ; i++ )
   {
      sed_process_data_val( (Sed_process)g_slist_nth_data(pl->failure_l,i) ,
                            turbidity_current                           ,
                            Failure_proc_t )
         = (Sed_process)g_slist_nth_data(pl->turbidity_l,0);

      sed_process_data_val( (Sed_process)g_slist_nth_data(pl->failure_l,i) ,
                            debris_flow                                 ,
                            Failure_proc_t )
         = (Sed_process)g_slist_nth_data(pl->debris_flow_l,0);

      sed_process_data_val( (Sed_process)g_slist_nth_data(pl->failure_l,i) ,
                            slump                                       ,
                            Failure_proc_t )
         = (Sed_process)g_slist_nth_data(pl->slump_l,0);
      sed_process_data_val( (Sed_process)g_slist_nth_data(pl->failure_l,i) ,
                            flow                                        ,
                            Failure_proc_t )
         = (Sed_process)g_slist_nth_data(pl->flow_l,0);
   }

   eh_key_file_destroy( epoch_tab );

   return pl;
}

int check_process_for_mismatch( int error_type , GSList *child , ... );
int check_process_for_error( int error_type , GSList *pl );

#define ERROR_MULTIPLE_PROCS (1<<0)
#define ERROR_NOT_FOUND      (1<<1)
#define ERROR_NOT_ACTIVE     (1<<2)
#define ERROR_NOT_ALWAYS     (1<<3)
#define ERROR_NO_PARENT      (1<<4)
#define ERROR_DT_MISMATCH    (1<<5)

gboolean check_process_list( Sed_process_list *pl , Epoch *cur_epoch )
{
   int error = 0;

   error = check_process_for_error(   ERROR_MULTIPLE_PROCS
                                    | ERROR_NOT_ACTIVE
                                    | ERROR_NOT_ALWAYS ,
                                    pl->plume_l );
   error = check_process_for_error(   ERROR_MULTIPLE_PROCS
                            | ERROR_NOT_ACTIVE
                            | ERROR_NOT_ALWAYS ,
                            pl->bedload_l );
   error = check_process_for_error(   ERROR_MULTIPLE_PROCS
                            | ERROR_NOT_ACTIVE
                            | ERROR_NOT_ALWAYS ,
                            pl->bbl_l );
   error = check_process_for_error(   ERROR_NOT_ACTIVE
                                    | ERROR_NOT_ALWAYS ,
                                    pl->river_l );

   //---
   // Check earthquakes:
   //    1. There is only one quake process.
   //    2. If quakes are on, failures are on.
   //    3. Quakes are on the same time step that failures are.
   //---
   check_process_for_error( ERROR_MULTIPLE_PROCS ,
                            pl->quake_l );
   check_process_for_mismatch( ERROR_NO_PARENT|ERROR_DT_MISMATCH ,
                               pl->quake_l , pl->failure_l , NULL );

   //---
   // Check storms:
   //    1. There is only one storm process.
   //    2. If storms are on, squall or diffusion is on.
   //    3. Storms are on the same time step that diffusion and squall are.
   //---
   check_process_for_error( ERROR_MULTIPLE_PROCS ,
                            pl->storm_l );
   check_process_for_mismatch( ERROR_NO_PARENT|ERROR_DT_MISMATCH ,
                               pl->storm_l                       ,
                               pl->squall_l                      ,
                               pl->diffusion_l                   ,
                               NULL );

   return error;
}

int check_process_for_mismatch( int error_type , GSList *child , ... )
{
   int error = 0;
   GSList *parent_list;
   Sed_process child_proc, parent_proc;
   double proc_dt;
   va_list ap;
   char *name;

   va_start( ap , child );
   parent_list  = va_arg( ap , GSList* );
   child_proc = g_slist_nth_data( child , 0 );
   proc_dt    = sed_process_interval( child_proc );
   name       = sed_process_name    ( child_proc );
   for ( ; parent_list ; parent_list = va_arg( ap , GSList* ) )
   {
      parent_proc = g_slist_nth_data( parent_list , 0 );

      if ( error_type & ERROR_NO_PARENT )
         if ( sed_process_is_active(child_proc) && !sed_process_is_active(parent_proc) )
            error |= ERROR_NO_PARENT;

      if ( error_type & ERROR_DT_MISMATCH )
         if ( sed_process_is_active(child_proc) && sed_process_is_active(parent_proc) )
            if ( !eh_compare_dbl( sed_process_interval(child_proc) ,
                                  sed_process_interval(parent_proc) , 1e-12 ) )
               error |= ERROR_DT_MISMATCH;
   }
      
   va_end( ap );

   error &= error_type;

   if ( error & ERROR_NO_PARENT )
      eh_warning( "%s: Sed_process does not have a parent process." , name );
   if ( error & ERROR_DT_MISMATCH )
      eh_warning( "%s: Sed_process time step should match parent's." , name );

   eh_free( name );

   return error;
}

int check_process_for_error( int error_type , GSList *pl )
{
   int error = 0;
   int i, n;
   gboolean is_active;
   Sed_process this_proc;
   char *name = NULL;

   if ( !pl )
      error |= ERROR_NOT_FOUND;
   else
   {
      n = g_slist_length( pl );
      if ( n>1 )
         error |= ERROR_MULTIPLE_PROCS;
      for ( i=0,is_active=FALSE ; i<n ; i++ )
      {
         this_proc = g_slist_nth_data( pl , i );
         is_active |= sed_process_is_active(this_proc);
         if ( sed_process_interval(this_proc) > 0 )
            error |= ERROR_NOT_ALWAYS;
      }
      name = sed_process_name( this_proc );
      if ( !is_active )
         error |= ERROR_NOT_ACTIVE;
   }

   error &= error_type;

   if ( error & ERROR_MULTIPLE_PROCS )
      eh_warning( "%s: Sed_process definition not found." , (name)?(name):"(null)" );
   if ( error & ERROR_MULTIPLE_PROCS )
      eh_warning( "%s: Multiple processes found." , (name)?(name):"(null)" );
   if ( error & ERROR_NOT_ACTIVE )
      eh_warning( "%s: Sed_process should be active." , (name)?(name):"(null)" );
   if ( error & ERROR_NOT_ALWAYS )
      eh_warning( "%s: Sed_process time step should be always." , (name)?(name):"(null)" );

   eh_free( name );

   return error;
}

/** Destroy the processes used in sedflux.

@param pl A pointer to a Sed_process_list.

@see create_process_list , init_process_list .
*/
void destroy_process_list( Sed_process_list *pl )
{
eh_debug( "DESTROY PROCESS LIST: START" );
   g_slist_foreach( pl->slump_l        , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->tide_l         , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->sea_level_l    , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: STORM" );
   g_slist_foreach( pl->storm_l        , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: AVULSION" );
   g_slist_foreach( pl->avulsion_l     , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: EROSION" );
   g_slist_foreach( pl->erosion_l      , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: RIVER" );
   g_slist_foreach( pl->river_l        , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: BEDLOAD" );
   g_slist_foreach( pl->bedload_l      , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: PLUME" );
   g_slist_foreach( pl->plume_l        , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: TURBIDITY" );
   g_slist_foreach( pl->turbidity_l    , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->debris_flow_l  , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->diffusion_l    , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: XSHORE" );
   g_slist_foreach( pl->xshore_l       , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: SQUALL" );
   g_slist_foreach( pl->squall_l       , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->failure_l      , (GFunc)&sed_free_process , NULL );
eh_debug( "FREE: QUAKE" );
   g_slist_foreach( pl->quake_l        , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->constants_l    , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->compaction_l   , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->flow_l         , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->isostasy_l     , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->subsidence_l   , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->data_dump_l    , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->final_dump_l   , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->met_station_l  , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->bbl_l          , (GFunc)&sed_free_process , NULL );
   g_slist_foreach( pl->cpr_l          , (GFunc)&sed_free_process , NULL );

   sed_process_destroy( pl->slump );
   sed_process_destroy( pl->tide  );
   sed_process_destroy( pl->sea_level  );
eh_debug( "DESTROY: STORM" );
   sed_process_destroy( pl->storm  );
eh_debug( "DESTROY: AVULSION" );
   sed_process_destroy( pl->avulsion  );
   sed_process_destroy( pl->erosion  );
   sed_process_destroy( pl->river  );
   sed_process_destroy( pl->bedload  );
   sed_process_destroy( pl->plume  );
   sed_process_destroy( pl->turbidity  );
   sed_process_destroy( pl->debris_flow  );
   sed_process_destroy( pl->diffusion  );
   sed_process_destroy( pl->xshore  );
   sed_process_destroy( pl->squall  );
   sed_process_destroy( pl->failure  );
   sed_process_destroy( pl->quake  );
   sed_process_destroy( pl->constants  );
   sed_process_destroy( pl->compaction  );
   sed_process_destroy( pl->flow  );
   sed_process_destroy( pl->isostasy  );
   sed_process_destroy( pl->subsidence  );
   sed_process_destroy( pl->data_dump  );
   sed_process_destroy( pl->final_dump  );
   sed_process_destroy( pl->met_station  );
   sed_process_destroy( pl->bbl  );
   sed_process_destroy( pl->cpr  );

   eh_free( pl );
eh_debug( "DESTROY PROCESS LIST: DONE" );
}

/** Check the input process files.

This function will read in all of the epoch files, and initialize the
processes.  This is intended to find any errors in the input files at 
the begining of the model run.

@param e_list A singly liked list of pointers to Epoch's.

@return TRUE if no problems were found, FALSE otherwise.
*/
gboolean check_process_files( GSList *e_list )
{
   int n_epochs = g_slist_length( e_list );
   int epoch_no;
   Sed_process_list *pl;
   Epoch *cur_epoch;

eh_debug( "CHECK PROCESS FILE: START" );
   //---
   // For each epoch, we create each of the processes, initialize the 
   // processes from the appropriate input file, and destroy the processes.
   //---
   for ( epoch_no=0 ; epoch_no < n_epochs ; epoch_no++ )
   {
eh_debug( "CREATE PROCESS LIST" );
      pl = create_process_list( );

      cur_epoch = (Epoch*)g_slist_nth_data( e_list , epoch_no );
eh_debug( "INIT PROCESS LIST" );
      init_process_list( pl , cur_epoch );

eh_debug( "CHECK PROCESS LIST" );
      check_process_list( pl , cur_epoch );

eh_debug( "DESTROY PROCESS LIST" );
      destroy_process_list( pl );
   } 
eh_debug( "CHECK PROCESS FILE: DONE" );

   return TRUE;
}

void sed_free_process( Sed_process p , gpointer free_seg )
{
   if ( p )
      sed_process_destroy(p);
}

